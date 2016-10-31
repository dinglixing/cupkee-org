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
static const char *gpio_confs[] = {
    "select", "dir", "mode", "speed"
};
static const char *gpio_modes[] = {
    "push-pull", "open-drain", "analog", "floating", "pull-down", "pull-up"
};
static const char *gpio_dirs[] = {
    "out", "in", "dual"
};
static const char *gpio_events[] = {
    "change"
};

static void gpio_add_pin(hw_gpio_conf_t *conf, uint8_t pin);

static val_t get_select(cupkee_device_t *dev, env_t *env)
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

static val_t set_select(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    if (!control) {
        return VAL_FALSE;
    }

    (void) env;

    hw_gpio_conf_t *conf = &control->conf;

    conf->pin_num = 0;
    if (val_is_number(setting)) {
        gpio_add_pin(conf, val_2_double(setting));
    } else
    if (val_is_array(setting)) {
        val_t *v;
        int i = 0;

        while (NULL != (v = _array_element(setting, i++))) {
            if (val_is_number(v)) {
                gpio_add_pin(conf, val_2_double(v));
            }
        }

        if (i != conf->pin_num + 1) {
            // some invalid pin set exist
            conf->pin_num = 0;
        }
    }

    return conf->pin_num ? VAL_TRUE : VAL_FALSE;
}

static val_t set_dir(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    gpio_group_ctrl_t *control = (gpio_group_ctrl_t*)dev->data;
    int dir = cupkee_id(setting, 3, gpio_dirs);

    if (control && dir < OPT_GPIO_DIR_MAX) {
        control->conf.dir = dir;
        return VAL_TRUE;
    }
    return VAL_UNDEFINED;

    (void) env;
}

static val_t get_dir(cupkee_device_t *dev, env_t *env)
{
    gpio_group_ctrl_t *control = (gpio_group_ctrl_t*)dev->data;

    if (!control) {
        return VAL_UNDEFINED;
    }

    return val_mk_static_string((intptr_t)gpio_dirs[control->conf.dir]);

    (void) env;
}

static val_t set_mode(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    gpio_group_ctrl_t *control = (gpio_group_ctrl_t*)dev->data;
    int mod = cupkee_id(setting, OPT_GPIO_MOD_MAX, gpio_modes);

    if (control && mod < OPT_GPIO_MOD_MAX) {
        control->conf.mod = mod;
        return VAL_TRUE;
    }
    return VAL_UNDEFINED;

    (void) env;
}

static val_t get_mode(cupkee_device_t *dev, env_t *env)
{
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;
    if (!control) {
        return VAL_UNDEFINED;
    }

    return val_mk_static_string((intptr_t)gpio_modes[control->conf.mod]);

    (void) env;
}

static val_t set_speed(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    gpio_group_ctrl_t *control = (gpio_group_ctrl_t*)dev->data;
    if (!control || !val_is_number(setting)) {
        return VAL_UNDEFINED;
    }

    int speed = val_2_double(setting);

    if (speed > MAX_GPIO_SPEED || speed < MIN_GPIO_SPEED) {
        return VAL_FALSE;
    }

    control->conf.speed = speed;
    return VAL_TRUE;

    (void) env;
}

static val_t get_speed(cupkee_device_t *dev, env_t *env)
{
    gpio_group_ctrl_t *control = (gpio_group_ctrl_t*)dev->data;
    if (!control) {
        return VAL_UNDEFINED;
    } else {
        return val_mk_number(control->conf.speed);
    }

    (void) env;
}

static device_config_handle_t config_handles[] = {
    {set_select,    get_select},
    {set_dir,       get_dir},
    {set_mode,      get_mode},
    {set_speed,     get_speed},
};

/************************************************************************************
 * GPIO driver interface
 ***********************************************************************************/
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
    int event_id = cupkee_id(event, GPIO_EVENT_MAX, gpio_events);

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
    int event_id = cupkee_id(event, GPIO_EVENT_MAX, gpio_events);

    if (event_id >= GPIO_EVENT_MAX) {
        return -CUPKEE_EINVAL;
    }
    gpio_group_ctrl_t *control= (gpio_group_ctrl_t*)dev->data;

    if (hw_gpio_event_disable(control->group, event_id)) {
        return -CUPKEE_ERROR;
    }

    if (control->events_handle[event_id]) {
        reference_release(control->events_handle[event_id]);
    }

    return CUPKEE_OK;
}

static val_t gpio_config(cupkee_device_t *dev, env_t *env, val_t *name, val_t *setting)
{
    int id;

    if (!name) {
        // not support
        return VAL_FALSE;
    }

    id = cupkee_id(name, 4, gpio_confs);
    if (id < 4) {
        if (setting) {
            return config_handles[id].setter(dev, env, setting);
        } else {
            return config_handles[id].getter(dev, env);
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

static void gpio_event_handle(env_t *env, uint8_t which, uint8_t event)
{
    if (which < GPIO_GROUP_MAX) {
        if (event == GPIO_EVENT_CHANGE) {
            cupkee_do_callback(env, controls[which].events_handle[GPIO_EVENT_CHANGE], 0, NULL);
        }
    }
}

static void gpio_add_pin(hw_gpio_conf_t *conf, uint8_t pin)
{
    if (hw_gpio_pin_is_valid(pin) && conf->pin_num < GPIO_GROUP_SIZE) {
        conf->pins[conf->pin_num++] = pin;
    }
}

/************************************************************************************
 * GPIO driver exports
 ***********************************************************************************/
cupkee_driver_t cupkee_gpio_driver = {
    .init    = gpio_init,
    .deinit  = gpio_deinit,
    .enable  = gpio_enable,
    .disable = gpio_disable,

    .config  = gpio_config,
    .read    = gpio_read,
    .write   = gpio_write,

    .listen  = gpio_listen,
    .ignore  = gpio_ignore,
    .event_handle = gpio_event_handle,
};

/************************************************************************************
 * GPIO native
 ***********************************************************************************/
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

int gpio_setup(void)
{
    return 0;
}

