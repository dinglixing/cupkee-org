#include <bsp.h>

#include "device.h"
#include "gpio.h"
#include "misc.h"

typedef struct gpio_group_ctrl_t {
    int             group;
    val_t          *events_handle[GPIO_EVENT_MAX];
    hw_gpio_conf_t  conf;
} gpio_group_ctrl_t;

static gpio_group_ctrl_t controls[GPIO_GROUP_MAX];
static const char * const gpio_modes[] = {
    "push-pull", "open-drain", "analog", "floating", "pull-down", "pull-up"
};
static const char * const gpio_dirs[] = {
    "out", "in", "dual"
};
static const char * const gpio_events[] = {
    "change"
};

static int gpio_init(cupkee_device_t *dev)
{
    int grp = hw_gpio_group_alloc();

    if (grp >= 0 && grp < GPIO_GROUP_MAX) {
        gpio_group_ctrl_t *control = &controls[grp];
        int i;

        hw_gpio_conf_reset(&control->conf);
        control->group = grp;
        for (i = 0; i < GPIO_EVENT_MAX; i++) {
            control->events_handle[i] = NULL;
        }

        dev->data = control;
        return CUPKEE_OK;
    }
    return -CUPKEE_ERESOURCE;
}

static int gpio_deinit(cupkee_device_t *dev)
{
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;

    if (!control) {
        return -CUPKEE_EINVAL;
    }

    hw_gpio_disable(control->group);
    dev->data = NULL;

    return CUPKEE_OK;
}

static int gpio_enable(cupkee_device_t *dev)
{
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    return hw_gpio_enable(control->group, &control->conf);
}

static int gpio_disable(cupkee_device_t *dev)
{
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    return hw_gpio_disable(control->group);
}

static int gpio_listen(cupkee_device_t *dev, val_t *event, val_t *callback)
{
    int event_id;

    if (val_is_number(event)) {
        event_id = val_2_double(event);
    } else {
        const char *event_name = val_2_cstring(event);
        event_id = GPIO_EVENT_MAX;

        if (event_name) {
            unsigned i;
            for (i = 0; i < sizeof(gpio_events)/sizeof(const char *); i++) {
                if (!strcmp(event_name, gpio_events[i])) {
                    event_id = i;
                    break;
                }
            }
        }
    }

    if (event_id >= GPIO_EVENT_MAX) {
        return -CUPKEE_EINVAL;
    }
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;

    if (hw_gpio_event_enable(control->group, event_id)) {
        return -CUPKEE_ERROR;
    }

    if (control->events_handle[event_id] == NULL) {
        val_t *ref = reference_create(callback);

        if (!ref) {
            return -CUPKEE_ERESOURCE;
        }
        control->events_handle[event_id] = ref;
    } else {
        *control->events_handle[event_id] = *callback;
    }

    return CUPKEE_OK;
}

static int gpio_ignore(cupkee_device_t *dev, val_t *event)
{
    (void) dev;
    (void) event;

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
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    if (!control) {
        return VAL_FALSE;
    }

    hw_gpio_conf_t *conf = &control->conf;
    val_t pins[GPIO_GROUP_MAX];
    int i;

    for (i = 0; i < conf->pin_num; i++) {
        val_set_number(pins + i, conf->pins[i]);
    }

    return val_mk_array((void *)array_create(env, i, pins));
}

static val_t gpio_config_set_sel(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    if (!control) {
        return VAL_FALSE;
    }

    (void) env;

    hw_gpio_conf_t *conf = &control->conf;

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

static val_t gpio_config_set_dir(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    const char *s = val_2_cstring(setting);
    unsigned dir;

    (void) env;

    if (!s || !control) {
        return VAL_UNDEFINED;
    }
    hw_gpio_conf_t *conf = &control->conf;

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
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;

    (void) env;

    if (!control) {
        return VAL_UNDEFINED;
    }

    hw_gpio_conf_t *conf = &control->conf;
    uint8_t dir = conf->dir;

    return val_mk_static_string((intptr_t)gpio_dirs[dir]);
}

static val_t gpio_config_set_mod(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    const char *s = val_2_cstring(setting);
    unsigned mod;

    (void) env;

    if (!s || !control) {
        return VAL_UNDEFINED;
    }

    hw_gpio_conf_t *conf = &control->conf;
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
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    if (!control) {
        return VAL_UNDEFINED;
    }

    (void) env;

    hw_gpio_conf_t *conf = &control->conf;
    int mod = conf->mod;
    return val_mk_static_string((intptr_t)gpio_modes[mod]);
}

static val_t gpio_config_set_speed(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    if (!control || !val_is_number(setting)) {
        return VAL_UNDEFINED;
    }

    (void) env;

    int speed = val_2_double(setting);

    if (speed > MAX_GPIO_SPEED || speed < MIN_GPIO_SPEED) {
        return VAL_FALSE;
    }

    control->conf.speed = speed;
    return VAL_TRUE;
}

static val_t gpio_config_get_speed(cupkee_device_t *dev, env_t *env)
{
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    if (!control) {
        return VAL_UNDEFINED;
    }

    (void) env;

    return val_mk_number(control->conf.speed);
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
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    uint32_t d = val_2_double(data);

    if (control->conf.dir != OPT_GPIO_DIR_IN &&
        val_is_number(data) && hw_gpio_write(control->group, d) > 0) {
        return VAL_TRUE;
    } else {
        return VAL_FALSE;
    }
}

static val_t gpio_read(cupkee_device_t *dev)
{
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    uint32_t d;

    if (control->conf.dir != OPT_GPIO_DIR_OUT && hw_gpio_read(control->group, &d) > 0) {
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

