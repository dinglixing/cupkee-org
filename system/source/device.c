#include <cupkee.h>

#include "util.h"
#include "device.h"
#include "dev_gpio.h"
#include "dev_adc.h"

#define DEVICE_MAX  (16)


struct driver_entry_t {
    const char *name;
    const cupkee_driver_t *driver;
};

static uint16_t device_magic = 211;
static cupkee_device_t *device_free = NULL;
static cupkee_device_t devices[DEVICE_MAX];

static const struct driver_entry_t drivers[] = {
    {"GPIO", &cupkee_driver_gpio},
    {"ADC",  &cupkee_driver_adc},
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

static inline val_t device_write(cupkee_device_t *dev, val_t *data)
{
    if (0 == (dev->flags & DEV_FL_ENBALE)) {
        return VAL_FALSE;
    }

    return dev->driver->write(dev, data);
}

static inline val_t device_read(cupkee_device_t *dev, int off)
{
    if (0 == (dev->flags & DEV_FL_ENBALE)) {
        return VAL_FALSE;
    }

    return dev->driver->read(dev, off);
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

static inline void device_get_elem(cupkee_device_t *dev, int id, val_t *elem)
{
    (void) dev;
    (void) id;

    *elem = device_read(dev, id);
    //val_set_undefined(elem);
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
            device_get_elem(dev, val_2_integer(av), elem);
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
    if (ac && val_is_string(av)) {
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

    return device_write(dev, av+1);
}

val_t device_native_read(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    (void) env;

    if (ac < 1 || !(dev = device_get(av))) {
        return VAL_UNDEFINED;
    }

    if (ac > 1 && val_is_number(av + 1)) {
        return device_read(dev, val_2_integer(av + 1));
    } else {
        return device_read(dev, -1);
    }

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

