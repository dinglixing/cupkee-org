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
    uint16_t  last;
} hw_gpio_group_t;

static hw_gpio_group_t gpio_grps[GPIO_GROUP_MAX];
static hw_gpio_conf_t  gpio_cfgs[GPIO_GROUP_MAX];

static hw_gpio_group_t *hw_gpio_group_get(hw_gpio_conf_t *conf)
{
    int i;

    for (i = 0; i < GPIO_GROUP_MAX; i++) {
        if (gpio_cfgs + i == conf) {
            return gpio_grps + i;
        }
    }
    return NULL;
}

int hw_gpio_setup(void)
{
    int i;

    for (i = 0; i < GPIO_GROUP_MAX; i++) {
        gpio_grps[i].inused = 0;
        gpio_grps[i].enable = 0;
        gpio_grps[i].last   = 0;
    }

    return 0;
}

void hw_gpio_poll(void)
{
}

hw_gpio_conf_t *hw_gpio_conf_alloc(void)
{
    int i;

    for (i = 0; i < GPIO_GROUP_MAX; i++) {
        if (gpio_grps[i].inused == 0) {

            gpio_grps[i].inused = 1;
            gpio_grps[i].enable = 0;
            gpio_grps[i].last = 0;

            hw_gpio_conf_reset(gpio_cfgs + i);

            return gpio_cfgs + i;
        }
    }

    return NULL;
}

void hw_gpio_conf_release(hw_gpio_conf_t *conf)
{
    hw_gpio_group_t *grp = hw_gpio_group_get(conf);

    if (grp) {
        grp->inused = 0;
    }
}

void hw_gpio_conf_reset(hw_gpio_conf_t *conf)
{
    conf->pin_num = 0;
    conf->dir = OPT_GPIO_DIR_OUT;
    conf->mod = OPT_GPIO_MOD_PUSHPULL;
    conf->speed = MIN_GPIO_SPEED;
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

int hw_gpio_enable(hw_gpio_conf_t *conf)
{
    hw_gpio_group_t *grp = hw_gpio_group_get(conf);

    if (!grp) {
        return -1;
    }

    if (!grp->enable) {
        grp->enable = 1;

        //reset gpio group hw here
    }

    return 0;
}

int hw_gpio_disable(hw_gpio_conf_t *conf)
{
    hw_gpio_group_t *grp = hw_gpio_group_get(conf);

    if (!grp) {
        return -1;
    }

    if (grp->enable) {
        grp->enable = 0;

        //reset gpio group hw here
    }

    return 0;
}

int hw_gpio_write(hw_gpio_conf_t *conf, uint32_t data)
{
    int i;

    for (i = 0; i < conf->pin_num; i++) {
        uint8_t pin = conf->pins[i];
        uint8_t port = pin >> 4;

        dbg_gpio_pins[port][pin & 0xf] = (data & 1);
        data >>= 1;
    }

    return 1;
}

int hw_gpio_read(hw_gpio_conf_t *conf, uint32_t *data)
{
    int i;
    uint32_t d = 0;

    for (i = 0; i < conf->pin_num; i++) {
        uint8_t pin = conf->pins[i];
        uint8_t port = pin >> 4;

        d <<= 1;
        d += dbg_gpio_pins[port][pin & 0xf];
    }
    *data = d;

    return 1;
}

