#include <bsp.h>
#include "device.h"
#include "gpio.h"

static int gpio_init(cupkee_device_t *dev)
{
    hw_gpio_conf_t *conf = hw_gpio_conf_alloc();

    if (conf) {
        dev->data = conf;
        return CUPKEE_OK;
    }
    return -CUPKEE_ERESOURCE;
}

static int gpio_deinit(cupkee_device_t *dev)
{
    hw_gpio_conf_t *conf = (hw_gpio_conf_t *)dev->data;

    if (!conf) {
        return -CUPKEE_EINVAL;
    }

    hw_gpio_disable(conf);
    dev->data = NULL;

    return CUPKEE_OK;
}

static int gpio_enable(cupkee_device_t *dev)
{
    return hw_gpio_enable(dev->data);
}

static int gpio_disable(cupkee_device_t *dev)
{
    return hw_gpio_disable(dev->data);
}

static int gpio_listen(cupkee_device_t *dev)
{
    (void) dev;
    return -CUPKEE_EIMPLEMENT;
}

static int gpio_ignore(cupkee_device_t *dev)
{
    (void) dev;
    return -CUPKEE_EIMPLEMENT;
}

static void group_add_pin(hw_gpio_conf_t *conf, uint8_t pin)
{
    if (hw_gpio_pin_is_valid(pin) && conf->pin_num < GPIO_GROUP_SIZE) {
        conf->pins[conf->pin_num++] = pin;
    }
}

static val_t gpio_config_get_sel(cupkee_device_t *dev, env_t *env)
{
    hw_gpio_conf_t *conf = (hw_gpio_conf_t *)dev->data;
    val_t pins[GPIO_GROUP_MAX];
    int i;

    if (!conf) {
        return VAL_FALSE;
    }

    for (i = 0; i < conf->pin_num; i++) {
        val_set_number(pins + i, conf->pins[i]);
    }

    return val_mk_array((void *)array_create(env, i, pins));
}

static val_t gpio_config_set_sel(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    hw_gpio_conf_t *conf = (hw_gpio_conf_t *)dev->data;

    (void) env;
    if (!conf) {
        return VAL_FALSE;
    }

    conf->pin_num = 0;
    if (val_is_number(setting)) {
        group_add_pin(conf, val_2_double(setting));
    } else
    if (val_is_array(setting)) {
        val_t *v;
        int i = 0;

        while (NULL != (v = _array_element(setting, i++))) {
            if (val_is_number(v)) {
                group_add_pin(conf, val_2_double(v));
            }
        }

        if (i != conf->pin_num + 1) {
            // some invalid pin set exist
            conf->pin_num = 0;
        }
    }

    return conf->pin_num ? VAL_TRUE : VAL_FALSE;
}

static const char * const gpio_modes[] = {
    "push-pull", "open-drain", "analog", "floating", "pull-down", "pull-up"
};
static const char * const gpio_dirs[] = {
    "out", "in", "dual"
};

static val_t gpio_config_set_dir(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    hw_gpio_conf_t *conf = (hw_gpio_conf_t *)dev->data;
    const char *s = val_2_cstring(setting);
    unsigned dir;

    (void) env;

    if (!s || !conf) {
        return VAL_UNDEFINED;
    }

    for (dir = 0; dir < sizeof(gpio_dirs)/sizeof(const char *); dir++) {
        if (strcmp(s, gpio_dirs[dir])) {
            continue;
        }

        conf->dir = dir;
        return VAL_TRUE;
    }
    return VAL_UNDEFINED;
}

static val_t gpio_config_get_dir(cupkee_device_t *dev, env_t *env)
{
    hw_gpio_conf_t *conf = (hw_gpio_conf_t *)dev->data;

    (void) env;

    if (!conf) {
        return VAL_UNDEFINED;
    } else {
        uint8_t dir = conf->dir;
        return val_mk_static_string((intptr_t)gpio_dirs[dir]);
    }
}

static val_t gpio_config_set_mod(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    hw_gpio_conf_t *conf = (hw_gpio_conf_t *)dev->data;
    const char *s = val_2_cstring(setting);
    unsigned mod;

    (void) env;

    if (!s || !conf) {
        return VAL_UNDEFINED;
    }

    for (mod = 0; mod < sizeof(gpio_modes)/sizeof(const char *); mod++) {
        if (strcmp(s, gpio_modes[mod])) {
            continue;
        }

        conf->mod = mod;
        return VAL_TRUE;
    }
    return VAL_UNDEFINED;
}

static val_t gpio_config_get_mod(cupkee_device_t *dev, env_t *env)
{
    hw_gpio_conf_t *conf = (hw_gpio_conf_t *)dev->data;

    (void) env;

    if (!conf) {
        return VAL_UNDEFINED;
    } else {
        int mod = conf->mod;
        return val_mk_static_string((intptr_t)gpio_modes[mod]);
    }
}

static val_t gpio_config_set_speed(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    hw_gpio_conf_t *conf = (hw_gpio_conf_t *)dev->data;

    (void) env;

    if (!conf || !val_is_number(setting)) {
        return VAL_UNDEFINED;
    }

    int speed = val_2_double(setting);
    if (speed > MAX_GPIO_SPEED || speed < MIN_GPIO_SPEED) {
        return VAL_FALSE;
    }

    conf->speed = speed;
    return VAL_TRUE;
}

static val_t gpio_config_get_speed(cupkee_device_t *dev, env_t *env)
{
    hw_gpio_conf_t *conf = (hw_gpio_conf_t *)dev->data;

    (void) env;

    if (!conf) {
        return VAL_UNDEFINED;
    }

    return val_mk_number(conf->speed);
}

static val_t gpio_config(cupkee_device_t *dev, env_t *env, const char *name, val_t *setting)
{
    if (!name) {
        return VAL_FALSE;
    }

    if (!strcmp(name, "select")) {
        if (setting) {
            return gpio_config_set_sel(dev, env, setting);
        } else {
            return gpio_config_get_sel(dev, env);
        }
    } else
    if (!strcmp(name, "dir")) {
        if (setting) {
            return gpio_config_set_dir(dev, env, setting);
        } else {
            return gpio_config_get_dir(dev, env);
        }
    } else
    if (!strcmp(name, "mode")){
        if (setting) {
            return gpio_config_set_mod(dev, env, setting);
        } else {
            return gpio_config_get_mod(dev, env);
        }
    } else
    if (!strcmp(name, "speed")) {
        if (setting) {
            return gpio_config_set_speed(dev, env, setting);
        } else {
            return gpio_config_get_speed(dev, env);
        }
    } else {
        return VAL_UNDEFINED;
    }
}

static val_t gpio_write(cupkee_device_t *dev, val_t *data)
{
    uint32_t d;

    if (!val_is_number(data)) {
        return VAL_FALSE;
    }

    d = val_2_double(data);
    if (hw_gpio_write(dev->data, d) > 0) {
        return VAL_TRUE;
    } else {
        return VAL_FALSE;
    }
}

static val_t gpio_read(cupkee_device_t *dev)
{
    uint32_t d;

    if (hw_gpio_read(dev->data, &d) > 0) {
        return val_mk_number(d);
    } else {
        return VAL_FALSE;
    }
}

int gpio_setup(void)
{
    return 0;
}

val_t native_pin(env_t *env, int ac, val_t *av)
{
    int port;
    int pin;

    (void) env;

    if (ac != 2 || !val_is_string(av) || !val_is_number(av+1)) {
        return VAL_FALSE;
    }

    port = *(val_2_cstring(av));
    pin  = val_2_double(av+1);

    if (port >= 'a') {
        port -= 'a';
    } else
    if (port >= 'A') {
        port -= 'A';
    }

    if (port < GPIO_PORT_MAX && pin < GPIO_PIN_MAX && pin >= 0) {
        return val_mk_number((port << 4) + pin);
    } else {
        return VAL_FALSE;
    }
}

// export
cupkee_driver_t cupkee_gpio_driver = {
    .init    = gpio_init,
    .deinit  = gpio_deinit,
    .enable  = gpio_enable,
    .disable = gpio_disable,

    .listen  = gpio_listen,
    .ignore  = gpio_ignore,

    .config  = gpio_config,
    .read    = gpio_read,
    .write   = gpio_write,
};

