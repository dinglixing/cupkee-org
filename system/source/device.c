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

#if 0
#define _TRACE(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define _TRACE(fmt, ...)    //
#endif

#define DEVICE_MAX          (16)
#define DEVICE_ENABLE       0x80
#define DEVICE_EVNET_MASK   0x0f

typedef struct cupkee_device_t {
    uint16_t magic;
    uint8_t  flags;
    uint8_t  inst;
    const hw_device_t *desc;
    const hw_driver_t *driver;
    val_t    *event_handle[DEVICE_EVENT_MAX];
    struct cupkee_device_t *next;
} cupkee_device_t;

static const char *device_event_names[] = {
    "error", "data", "drain", "ready"
};

static int device_is_true(intptr_t id);
static void device_op_prop(void *env, intptr_t devid, val_t *name, val_t *prop);
static void device_op_elem(void *env, intptr_t devid, val_t *name, val_t *prop);
static cupkee_device_t device_queue[DEVICE_MAX];
static cupkee_device_t *device_free = NULL;
static cupkee_device_t *device_work = NULL;
static uint16_t device_magic_factor = 1;
static const val_foreign_op_t device_op = {
    .is_true = device_is_true,
    .prop = device_op_prop,
    .elem = device_op_elem,
};


static int device_is_true(intptr_t devid)
{
    int magic;

    magic = devid & 0xffff;
    devid >>= 16 & 0xff;

    if (devid >= DEVICE_MAX || device_queue[devid].magic != magic) {
        return 0;
    } else {
        return 1;
    }
}

static inline intptr_t device_id_gen(cupkee_device_t *device) {
    return ((device - device_queue) << 16) | device->magic;
}

static inline cupkee_device_t *device_id_control(intptr_t devid)
{
    int magic;

    magic = devid & 0xffff;
    devid >>= 16 & 0xff;

    if (devid >= DEVICE_MAX || device_queue[devid].magic != magic) {
        return NULL;
    } else {
        return device_queue + devid;
    }
}

static cupkee_device_t *device_val_control(val_t *d)
{
    val_foreign_t *vf;

    if (val_is_foreign(d)) {
        vf = (val_foreign_t *)val_2_intptr(d);
        if (vf->op == &device_op) {
            return device_id_control(vf->data);
        }
    }
    return NULL;
}

static cupkee_device_t *device_alloc(void)
{
    cupkee_device_t *device = device_free;

    if (device) {
        int i;
        device_free = device->next;

        for (i = 0; i < DEVICE_EVENT_MAX; i++) {
            device->event_handle[i] = NULL;
        }

        device->flags = 0;
        device->magic = (device_magic_factor + 1) ^ (device_magic_factor << 8);
        device_magic_factor++;
    }
    return device;
}

static int device_release(cupkee_device_t *device)
{
    if (device) {
        device->driver->release(device->desc->id, device->inst);

        memset(device, 0, sizeof(cupkee_device_t));
        device->next = device_free;
        device_free = device;
        return 0;
    }

    return -1;
}

static cupkee_device_t *device_create(const char *name, int instance)
{
    cupkee_device_t  *device;
    const hw_device_t *desc;
    const hw_driver_t *driver;

    desc = hw_device_take(name, instance, &driver);
    if (!desc) {
        return NULL;
    }

    device = device_alloc();
    if (device) {
        device->inst = instance;
        device->desc = desc;
        device->driver = driver;
    } else {
        driver->release(desc->id, instance);
    }

    return device;
}

static void device_work_list_join(cupkee_device_t *device)
{
    device->next = device_work;
    device_work = device;
}

static void device_work_list_drop(cupkee_device_t *device)
{
    cupkee_device_t *cur = device_work;

    if (cur == device) {
        device_work = cur->next;
        return;
    }

    while(cur) {
        cupkee_device_t *next = cur->next;

        if (next == device) {
            cur->next = device->next;
            return;
        }

        cur = next;
    }
}

static void device_enable(cupkee_device_t *device)
{
    if (!(device->flags & DEVICE_ENABLE)) {
        if (device->driver->enable(device->desc->id, device->inst)) {
            device->flags |= DEVICE_ENABLE;
            device_work_list_join(device);
        } else {
            //Todo: set error
        }
    }
}

static void device_disable(cupkee_device_t *device)
{
    if (device->flags & DEVICE_ENABLE) {
        if (device->driver->disable(device->desc->id, device->inst)) {
            device->flags &= ~DEVICE_ENABLE;
            device_work_list_drop(device);
        } else {
            //Todo: set error
        }
    }
}

static int device_str_map(const char *name, int max, const char **names)
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

static inline int device_val_map(val_t *in, int max, const char **names)
{
    if (val_is_number(in)) {
        return val_2_integer(in);
    } else {
        return device_str_map(val_2_cstring(in), max, names);
    }
}

/* return:
 *  0: ok
 * -1: invalid value
 */
static int device_setting_convert(const cupkee_device_t *device, const hw_config_desc_t *desc, val_t *value, int *setting)
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
        *setting = cupkee_id(value, desc->opt_num, device->desc->opt_names + desc->opt_start);
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
static int device_config_set(cupkee_device_t *device, int which, val_t *value)
{
    const hw_config_desc_t *conf;
    int setting;

    if (which < 0 || which >= device->desc->conf_num) {
        return -3;
    }

    conf = &device->desc->conf_descs[which];
    if (device_setting_convert(device, conf, value, &setting)) {
        return -2;
    }

    if (device->driver->config_set(device->desc->id, device->inst, which, setting)) {
        return 0;
    } else {
        return -1;
    }
}

static inline val_t device_config_set_one(cupkee_device_t *device, val_t *name, val_t *value)
{
    int err = device_config_set(device, device_val_map(name, device->desc->conf_num, device->desc->conf_names), value);

    if (!err) {
        return VAL_TRUE;
    }
    if (err == -3) {
        return VAL_UNDEFINED;
    }
    return VAL_FALSE;
}

static val_t device_config_set_all(cupkee_device_t *device, val_t *value)
{
    if (val_is_object(value)) {
        object_iter_t it;
        const char *k;
        val_t *v;

        _object_iter_init(&it, (object_t *)val_2_intptr(value));
        while (_object_iter_next(&it, &k, &v)) {
            int which = device_str_map(k, device->desc->conf_num, device->desc->conf_names);
            if (device_config_set(device, which, v)) {
                return VAL_FALSE;
            }
        }
        return VAL_TRUE;
    }
    return VAL_FALSE;
}

static val_t device_config_get(cupkee_device_t *device, val_t *name)
{
    int which = cupkee_id(name, device->desc->conf_num, device->desc->conf_names);
    const hw_config_desc_t *conf;
    int setting;

    if (which >= device->desc->conf_num) {
        return VAL_UNDEFINED;
    }

    if (!device->driver->config_get(device->desc->id, device->inst, which, &setting)) {
        return VAL_UNDEFINED;
    }

    conf = &device->desc->conf_descs[which];
    switch(conf->type) {
    case HW_CONFIG_BOOL:
        return setting ? VAL_TRUE : VAL_FALSE;
    case HW_CONFIG_NUM:
        return val_mk_number(setting);
    case HW_CONFIG_OPT:
        return val_mk_foreign_string((intptr_t)device->desc->opt_names[conf->opt_start + setting]);
    default:
        return VAL_UNDEFINED;
    }
}

static val_t device_config_get_all(cupkee_device_t *device)
{
    (void) device;
    return VAL_UNDEFINED;
}

static val_t device_native_destroy(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;

    (void) env;

    if (ac && (device = device_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_FALSE;
    }

    if (0 != device_release(device)) {
        return VAL_FALSE;
    } else {
        return VAL_TRUE;
    }
}

static val_t device_native_config(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;
    val_t *name = NULL;
    val_t *setting;

    (void) env;

    if (ac && (device = device_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (ac && (val_is_number(av) || val_is_string(av))) {
        name = av++; ac--;
    }

    setting = ac > 0 ? av : NULL;

    if (setting) {
        return name ? device_config_set_one(device, name, setting) :
                      device_config_set_all(device, setting);
    } else {
        return name ? device_config_get(device, name) :
                      device_config_get_all(device);
    }
}

static val_t device_native_enable(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;
    val_t *setting, *hnd;

    (void) env;

    if (ac && (device = device_val_control(av)) != NULL) {
        hnd = av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    setting = ac > 0 ? av : NULL;

    if (setting && val_is_object(setting)) {
        device_config_set_all(device, setting);
        ac--; av++;
    }
    device_enable(device);

    if (ac && val_is_function(av)) {
        val_t args[2];
        int err = device->driver->get_err(device->desc->id, device->inst);
        args[0] = err ? val_mk_number(err) : VAL_UNDEFINED;
        args[1] = *hnd;

        cupkee_do_callback(env, av, 2, args);
    }

    return (device->flags & DEVICE_ENABLE) ? VAL_TRUE : VAL_FALSE;
}

static val_t device_native_disable(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;

    (void) env;

    if (ac == 0 || (device = device_val_control(av)) == NULL) {
        return VAL_UNDEFINED;
    } else {
        device_disable(device);
        return (device->flags & DEVICE_ENABLE) ? VAL_FALSE : VAL_TRUE;
    }
}

static val_t device_native_is_enable(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;

    (void) env;

    if (ac == 0 || (device = device_val_control(av)) == NULL) {
        return VAL_UNDEFINED;
    } else {
        return (device->flags & DEVICE_ENABLE) ? VAL_TRUE : VAL_FALSE;
    }
}

static val_t device_native_listen(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;
    val_t *callback;
    int event_id;

    (void) env;

    if (ac >= 3 && (device = device_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    callback = av + 1;
    event_id = device_val_map(av, DEVICE_EVENT_MAX, device_event_names);
    if (event_id >= device->desc->event_num || !val_is_function(callback)) {
        return VAL_FALSE;
    }

    if (device->event_handle[event_id] == NULL) {
        val_t *ref = reference_create(callback);

        if (!ref) {
            return VAL_FALSE;
        }
        device->event_handle[event_id] = ref;
    } else {
        // origion ref released, automic
        *device->event_handle[event_id] = *callback;
    }

    device->driver->listen(device->desc->id, device->inst, event_id);
    return VAL_TRUE;
}

static int device_map_read_all(cupkee_device_t *device, env_t *env, val_t *data)
{
    uint32_t v;

    // support combine read
    if (device->driver->io.map.get(device->desc->id, device->inst, -1, &v)) {
        val_set_number(data, v);
    } else {
        int i, size;
        array_t *a;

        // not support combine read
        // scan all
        size = device->driver->io.map.size(device->desc->id, device->inst);
        a = _array_create(env, size);
        if (!a) {
            return -CUPKEE_ERESOURCE;
        }

        for (i = 0; i < size; i++) {
            if (device->driver->io.map.get(device->desc->id, device->inst, i, &v)) {
               val_set_number(_array_elem(a, i), v);
            } else {
                val_set_undefined(_array_elem(a, i));
            }
        }
        val_set_array(data, (intptr_t)a);
    }

    return 0;
}

static val_t device_native_read_map(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;
    int off;
    uint32_t v;

    (void) env;

    if (ac && (device = device_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (!(device->flags & DEVICE_ENABLE)) {
        return VAL_UNDEFINED;
    }

    if (!device_param_int(ac, av, &off)) {
        off = -1;
    }

    if (off >= 0) {
        if (device->driver->io.map.get(device->desc->id, device->inst, off, &v)) {
            return val_mk_number(v);
        }
    } else {
        val_t all;
        if (0 == device_map_read_all(device, env, &all)) {
            return all;
        }
    }
    return VAL_UNDEFINED;
}

static int device_stream_recv_data(cupkee_device_t *device, env_t *env, int n, val_t *data)
{
    type_buffer_t *buffer = buffer_create(env, n);

    if (!buffer) {
        return -CUPKEE_ERESOURCE;
    } else {
        n = device->driver->io.stream.recv(device->desc->id, device->inst, n, buffer->buf);

        val_set_buffer(data, buffer);

        return n;
    }
}

static val_t device_native_read_stream(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;
    int n, size, err;

    if (ac && (device = device_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (!(device->flags & DEVICE_ENABLE)) {
        return VAL_FALSE;
    }

    size = device->driver->io.stream.received(device->desc->id, device->inst);
    if (size < 0) {
        err = device->driver->get_err(device->desc->id, device->inst);
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
        val_t args[2];

        if (n > size) {
            n = size;
        }
        err = device_stream_recv_data(device, env, n, &args[1]);
        if (err < 0) {
            goto DO_ERROR;
        }
        val_set_undefined(args);
        cupkee_do_callback(env, av, 2, args);
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

static val_t device_native_read_block(env_t *env, int ac, val_t *av)
{
    (void) env;
    (void) ac;
    (void) av;
    return VAL_UNDEFINED;
}

static val_t device_native_write_map(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;
    int off, val;

    (void) env;

    if (ac && (device = device_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (!(device->flags & DEVICE_ENABLE)) {
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

    if (device->driver->io.map.set(device->desc->id, device->inst, off, (uint32_t)val)) {
        return VAL_TRUE;
    } else {
        return VAL_FALSE;
    }
}

static val_t device_native_write_stream(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;
    val_t cb_args[3];
    void *addr;
    int   size, n, err = 0;

    if (ac && (device = device_val_control(av)) != NULL) {
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

    n = device->driver->io.stream.send(device->desc->id, device->inst, n, addr);
    if (n < 0) {
        err = device->driver->get_err(device->desc->id, device->inst);
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

static val_t device_native_write_block(env_t *env, int ac, val_t *av)
{
    (void) env;
    (void) ac;
    (void) av;
    return VAL_FALSE;
}

static inline void device_get_map_elem(cupkee_device_t *device, env_t *env, int off, val_t *elem)
{
    uint32_t v;

    (void) env;

    if (device->desc->type == HW_DEVICE_MAP
        && (device->flags & DEVICE_ENABLE)
        && device->driver->io.map.get(device->desc->id, device->inst, off, &v)) {
        val_set_number(elem, v);
    } else {
        val_set_undefined(elem);
    }
}

static void device_op_prop(void *env, intptr_t devid, val_t *name, val_t *prop)
{
    cupkee_device_t *device = device_id_control(devid);
    const char *prop_name = val_2_cstring(name);

    (void) env;

    if (device && prop_name) {
        if (!strcmp(prop_name, "read")) {
            switch (device->desc->type) {
            case HW_DEVICE_MAP:    val_set_native(prop, (intptr_t)device_native_read_map);    return;
            case HW_DEVICE_STREAM: val_set_native(prop, (intptr_t)device_native_read_stream); return;
            case HW_DEVICE_BLOCK:  val_set_native(prop, (intptr_t)device_native_read_block);  return;
            default: val_set_undefined(prop); return;
            }
        } else
        if (!strcmp(prop_name, "write")) {
            switch (device->desc->type) {
            case HW_DEVICE_MAP:    val_set_native(prop, (intptr_t)device_native_write_map);    return;
            case HW_DEVICE_STREAM: val_set_native(prop, (intptr_t)device_native_write_stream); return;
            case HW_DEVICE_BLOCK:  val_set_native(prop, (intptr_t)device_native_write_block);  return;
            default: val_set_undefined(prop); return;
            }
        } else
        if (!strcmp(prop_name, "config")) {
            val_set_native(prop, (intptr_t)device_native_config);
            return;
        } else
        if (!strcmp(prop_name, "enable")) {
            val_set_native(prop, (intptr_t)device_native_enable);
            return;
        } else
        if (!strcmp(prop_name, "disable")) {
            val_set_native(prop, (intptr_t)device_native_disable);
            return;
        } else
        if (!strcmp(prop_name, "isEnable")) {
            val_set_native(prop, (intptr_t)device_native_is_enable);
            return;
        } else
        if (!strcmp(prop_name, "listen")) {
            val_set_native(prop, (intptr_t)device_native_listen);
            return;
        } else
        if (!strcmp(prop_name, "destroy")) {
            val_set_native(prop, (intptr_t)device_native_destroy);
            return;
        }
    }

    val_set_undefined(prop);
}

static void device_op_elem(void *env, intptr_t devid, val_t *idx, val_t *elem)
{
    if (val_is_number(idx)) {
        cupkee_device_t *device = device_id_control(devid);

        device_get_map_elem (device, env, val_2_integer(idx), elem);
    } else {
        device_op_prop(env, devid, idx, elem);

    }
}

static void device_error_proc(cupkee_device_t *device, env_t *env)
{

    int err = device->driver->get_err(device->desc->id, device->inst);

    cupkee_do_callback_error(env, device->event_handle[0], err);
}

static void device_map_data_proc(cupkee_device_t *device, env_t *env)
{
    val_t data;
    int err;

    err = device_map_read_all(device, env, &data);
    if (err) {
        cupkee_do_callback_error(env, device->event_handle[DEVICE_EVENT_ERR], err);
        return;
    }
    cupkee_do_callback(env, device->event_handle[DEVICE_EVENT_DATA], 1, &data);
}

static void device_stream_data_proc(cupkee_device_t *device, env_t *env)
{
    val_t data;
    int err, n;

    n = device->driver->io.stream.received(device->desc->id, device->inst);
    if (n > 0) {
        err = device_stream_recv_data(device, env, n, &data);
        if (err < 0) {
            cupkee_do_callback_error(env, device->event_handle[DEVICE_EVENT_ERR], err);
        } else {
            cupkee_do_callback(env, device->event_handle[DEVICE_EVENT_DATA], 1, &data);
        }
    }
}

static void device_data_proc(cupkee_device_t *device, env_t *env)
{
    switch(device->desc->type) {
    case HW_DEVICE_MAP:     device_map_data_proc(device, env);    break;
    case HW_DEVICE_STREAM:  device_stream_data_proc(device, env); break;
    case HW_DEVICE_BLOCK:
    default:                break;
    }
}

void device_event_proc(env_t *env, int event)
{
    cupkee_device_t *device;
    uint8_t id, inst, e;

    e = event & 0xff;

    if (e >= DEVICE_EVENT_MAX) {
        return;
    }
    id   = (event >> 16) & 0xff;
    inst = (event >> 8) & 0xff;
    device = device_work;

    while (device) {
        if (device->inst == inst && device->desc->id == id) {
            break;
        }
        device = device->next;
    }

    if (!device || !device->event_handle[e]) {
        return;
    }

    switch(e) {
    case DEVICE_EVENT_ERR:   device_error_proc(device, env); break;
    case DEVICE_EVENT_DATA:  device_data_proc(device, env);  break;
    case DEVICE_EVENT_DRAIN: cupkee_do_callback(env, device->event_handle[e], 0, NULL); break;
    case DEVICE_EVENT_READY: break;
    default: break; // what happen ?
    }
}

void device_setup(void)
{
    int i;

    device_free = NULL;
    device_work = NULL;
    for (i = 0; i < DEVICE_MAX; i++) {
        cupkee_device_t *device = device_queue + i;

        device->magic = 0;
        device->flags = 0;
        device->next = device_free;

        device_free = device;
    }

    return;
}

val_t device_native_create(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device = NULL;
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

    device = device_create(name, instance);
    if (device) {
        return val_create(env, &device_op, device_id_gen(device));
    } else {
        return VAL_UNDEFINED;
    }
}

val_t device_native_led(env_t *env, int ac, val_t *av)
{
    (void) env;

    if (ac > 0) {
        if (val_is_true(av)) {
            hw_led_set();
        } else {
            hw_led_clear();
        }
    } else {
        hw_led_toggle();
    }
    return VAL_UNDEFINED;
}

