/*******************************************************************************
 * dbg field
*******************************************************************************/
#include "hardware.h"

static int dbg_gpio_pins[8][16];

int hw_dbg_gpio_get_pin(int port, int pin)
{
    return dbg_gpio_pins[port][pin];
}

int hw_dbg_gpio_clr_pin(int port, int pin)
{
    return dbg_gpio_pins[port][pin] = 0;
}

int hw_dbg_gpio_set_pin(int port, int pin)
{
    return dbg_gpio_pins[port][pin] = 1;
}

/*******************************************************************************
 * hw field
*******************************************************************************/
#include <bsp.h>
typedef struct hw_gpio_group_t{
    uint8_t inused;
    uint8_t enable;
    uint8_t listen;
    uint8_t  last;
} hw_gpio_group_t;

static hw_gpio_group_t gpio_grps[GPIO_GROUP_MAX];
static hw_gpio_conf_t *gpio_cfgs[GPIO_GROUP_MAX];

static int hw_gpio_group_check(int instance)
{
    if (instance >= 0 && instance < GPIO_GROUP_MAX && gpio_grps[instance].inused) {
        return 0;
    }
    return -1;
}

static int hw_gpio_config_set(int instance, hw_gpio_conf_t *cfg)
{
    // hardware setting code here
    gpio_cfgs[instance] = cfg;
    return 0;
}

static int hw_gpio_config_clr(int instance)
{
    // hardware clear setting code here
    gpio_cfgs[instance] = NULL;
    return 0;
}

int hw_gpio_setup(void)
{
    int i;

    for (i = 0; i < GPIO_GROUP_MAX; i++) {
        gpio_grps[i].inused = 0;
        gpio_grps[i].enable = 0;
        gpio_grps[i].listen = 0;
        gpio_grps[i].last   = 0;
    }

    return 0;
}

void hw_gpio_poll(void)
{
    int i;

    for (i = 0; i < GPIO_GROUP_MAX; i++) {
        hw_gpio_group_t *grp = gpio_grps + i;

        if (grp->listen) {
            uint32_t d;
            if (hw_gpio_read(i, -1, &d) > 0) {
                if (d != grp->last) {
                    devices_event_post(GPIO_DEVICE_ID, i, GPIO_EVENT_CHANGE);
                    grp->last = d;
                }
            }
        }
    }
}

int hw_gpio_group_alloc(void)
{
    int i;

    for (i = 0; i < GPIO_GROUP_MAX; i++) {
        if (gpio_grps[i].inused == 0) {

            gpio_grps[i].inused = 1;
            gpio_grps[i].enable = 0;
            gpio_grps[i].last = 0;

            return i;
        }
    }

    return -1;
}

int hw_gpio_group_release(int instance)
{
    if (hw_gpio_group_check(instance)) {
        gpio_grps[instance].inused = 0;
        return 0;
    }
    return -1;
}

void hw_gpio_conf_reset(hw_gpio_conf_t *conf)
{
    conf->pin_num = 0;
    conf->mod = OPT_GPIO_MOD_INPUT_FLOAT;
    conf->speed = OPT_GPIO_SPEED_MIN;
}

int hw_gpio_pin_is_valid(uint8_t pin)
{
    int port = pin >> 4;

    pin &= 0x0f;

    if (port < 8) {
        return 1;
    }

    return 0;
}

int hw_gpio_enable(int instance, hw_gpio_conf_t *cfg)
{
    if (hw_gpio_group_check(instance)) {
        return -1;
    }

    if (!gpio_grps[instance].enable) {
        if (0 == hw_gpio_config_set(instance, cfg)) {
            gpio_grps[instance].enable = 1;
            return 0;
        }
    }
    return -1;
}

int hw_gpio_disable(int instance)
{
    if (hw_gpio_group_check(instance)) {
        return -1;
    }

    if (gpio_grps[instance].enable) {
        if (0 == hw_gpio_config_clr(instance)) {
            gpio_grps[instance].enable = 0;
            return 0;
        }
    }
    return -1;
}

int hw_gpio_write(int instance, int off, uint32_t data)
{
    hw_gpio_conf_t *conf;

    if (hw_gpio_group_check(instance)) {
        return -1;
    }

    conf = gpio_cfgs[instance];
    if (!GPIO_WRITEABLE(conf->mod)) {
        return -1;
    }

    if (off < 0) {
        int i;
        for (i = 0; i < conf->pin_num; i++) {
            uint8_t pin = conf->pin_seq[i];
            uint8_t port = pin >> 4;

            dbg_gpio_pins[port][pin & 0xf] = (data & 1);
            data >>= 1;
        }
    } else
    if (off < conf->pin_num) {
        uint8_t pin = conf->pin_seq[off];
        uint8_t port = pin >> 4;

        dbg_gpio_pins[port][pin & 0xf] = (data & 1);
    } else {
        return 0;
    }

    return 1;
}

int hw_gpio_read(int instance, int off, uint32_t *data)
{
    hw_gpio_conf_t *conf;
    int i;
    uint32_t d = 0;

    if (hw_gpio_group_check(instance)) {
        return -1;
    }
    conf = gpio_cfgs[instance];
    if (!GPIO_READABLE(conf->mod)) {
        return -1;
    }

    if (off < 0) {
        for (i = conf->pin_num - 1; i >= 0; i--) {
            uint8_t pin = conf->pin_seq[i];
            uint8_t port = pin >> 4;

            d <<= 1;
            d += dbg_gpio_pins[port][pin & 0xf];
        }
    } else
    if (off < conf->pin_num) {
        uint8_t pin = conf->pin_seq[off];
        uint8_t port = pin >> 4;
        d = dbg_gpio_pins[port][pin & 0xf];
    } else {
        return 0;
    }

    *data = d;

    return 1;
}

int hw_gpio_event_enable(int instance, int event)
{
    if (hw_gpio_group_check(instance) || event >= GPIO_EVENT_MAX) {
        return -1;
    }

    gpio_grps[instance].listen |= (1 << event);

    return 0;
}

int hw_gpio_event_disable(int instance, int event)
{
    if (hw_gpio_group_check(instance) || event >= GPIO_EVENT_MAX) {
        return -1;
    }

    gpio_grps[instance].listen &= ~(1 << event);

    return 0;
}
