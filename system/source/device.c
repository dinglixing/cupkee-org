#include <cupkee.h>

#include "util.h"
#include "device.h"
#include "gpio.h"

#define DEVICE_MAX  (16)


struct driver_entry_t {
    const char *name;
    const cupkee_driver_t *driver;
};

static const struct driver_entry_t drivers[] = {
    {"GPIO", &cupkee_gpio_driver},
};
static uint16_t device_magic = 211;
static cupkee_device_t *device_free = NULL;
static cupkee_device_t devices[DEVICE_MAX];

static cupkee_device_t *device_get(val_t *d)
{
    int devid;
    int magic;

    if (!val_is_number(d)) {
        return NULL;
    }
    devid = val_2_double(d);
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

static inline int device_id(cupkee_device_t *dev) {
    return ((dev - devices) << 16) | dev->magic;
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
    if (dev->flags & DEV_FL_ENBALE && setting) {
        return VAL_FALSE;
    }
    return dev->driver->config(dev, env, name, setting);
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

static inline val_t device_read(cupkee_device_t *dev)
{
    if (0 == (dev->flags & DEV_FL_ENBALE)) {
        return VAL_FALSE;
    }

    return dev->driver->read(dev);
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

    gpio_setup();

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
    val_t *device = NULL;
    val_t *settings = NULL;
    val_t *callback = NULL;
    val_t gen[2];
    int err = 0;

    if (ac == 0) {
        return device_name_list(env);
    } else {
        if (val_is_string(av)) {
            device = av;
        }
        ac--; av++;
    }

    if (device && ac) {
        if (val_is_function(av)) {
            callback = av;
        } else {
            settings = av;
            if (ac > 1 && val_is_function(av + 1)) {
                callback = av + 1;
            }
        }
    }

    if (device && 0 == (err = device_alloc(val_2_cstring(device), &dev))) {
        if (settings) {
           err = device_config(dev, env, NULL, settings);
        }
    }

    if (err) {
        gen[0] = cupkee_error(env, err);
        gen[1] = VAL_UNDEFINED;
    } else {
        gen[0] = VAL_UNDEFINED;
        gen[1] = val_mk_number(device_id(dev));
    }

    if (callback) {
        cupkee_do_callback(env, callback, 2, gen);
    }

    return gen[1];
}

val_t native_config(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;
    val_t *name;
    val_t *setting;

    if (ac == 0 || !(dev = device_get(av))) {
        return VAL_UNDEFINED;
    }

    if (ac > 1) {
        name = av + 1;
    } else {
        name = NULL;
    }

    if (ac > 2) {
        setting = av + 2;
    } else {
        setting = NULL;
    }

    return device_config(dev, env, name, setting);
}

val_t native_enable(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    (void) env;

    if (ac == 0 || !(dev = device_get(av))) {
        return VAL_UNDEFINED;
    }

    if (ac > 1) {
        int err;
        if (val_is_true(av + 1)) {
            err = device_enable(dev);
        } else {
            err = device_disable(dev);
        }
        return (err == CUPKEE_OK) ? VAL_TRUE : VAL_FALSE;
    } else {
        return (dev->flags & DEV_FL_ENBALE) ? VAL_TRUE : VAL_FALSE;
    }
}

val_t native_write(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    (void) env;

    if (ac < 2 || !(dev = device_get(av))) {
        return VAL_UNDEFINED;
    }

    return device_write(dev, av+1);
}

val_t native_read(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    (void) env;

    if (ac < 1 || !(dev = device_get(av))) {
        return VAL_UNDEFINED;
    }

    return device_read(dev);
}

val_t native_listen(env_t *env, int ac, val_t *av)
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

val_t native_ignore(env_t *env, int ac, val_t *av)
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

