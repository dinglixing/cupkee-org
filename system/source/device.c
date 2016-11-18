/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016 Lixing Ding <ding.lixing@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cupkee.h>

#include "util.h"
#include "misc.h"
#include "device.h"
#include "dev_gpio.h"
#include "dev_adc.h"
#include "dev_usart.h"

#if 0
#define _TRACE(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define _TRACE(fmt, ...)    //
#endif

#define DEVICE_MAX  (16)

struct driver_entry_t {
    const char *name;
    const cupkee_driver_t *driver;
};

static uint16_t device_magic = 211;
static cupkee_device_t *device_free = NULL;
static cupkee_device_t devices[DEVICE_MAX];

static const struct driver_entry_t drivers[] = {
    {"GPIO",  &cupkee_driver_gpio},
    {"ADC",   &cupkee_driver_adc},
    {"USART", &cupkee_driver_usart},
};
static int device_is_true(intptr_t dev);
static void device_elem(void *env, intptr_t devid, val_t *av, val_t *elem);
static const val_foreign_op_t device_op = {
    .is_true    = device_is_true,
    .elem       = device_elem,
    .prop       = device_elem,
};

static inline intptr_t device_id(cupkee_device_t *dev) {
    return ((dev - devices) << 16) | dev->magic;
}

static cupkee_device_t *device_ctrl(intptr_t devid)
{
    int magic;

    magic = devid & 0xffff;
    devid >>= 16 & 0xff;

    if (devid >= DEVICE_MAX || devices[devid].magic != magic) {
        return NULL;
    } else {
        return devices + devid;
    }
}

static cupkee_device_t *device_get(val_t *d)
{
    val_foreign_t *vf;
    intptr_t devid;
    int magic;

    if (!val_is_foreign(d)) {
        return NULL;
    } else {
        vf = (val_foreign_t *)val_2_intptr(d);
        if (vf->op != &device_op) {
            return NULL;
        }
    }

    devid = vf->data;
    magic = devid & 0xffff;
    devid >>= 16;

    if (devid >= DEVICE_MAX || devices[devid].magic != magic) {
        return NULL;
    } else {
        return devices + devid;
    }
}

static val_t device_name_list(env_t *env)
{
    void *names = (void *) array_create(env, 0, NULL);

    if (names) {
        return val_mk_array(names);
    } else {
        return VAL_UNDEFINED;
    }
}

static inline
int device_init(cupkee_device_t *dev, const cupkee_driver_t *driver)
{
    dev->magic = device_magic++;
    dev->driver = driver;

    return driver->init(dev);
}

static int device_alloc(const char *name, cupkee_device_t **pdev)
{
    int i = 0;
    int max = sizeof(drivers)/sizeof(struct driver_entry_t);

    if (!name) {
        return -CUPKEE_EINVAL;
    }

    if (!device_free) {
        return -CUPKEE_ERESOURCE;
    }

    while (i < max) {
        if (!strcmp(name, drivers[i].name)) {
            cupkee_device_t *dev = device_free;
            int err;

            err = device_init(dev, drivers[i].driver);
            if (err == CUPKEE_OK) {
                device_free = dev->next;
                dev->next = NULL;
                *pdev = dev;
            }
            return err;
        }
        i++;
    }

    return -CUPKEE_ENAME;
}

static inline val_t device_config(cupkee_device_t *dev, env_t *env, val_t *name, val_t *setting)
{
    device_config_handle_t *handle;

    if ((dev->flags & DEV_FL_ENBALE) && setting) {
        return VAL_FALSE;
    }

    handle = dev->driver->config(dev, name);
    if (handle) {
        if (setting) {
            return (0 == handle->setter(dev, env, setting)) ? VAL_TRUE : VAL_FALSE;
        } else {
            return handle->getter(dev, env);
        }
    }
    return VAL_UNDEFINED;
}

static val_t device_config_all(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    if (setting && val_is_object(setting)) {
        // multi config
        object_iter_t it;
        const char *k;
        val_t *v;

        _object_iter_init(&it, (object_t *)val_2_intptr(setting));
        while (_object_iter_next(&it, &k, &v)) {
            val_t key = val_mk_foreign_string((intptr_t)k);
            device_config_handle_t *handle = dev->driver->config(dev, &key);

            _TRACE("set %s\n", k);
            if (handle && 0 != handle->setter(dev, env, v)) {
                return VAL_FALSE;
            }
        }
        return VAL_TRUE;
    } else {
        // not support get all
        return VAL_UNDEFINED;
    }
}

static inline int device_enable(cupkee_device_t *dev)
{
    if (dev->flags & DEV_FL_ENBALE) {
        return CUPKEE_OK;
    }

    int err = dev->driver->enable(dev);
    if (err) {
        dev->flags |= DEV_FL_ERROR;
    } else {
        dev->flags = DEV_FL_ENBALE;
    }
    return err;
}

static inline int device_disable(cupkee_device_t *dev)
{
    if (0 == (dev->flags & DEV_FL_ENBALE)) {
        return CUPKEE_OK;
    }

    int err = dev->driver->disable(dev);
    if (err == CUPKEE_OK) {
        dev->flags &= ~DEV_FL_ENBALE;
    }

    return err;
}

static inline val_t device_write(cupkee_device_t *dev, env_t *env, int ac, val_t *av)
{
    if (0 == (dev->flags & DEV_FL_ENBALE)) {
        return VAL_FALSE;
    }

    return dev->driver->write(dev, env, ac, av);
}

static inline val_t device_read(cupkee_device_t *dev, env_t *env, int ac, val_t *av)
{
    if (0 == (dev->flags & DEV_FL_ENBALE)) {
        return VAL_UNDEFINED;
    }

    return dev->driver->read(dev, env, ac, av);
}

static inline val_t device_listen(cupkee_device_t *dev, val_t *e, val_t *cb)
{
    if ((dev->flags & DEV_FL_ERROR)) {
        return VAL_FALSE;
    }

    return dev->driver->listen(dev, e, cb) == CUPKEE_OK ?
           VAL_TRUE : VAL_FALSE;
}

static inline val_t device_ignore(cupkee_device_t *dev, val_t *e)
{
    if ((dev->flags & DEV_FL_ERROR)) {
        return VAL_FALSE;
    }

    return dev->driver->ignore(dev, e) == CUPKEE_OK ?
        VAL_TRUE : VAL_FALSE;
}

static inline void device_get_elem(cupkee_device_t *dev, env_t *env, int id, val_t *elem)
{
    val_t index = val_mk_number(id);
    *elem = device_read(dev, env, 1, &index);
}

static inline void device_get_prop(cupkee_device_t *dev, const char *name, val_t *elem)
{
    (void) dev;
    (void) name;

    val_set_undefined(elem);
}

static int device_is_true(intptr_t devid)
{
    if (device_ctrl(devid)) {
        return 1;
    } else {
        return 0;
    }
}

static void device_elem(void *env, intptr_t devid, val_t *av, val_t *elem)
{
    const char *name = val_2_cstring(av);
    cupkee_device_t *dev = device_ctrl(devid);

    (void) env;

    if (!dev) {
        val_set_undefined(elem);
    }

    if (!name) {
        if (val_is_number(av)) {
            device_get_elem(dev, env, val_2_integer(av), elem);
        } else {
            val_set_undefined(elem);
        }
    } else {
        if (strcmp(name, "config") == 0) {
            val_set_native(elem, (intptr_t)device_native_config);
        } else
        if (strcmp(name, "enable") == 0) {
            val_set_native(elem, (intptr_t)device_native_enable);
        } else
        if (strcmp(name, "read") == 0) {
            val_set_native(elem, (intptr_t)device_native_read);
        } else
        if (strcmp(name, "write") == 0) {
            val_set_native(elem, (intptr_t)device_native_write);
        } else
        if (strcmp(name, "listen") == 0) {
            val_set_native(elem, (intptr_t)device_native_listen);
        } else
        if (strcmp(name, "ignore") == 0) {
            val_set_native(elem, (intptr_t)device_native_ignore);
        } else {
            device_get_prop(dev, name, elem);
        }
    }
}

void devices_setup(void)
{
    int i;

    device_free = NULL;
    for (i = 0; i < DEVICE_MAX; i++) {
        cupkee_device_t *dev = devices + i;

        dev->magic = 0;
        dev->flags = 0;
        dev->driver = NULL;
        dev->next = device_free;

        device_free = dev;
    }

    dev_setup_gpio();
    dev_setup_adc();
    dev_setup_usart();

    return;
}

void devices_event_proc(env_t *env, int event)
{
    uint8_t dev   = (event >> 16) & 0xff;
    uint8_t which = (event >> 8) & 0xff;
    int max = sizeof(drivers)/sizeof(struct driver_entry_t);

    if (dev < max) {
        event = (event & 0xff);
        const cupkee_driver_t *driver = drivers[dev].driver;
        if (driver->event_handle) {
            driver->event_handle(env, which, event);
        }
    }
}

val_t native_device(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev = NULL;
    int err = 0;

    if (ac == 0) {
        return device_name_list(env);
    }

    if (0 == (err = device_alloc(val_2_cstring(av), &dev))) {
        val_t d = val_create(env, &device_op, device_id(dev));
        return d;
    } else {
        return VAL_UNDEFINED;
    }
}

val_t device_native_config(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;
    val_t *name = NULL;
    val_t *setting;

    if (ac == 0 || !(dev = device_get(av))) {
        return VAL_UNDEFINED;
    }

    av++; ac--;
    if (ac && (val_is_number(av) || val_is_string(av))) {
        name = av++;
        ac--;
    }

    if (ac) {
        setting = av;
    } else {
        setting = NULL;
    }

    return device_config(dev, env, name, setting);
}

val_t device_native_enable(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;
    int err = 0;

    (void) env;

    if (ac == 0 || !(dev = device_get(av))) {
        return VAL_UNDEFINED;
    }

    if (ac == 1) {
        return (dev->flags & DEV_FL_ENBALE) ? VAL_TRUE : VAL_FALSE;
    }
    val_t *hnd = av++;

    ac--;
    if (!val_is_true(av)) {
        err = device_disable(dev);
        av++; ac--;
    } else {
        if (val_is_object(av)) {
            device_config_all(dev, env, av);
            ac--; av++;
        }
        if (!err) {
            err = device_enable(dev);
        }
    }

    if (ac && val_is_function(av)) {
        val_t args[2];
        args[0] = err ? val_mk_number(err) : VAL_UNDEFINED;
        args[1] = *hnd;

        cupkee_do_callback(env, av, 2, args);
    }

    return (err == CUPKEE_OK) ? VAL_TRUE : VAL_FALSE;
}

val_t device_native_write(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    (void) env;

    if (ac < 2 || !(dev = device_get(av))) {
        return VAL_UNDEFINED;
    }

    return device_write(dev, env, ac - 1, av + 1);
}

val_t device_native_read(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    (void) env;

    if (ac < 1 || !(dev = device_get(av))) {
        return VAL_UNDEFINED;
    }

    return device_read(dev, env, ac - 1, av + 1);
}

val_t device_native_listen(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    (void) env;

    if (ac < 3 || !(dev = device_get(av))) {
        return VAL_UNDEFINED;
    }

    val_t *event = av + 1;
    val_t *cb = av + 2;

    if (!val_is_string(event) && !val_is_number(event)) {
        return VAL_FALSE;
    }

    if (!val_is_function(cb)) {
        return VAL_FALSE;
    }

    return device_listen(dev, event, cb);
}

val_t device_native_ignore(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    (void) env;

    if (ac < 2 || !(dev = device_get(av))) {
        return VAL_UNDEFINED;
    }

    val_t *event = av + 1;
    if (!val_is_string(event) && !val_is_number(event)) {
        return VAL_FALSE;
    }

    return device_ignore(dev, event);
}

// new here
#define DEVICE_ENABLE       0x80
#define DEVICE_EVNET_MASK   0x0f

typedef struct cupkee_xxx_t {
    uint16_t magic;
    uint8_t  flags;
    uint8_t  inst;
    const hw_device_desc_t *desc;
    const hw_driver_t      *driver;
    val_t    *event_handle[DEVICE_EVENT_MAX];
    struct cupkee_xxx_t *next;
} cupkee_xxx_t;

static const char *device_event_names[] = {
    "error", "data", "drain", "ready"
};
static void xxx_op_prop(void *env, intptr_t devid, val_t *name, val_t *prop);
static void xxx_op_elem(void *env, intptr_t devid, val_t *name, val_t *prop);
static cupkee_xxx_t xxx_queue[DEVICE_MAX];
static cupkee_xxx_t *xxx_free = NULL;
static cupkee_xxx_t *xxx_work = NULL;
static uint16_t xxx_magic_factor = 1;
static const val_foreign_op_t xxx_op = {
    .prop = xxx_op_prop,
    .elem = xxx_op_elem,
};

static inline intptr_t xxx_id_gen(cupkee_xxx_t *xxx) {
    return ((xxx - xxx_queue) << 16) | xxx->magic;
}

static cupkee_xxx_t *xxx_id_control(intptr_t devid)
{
    int magic;

    magic = devid & 0xffff;
    devid >>= 16 & 0xff;

    if (devid >= DEVICE_MAX || xxx_queue[devid].magic != magic) {
        return NULL;
    } else {
        return xxx_queue + devid;
    }
}

static cupkee_xxx_t *xxx_val_control(val_t *d)
{
    val_foreign_t *vf;

    if (val_is_foreign(d)) {
        vf = (val_foreign_t *)val_2_intptr(d);
        if (vf->op == &xxx_op) {
            return xxx_id_control(vf->data);
        }
    }
    return NULL;
}

static cupkee_xxx_t *xxx_alloc(void)
{
    cupkee_xxx_t *xxx = xxx_free;

    if (xxx) {
        int i;
        xxx_free = xxx->next;

        for (i = 0; i < DEVICE_EVENT_MAX; i++) {
            xxx->event_handle[i] = NULL;
        }

        xxx->flags = 0;
        xxx->magic = (xxx_magic_factor + 1) ^ (xxx_magic_factor << 8);
        xxx_magic_factor++;
    }
    return xxx;
}

static int xxx_release(cupkee_xxx_t *xxx)
{
    if (xxx) {
        xxx->driver->release(xxx->inst);

        memset(xxx, 0, sizeof(cupkee_xxx_t));
        xxx->next = xxx_free;
        xxx_free = xxx;
        return 0;
    }

    return -1;
}

static cupkee_xxx_t *device_create(const char *name, int instance)
{
    cupkee_xxx_t  *xxx;
    const hw_device_desc_t *desc;
    const hw_driver_t      *driver;

    desc = hw_device_take(name, instance, &driver);
    if (!desc) {
        return NULL;
    }

    xxx = xxx_alloc();
    if (xxx) {
        xxx->inst = instance;
        xxx->desc = desc;
        xxx->driver = driver;
    } else {
        driver->release(instance);
    }

    return xxx;
}

static void xxx_work_list_join(cupkee_xxx_t *xxx)
{
    xxx->next = xxx_work;
    xxx_work = xxx;
}

static void xxx_work_list_drop(cupkee_xxx_t *xxx)
{
    cupkee_xxx_t *cur = xxx_work;

    if (cur == xxx) {
        xxx_work = cur->next;
        return;
    }

    while(cur) {
        cupkee_xxx_t *next = cur->next;

        if (next == xxx) {
            cur->next = xxx->next;
            return;
        }

        cur = next;
    }
}

static void xxx_enable(cupkee_xxx_t *xxx)
{
    if (!(xxx->flags & DEVICE_ENABLE)) {
        if (xxx->driver->enable(xxx->inst)) {
            xxx->flags |= DEVICE_ENABLE;
            xxx_work_list_join(xxx);
        } else {
            //Todo: set error
        }
    }
}

static void xxx_disable(cupkee_xxx_t *xxx)
{
    if (xxx->flags & DEVICE_ENABLE) {
        if (xxx->driver->disable(xxx->inst)) {
            xxx->flags &= ~DEVICE_ENABLE;
            xxx_work_list_drop(xxx);
        } else {
            //Todo: set error
        }
    }
}

static int xxx_str_map(const char *name, int max, const char **names)
{
    int id;

    if (!name)
        return max;

    for (id = 0; id < max && names[id]; id++) {
        if (!strcmp(name, names[id])) {
            return id;
        }
    }
    return id;
}

static inline int xxx_val_map(val_t *in, int max, const char **names)
{
    if (val_is_number(in)) {
        return val_2_integer(in);
    } else {
        return xxx_str_map(val_2_cstring(in), max, names);
    }
}

/* return:
 *  0: ok
 * -1: invalid value
 */
static int xxx_setting_convert(const cupkee_xxx_t *xxx, const hw_config_desc_t *desc, val_t *value, int *setting)
{
    switch(desc->type) {
    case HW_CONFIG_BOOL:
        *setting = val_is_true(value);
        break;
    case HW_CONFIG_NUM:
        if (!val_is_number(value)) {
            return -1;
        }
        *setting = val_2_integer(value);
        break;
    case HW_CONFIG_OPT:
        *setting = cupkee_id(value, desc->opt_num, xxx->desc->opt_names + desc->opt_start);
        if (*setting >= desc->opt_num) {
            return -1;
        }
        break;
    default:
        return -1;
    }

    return 0;
}

// return:
//   0: ok
//   -1: hw false
//   -2: invalid item
//   -3: invalid value
static int xxx_config_set(cupkee_xxx_t *xxx, int which, val_t *value)
{
    const hw_config_desc_t *conf;
    int setting;

    if (which < 0 || which >= xxx->desc->conf_num) {
        return -3;
    }

    conf = &xxx->desc->conf_descs[which];
    if (xxx_setting_convert(xxx, conf, value, &setting)) {
        return -2;
    }

    if (xxx->driver->config_set(xxx->inst, which, setting)) {
        return 0;
    } else {
        return -1;
    }
}

static inline val_t xxx_config_set_one(cupkee_xxx_t *xxx, val_t *name, val_t *value)
{
    int err = xxx_config_set(xxx, xxx_val_map(name, xxx->desc->conf_num, xxx->desc->conf_names), value);

    if (!err) {
        return VAL_TRUE;
    }
    if (err == -3) {
        return VAL_UNDEFINED;
    }
    return VAL_FALSE;
}

static val_t xxx_config_set_all(cupkee_xxx_t *xxx, val_t *value)
{
    if (val_is_object(value)) {
        object_iter_t it;
        const char *k;
        val_t *v;

        _object_iter_init(&it, (object_t *)val_2_intptr(value));
        while (_object_iter_next(&it, &k, &v)) {
            int which = xxx_str_map(k, xxx->desc->conf_num, xxx->desc->conf_names);
            if (xxx_config_set(xxx, which, v)) {
                return VAL_FALSE;
            }
        }
        return VAL_TRUE;
    }
    return VAL_FALSE;
}

static val_t xxx_config_get(cupkee_xxx_t *xxx, val_t *name)
{
    int which = cupkee_id(name, xxx->desc->conf_num, xxx->desc->conf_names);
    const hw_config_desc_t *conf;
    int setting;

    if (which >= xxx->desc->conf_num) {
        return VAL_UNDEFINED;
    }

    if (!xxx->driver->config_get(xxx->inst, which, &setting)) {
        return VAL_UNDEFINED;
    }

    conf = &xxx->desc->conf_descs[which];
    switch(conf->type) {
    case HW_CONFIG_BOOL:
        return setting ? VAL_TRUE : VAL_FALSE;
    case HW_CONFIG_NUM:
        return val_mk_number(setting);
    case HW_CONFIG_OPT:
        return val_mk_foreign_string((intptr_t)xxx->desc->opt_names[conf->opt_start + setting]);
    default:
        return VAL_UNDEFINED;
    }
}

static val_t xxx_config_get_all(cupkee_xxx_t *xxx)
{
    (void) xxx;
    return VAL_UNDEFINED;
}

static val_t xxx_native_destroy(env_t *env, int ac, val_t *av)
{
    cupkee_xxx_t *xxx;

    (void) env;

    if (ac && (xxx = xxx_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_FALSE;
    }

    if (0 != xxx_release(xxx)) {
        return VAL_FALSE;
    } else {
        return VAL_TRUE;
    }
}

static val_t xxx_native_config(env_t *env, int ac, val_t *av)
{
    cupkee_xxx_t *xxx;
    val_t *name = NULL;
    val_t *setting;

    (void) env;

    if (ac && (xxx = xxx_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (ac && (val_is_number(av) || val_is_string(av))) {
        name = av++; ac--;
    }

    setting = ac > 0 ? av : NULL;

    if (setting) {
        return name ? xxx_config_set_one(xxx, name, setting) :
                      xxx_config_set_all(xxx, setting);
    } else {
        return name ? xxx_config_get(xxx, name) :
                      xxx_config_get_all(xxx);
    }
}

static val_t xxx_native_enable(env_t *env, int ac, val_t *av)
{
    cupkee_xxx_t *xxx;
    val_t *setting, *hnd;

    (void) env;

    if (ac && (xxx = xxx_val_control(av)) != NULL) {
        hnd = av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    setting = ac > 0 ? av : NULL;

    if (setting) {
        if (val_is_true(setting)) {
            if (val_is_object(setting)) {
                if (VAL_TRUE == xxx_config_set_all(xxx, setting)) {
                    xxx_enable(xxx);
                }
            } else {
                xxx_enable(xxx);
            }
        } else {
            xxx_disable(xxx);
        }
        ac--; av++;
    }

    if (ac && val_is_function(av)) {
        val_t args[2];
        int err = xxx->driver->get_err(xxx->inst);
        args[0] = err ? val_mk_number(err) : VAL_UNDEFINED;
        args[1] = *hnd;

        cupkee_do_callback(env, av, 2, args);
    }

    return (xxx->flags & DEVICE_ENABLE) ? VAL_TRUE : VAL_FALSE;
}

static val_t xxx_native_listen(env_t *env, int ac, val_t *av)
{
    cupkee_xxx_t *xxx;
    val_t *callback;
    int event_id;

    (void) env;

    if (ac >= 3 && (xxx = xxx_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    callback = av + 1;
    event_id = xxx_val_map(av, DEVICE_EVENT_MAX, device_event_names);
    if (event_id >= xxx->desc->event_num || !val_is_function(callback)) {
        return VAL_FALSE;
    }

    if (xxx->event_handle[event_id] == NULL) {
        val_t *ref = reference_create(callback);

        if (!ref) {
            return VAL_FALSE;
        }
        xxx->event_handle[event_id] = ref;
    } else {
        // origion ref released, automic
        *xxx->event_handle[event_id] = *callback;
    }

    xxx->driver->listen(xxx->inst, event_id);
    return VAL_TRUE;
}

static val_t xxx_native_read_map(env_t *env, int ac, val_t *av)
{
    cupkee_xxx_t *xxx;
    int off;
    uint32_t v;

    (void) env;

    if (ac && (xxx = xxx_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (!(xxx->flags & DEVICE_ENABLE)) {
        return VAL_UNDEFINED;
    }

    if (!device_param_int(ac, av, &off)) {
        off = -1;
    }

    if (xxx->driver->io.map.get(xxx->inst, off, &v)) {
        return val_mk_number(v);
    } else {
        return VAL_UNDEFINED;
    }
}

static void xxx_serial_recv(cupkee_xxx_t *xxx, env_t *env, val_t *cb, int n)
{
    type_buffer_t *buffer = buffer_create(env, n);

    if (!buffer) {
        cupkee_do_callback_error(env, cb, CUPKEE_ERESOURCE);
        return;
    } else {
        val_t args[2];

        // Todo: check return value
        xxx->driver->io.serial.recv(xxx->inst, n, buffer->buf);

        val_set_undefined(args);
        val_set_buffer(args + 1, buffer);
        cupkee_do_callback(env, cb, 2, args);
    }
}

static val_t xxx_native_read_serial(env_t *env, int ac, val_t *av)
{
    cupkee_xxx_t *xxx;
    int n, size, err;

    if (ac && (xxx = xxx_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (!(xxx->flags & DEVICE_ENABLE)) {
        return VAL_FALSE;
    }

    size = xxx->driver->io.serial.received(xxx->inst);
    if (size < 0) {
        err = xxx->driver->get_err(xxx->inst);
        // hardware error, bsp driver should post a err message
        goto DO_ERROR;
    }

    if (device_param_int(ac, av, &n)) {
        ac--; av++;
        if (n < 0) {
            err = CUPKEE_EINVAL;
            goto DO_ERROR;
        }
    } else {
        n = size;
    }

    if (ac && val_is_function(av)) {
        if (n > size) {
            n = size;
        }
        if (n > 0)
            xxx_serial_recv(xxx, env, av, n);
    } else {
        // Do nothing if without callback
    }
    return VAL_TRUE;

DO_ERROR:
    if (ac && val_is_function(av)) {
        cupkee_do_callback_error(env, av, err);
    }
    return VAL_FALSE;
}

static val_t xxx_native_read_block(env_t *env, int ac, val_t *av)
{
    (void) env;
    (void) ac;
    (void) av;
    return VAL_UNDEFINED;
}

static val_t xxx_native_write_map(env_t *env, int ac, val_t *av)
{
    cupkee_xxx_t *xxx;
    int off, val;

    (void) env;

    if (ac && (xxx = xxx_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (!(xxx->flags & DEVICE_ENABLE)) {
        return VAL_UNDEFINED;
    }

    if (!device_param_int(ac, av, &off)) {
        return VAL_FALSE;
    }

    av++; ac--;
    if (!device_param_int(ac, av, &val)) {
        val = off;
        off = -1;
    }

    if (xxx->driver->io.map.set(xxx->inst, off, (uint32_t)val)) {
        return VAL_TRUE;
    } else {
        return VAL_FALSE;
    }
}

static val_t xxx_native_write_serial(env_t *env, int ac, val_t *av)
{
    cupkee_xxx_t *xxx;
    val_t cb_args[3];
    void *addr;
    int   size, n, err = 0;

    if (ac && (xxx = xxx_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        err = CUPKEE_EINVAL;
        goto DO_ERROR;
    }

    if (device_param_stream(ac, av, &addr, &size)) {
        cb_args[2] = *av++; ac--;
    } else {
        err = CUPKEE_EINVAL;
        goto DO_ERROR;
    }

    if (device_param_int(ac, av, &n)) {
        ac--; av++;
        if (n < 0) {
            err = CUPKEE_EINVAL;
            goto DO_ERROR;
        } else
        if (n > size) {
            n = size;
        }
    } else {
        n = size;
    }

    n = xxx->driver->io.serial.send(xxx->inst, n, addr);
    if (n < 0) {
        err = xxx->driver->get_err(xxx->inst);
        goto DO_ERROR;
    }

    if (ac && val_is_function(av)) {
        val_set_undefined(cb_args);
        val_set_number(cb_args + 1, n);
        cupkee_do_callback(env, av, 3, cb_args);
    }
    return val_mk_number(n);

DO_ERROR:
    if (ac && val_is_function(av)) {
        cupkee_do_callback_error(env, av, err);
    }
    return VAL_FALSE;
}

static val_t xxx_native_write_block(env_t *env, int ac, val_t *av)
{
    (void) env;
    (void) ac;
    (void) av;
    return VAL_FALSE;
}

static inline void xxx_get_map_elem(cupkee_xxx_t *xxx, env_t *env, int off, val_t *elem)
{
    uint32_t v;

    (void) env;

    if (xxx->desc->type == HW_DEVICE_MAP
        && (xxx->flags & DEVICE_ENABLE)
        && xxx->driver->io.map.get(xxx->inst, off, &v)) {
        val_set_number(elem, v);
    } else {
        val_set_undefined(elem);
    }
}

static void xxx_op_prop(void *env, intptr_t devid, val_t *name, val_t *prop)
{
    cupkee_xxx_t *xxx = xxx_id_control(devid);
    const char *prop_name = val_2_cstring(name);

    (void) env;

    if (xxx && prop_name) {
        if (!strcmp(prop_name, "read")) {
            switch (xxx->desc->type) {
            case HW_DEVICE_MAP:    val_set_native(prop, (intptr_t)xxx_native_read_map);    return;
            case HW_DEVICE_SERIAL: val_set_native(prop, (intptr_t)xxx_native_read_serial); return;
            case HW_DEVICE_BLOCK:  val_set_native(prop, (intptr_t)xxx_native_read_block);  return;
            default: val_set_undefined(prop); return;
            }
        } else
        if (!strcmp(prop_name, "write")) {
            switch (xxx->desc->type) {
            case HW_DEVICE_MAP:    val_set_native(prop, (intptr_t)xxx_native_write_map);    return;
            case HW_DEVICE_SERIAL: val_set_native(prop, (intptr_t)xxx_native_write_serial); return;
            case HW_DEVICE_BLOCK:  val_set_native(prop, (intptr_t)xxx_native_write_block);  return;
            default: val_set_undefined(prop); return;
            }
        } else
        if (!strcmp(prop_name, "config")) {
            val_set_native(prop, (intptr_t)xxx_native_config);
            return;
        } else
        if (!strcmp(prop_name, "enable")) {
            val_set_native(prop, (intptr_t)xxx_native_enable);
            return;
        } else
        if (!strcmp(prop_name, "listen")) {
            val_set_native(prop, (intptr_t)xxx_native_listen);
            return;
        } else
        if (!strcmp(prop_name, "destroy")) {
            val_set_native(prop, (intptr_t)xxx_native_destroy);
            return;
        }
    }

    val_set_undefined(prop);
}

static void xxx_op_elem(void *env, intptr_t devid, val_t *idx, val_t *elem)
{
    if (val_is_number(idx)) {
        cupkee_xxx_t *xxx = xxx_id_control(devid);

        xxx_get_map_elem (xxx, env, val_2_integer(idx), elem);
    } else {
        xxx_op_prop(env, devid, idx, elem);

    }
}

static void xxx_error_proc(cupkee_xxx_t *xxx, env_t *env)
{

    int err = xxx->driver->get_err(xxx->inst);

    cupkee_do_callback_error(env, xxx->event_handle[0], err);
}

static int xxx_map_read_all(cupkee_xxx_t *xxx, env_t *env, val_t *data)
{
    uint32_t v;

    // support combine read
    if (xxx->driver->io.map.get(xxx->inst, -1, &v)) {
        val_set_number(data, v);
    } else {
        int i, size;
        array_t *a;

        // not support combine read
        // scan all
        size = xxx->driver->io.map.size(xxx->inst);
        a = _array_create(env, size);
        if (!a) {
            return -CUPKEE_ERESOURCE;
        }

        for (i = 0; i < size; i++) {
            if (xxx->driver->io.map.get(xxx->inst, i, &v)) {
               val_set_number(_array_elem(a, i), v);
            } else {
                val_set_undefined(_array_elem(a, i));
            }
        }
        val_set_array(data, (intptr_t)a);
    }

    return 0;
}

static void xxx_map_data_proc(cupkee_xxx_t *xxx, env_t *env)
{
    val_t data;
    int err;

    err = xxx_map_read_all(xxx, env, &data);
    if (err) {
        cupkee_do_callback_error(env, xxx->event_handle[DEVICE_EVENT_ERR], err);
        return;
    }
    cupkee_do_callback(env, xxx->event_handle[DEVICE_EVENT_DATA], 1, &data);
}

static void xxx_data_proc(cupkee_xxx_t *xxx, env_t *env)
{
    if (xxx->desc->type != HW_DEVICE_MAP) {
        return;
    }

    switch(xxx->desc->type) {
    case HW_DEVICE_MAP:     xxx_map_data_proc(xxx, env);    break;
    case HW_DEVICE_SERIAL:
    case HW_DEVICE_BLOCK:
    default:                break;
    }
}

void xxx_event_proc(env_t *env, int event)
{
    cupkee_xxx_t *xxx;
    uint8_t id, inst, e;

    e = event & 0xff;

    if (e >= DEVICE_EVENT_MAX) {
        return;
    }
    id   = (event >> 16) & 0xff;
    inst = (event >> 8) & 0xff;
    xxx = xxx_work;

    while (xxx) {
        if (xxx->inst == inst && xxx->desc->id == id) {
            break;
        }
        xxx = xxx->next;
    }

    if (!xxx || !xxx->event_handle[e]) {
        return;
    }

    switch(e) {
    case DEVICE_EVENT_ERR:   xxx_error_proc(xxx, env);    break;
    case DEVICE_EVENT_DATA:  xxx_data_proc(xxx, env);     break;
    case DEVICE_EVENT_DRAIN: break;
    case DEVICE_EVENT_READY: break;
    default: break; // what happen ?
    }
}

void device_setup(void)
{
    int i;

    xxx_free = NULL;
    xxx_work = NULL;
    for (i = 0; i < DEVICE_MAX; i++) {
        cupkee_xxx_t *xxx = xxx_queue + i;

        xxx->magic = 0;
        xxx->flags = 0;
        xxx->next = xxx_free;

        xxx_free = xxx;
    }

    return;
}

val_t device_native_create(env_t *env, int ac, val_t *av)
{
    cupkee_xxx_t *xxx = NULL;
    const char *name;
    int instance;

    if (ac == 0) {
        return VAL_UNDEFINED;
    }

    name = val_2_cstring(av);
    if (!name) {
        return VAL_UNDEFINED;
    }

    if (ac > 1 && val_is_number(av + 1)) {
        instance = val_2_integer(av + 1);
    } else {
        instance = 0;
    }

    xxx = device_create(name, instance);
    if (xxx) {
        return val_create(env, &xxx_op, xxx_id_gen(xxx));
    } else {
        return VAL_UNDEFINED;
    }
}

