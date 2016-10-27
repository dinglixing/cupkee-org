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
    uint32_t devid;
    uint16_t magic;

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

static val_t device_config_get(cupkee_device_t *dev, env_t *env, val_t *which)
{
    (void) dev;

    if (which) {
        const char *item = val_2_cstring(which);

        if (!strcmp(item, "select")) {
            intptr_t sel = array_create(env, 0, NULL);
            if (sel) {
                return val_mk_array((void *)sel);
            } else {
                // return error_create(env, code, msg);
                return val_mk_undefined();
            }
        } else
        if (!strcmp(item, "dir")) {
            return val_mk_static_string((intptr_t)"out");
        } else
        if (!strcmp(item, "mode")) {
            return val_mk_static_string((intptr_t)"push-pull");
        } else
        if (!strcmp(item, "enable")) {
            return val_mk_boolean(0);
        } else {
            return val_mk_undefined();
        }
    } else {
        // all items compose a dictionary
        return val_mk_undefined();
    }
}

static val_t device_config_set(cupkee_device_t *dev, env_t *env, val_t *which, val_t *setting)
{
    (void) dev;
    (void) env;
    (void) which;
    (void) setting;

    return val_mk_boolean(1);
}

void device_setup(void)
{
    int i;

    device_free = NULL;
    for (i = 0; i < DEVICE_MAX; i++) {
        cupkee_device_t *cd = devices + i;

        cd->magic = 0;
        cd->driver = NULL;
        cd->next = device_free;

        device_free = cd;
    }

    return;
}

static val_t device_name_list(env_t *env)
{
    void *names = array_create(env, 0, NULL);

    if (names) {
        return val_mk_array(names);
    } else {
        return val_mk_undefined();
    }
}

static int32_t device_init(cupkee_device_t *dev, cupkee_driver_t *driver)
{
    int32_t id = ((dev - devices) << 16) | device_magic;

    dev->magic = device_magic++;
    dev->driver = driver;
    dev->next = NULL;

    if (driver->init(dev) != CUPKEE_OK) {
        dev->flags |= DEV_FL_ERROR;
    }

    return id;
}

static int32_t device_alloc(const char *name)
{
    int i = 0;

    if (!device_free) {
        return -CUPKEE_ERESOURCE;
    }

    while (i < sizeof(drivers)/sizeof(struct driver_entry_t)) {
        if (!strcmp(name, drivers[i].name)) {
            cupkee_device_t *dev = device_free;

            device_free = dev->next;
            return device_init(dev, drivers[i].driver);
        }

        i++;
    }

    return -CUPKEE_ENAME;
}

static int device_config(cupkee_device_t *dev, env_t *env, val_t *name, val_t *val)
{
    return -CUPKEE_EIMPLEMENT;
}

static int device_config_multi(cupkee_device_t *dev, env_t *env, val_t *settings)
{
    return CUPKEE_OK;
}

val_t native_device(env_t *env, int ac, val_t *av)
{
    int32_t dev_id = -1;
    val_t *device = NULL;
    val_t *settings = NULL;
    val_t *callback = NULL;
    val_t gen[2] = {TAG_UNDEFINED, TAG_UNDEFINED};
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

    if (device) {
        dev_id = device_alloc(val_2_cstring(device));
    }

    if (dev_id < 0){
        err = dev_id;
    } else {
        gen[1] = val_mk_number(dev_id);
        if (settings) {
           err = device_config_multi(dev_id, env, settings);
        }
    }

    if (callback) {
        if (err) {
            cupkee_error(gen, env, err);
        }
        cupkee_do_callback(env, callback, 2, &gen);
    }

    return gen[1];
}

val_t native_config(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    if (ac == 0 || !(dev = device_get(av))) {
        return TAG_UNDEFINED;
    } else {
        av++; ac--;
    }

    if (ac == 0) {
        return device_config_get(dev, env, NULL);
    } else {
        if (ac == 1) {
            if (val_is_string(av)) {
                return device_config_get(dev, env, av);
            } else {
                return device_config_set(dev, env, NULL, av);
            }
        } else {
            return device_config_set(dev, env, av, av + 1);
        }
    }
}

