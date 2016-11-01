
#include "hardware.h"
#include <bsp.h>

typedef struct hw_gpio_group_t{
    uint8_t inused;
    uint8_t enable;
    uint8_t listen;
    uint8_t  last;
} hw_gpio_group_t;

static hw_gpio_group_t gpio_grps[GPIO_GROUP_MAX];
static hw_gpio_conf_t *gpio_cfgs[GPIO_GROUP_MAX];
static uint8_t port_inused = 0;
static const uint32_t hw_gpio_ports[] = {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOF
};

static int hw_gpio_group_check(int grp)
{
    if (grp >= 0 && grp < GPIO_GROUP_MAX && gpio_grps[grp].inused) {
        return 0;
    }
    return -1;
}

static uint8_t hw_gpio_port_used(int n, uint8_t *pins)
{
    uint8_t ports = 0;
    int i;

    for (i = 0; i < n; i++) {
        uint8_t port = (pins[i] >> 4) & 0xf;
        ports |= 1 << port;
    }

    return ports;
}

static void hw_gpio_open_ports(uint8_t ports)
{
    if (!ports) {
        return;
    }

    if (ports & 1)   rcc_periph_clock_enable(RCC_GPIOA);
    if (ports & 2)   rcc_periph_clock_enable(RCC_GPIOB);
    if (ports & 4)   rcc_periph_clock_enable(RCC_GPIOC);
    if (ports & 8)   rcc_periph_clock_enable(RCC_GPIOD);
    if (ports & 16)  rcc_periph_clock_enable(RCC_GPIOE);
    if (ports & 32)  rcc_periph_clock_enable(RCC_GPIOF);
    if (ports & 64)  rcc_periph_clock_enable(RCC_GPIOG);
}

static void hw_gpio_close_ports(uint8_t ports)
{
    if (!ports) {
        return;
    }

    if (ports & 1)   rcc_periph_clock_disable(RCC_GPIOA);
    if (ports & 2)   rcc_periph_clock_disable(RCC_GPIOB);
    if (ports & 4)   rcc_periph_clock_disable(RCC_GPIOC);
    if (ports & 8)   rcc_periph_clock_disable(RCC_GPIOD);
    if (ports & 16)  rcc_periph_clock_disable(RCC_GPIOE);
    if (ports & 32)  rcc_periph_clock_disable(RCC_GPIOF);
    if (ports & 64)  rcc_periph_clock_disable(RCC_GPIOG);
}

static void hw_gpio_setup_pin(uint16_t pin, uint8_t mode, uint8_t cnf)
{
    uint32_t port;

    port = (pin >> 4) & 0xf;
    if (port < GPIO_PORT_MAX) {
        port = hw_gpio_ports[port];
    } else {
        return;
    }
    pin  = 1 << (pin & 0xf);

    gpio_set_mode(port, mode, cnf, pin);
}

static void hw_gpio_write_pin(uint16_t pin, int d)
{
    uint32_t port;

    port = (pin >> 4) & 0xf;
    if (port < GPIO_PORT_MAX) {
        port = hw_gpio_ports[port];
    } else {
        return;
    }
    pin  = 1 << (pin & 0xf);

    if (d) {
        gpio_set(port, pin);
    } else {
        gpio_clear(port, pin);
    }
}

static int hw_gpio_read_pin(uint16_t pin)
{
    uint32_t port;

    port = (pin >> 4) & 0xf;
    if (port < GPIO_PORT_MAX) {
        port = hw_gpio_ports[port];
        pin  = 1 << (pin & 0xf);
        return gpio_port_read(port) & pin;
    }
    return 0;
}

static int hw_gpio_config_set(int grp, hw_gpio_conf_t *cfg)
{
    uint8_t ports = hw_gpio_port_used(cfg->pin_num, cfg->pins);
    uint8_t cnf, mode;

    switch(cfg->mod) {
    case OPT_GPIO_MOD_INPUT_FLOAT:       cnf = GPIO_CNF_INPUT_FLOAT; break;
    case OPT_GPIO_MOD_INPUT_PULL_UPDOWN: cnf = GPIO_CNF_INPUT_PULL_UPDOWN; break;
    case OPT_GPIO_MOD_OUTPUT_PUSHPULL:   cnf = GPIO_CNF_OUTPUT_PUSHPULL; break;
    case OPT_GPIO_MOD_OUTPUT_OPENDRAIN:
    case OPT_GPIO_MOD_DUAL:              cnf = GPIO_CNF_OUTPUT_OPENDRAIN; break;
    default: return -1;
    }
    if (cfg->mod >= OPT_GPIO_MOD_OUTPUT_PUSHPULL) {
        if (cfg->speed <= 2) {
            mode = GPIO_MODE_OUTPUT_2_MHZ;
        } else
        if (cfg->speed <= 10) {
            mode = GPIO_MODE_OUTPUT_10_MHZ;
        } else {
            mode = GPIO_MODE_OUTPUT_50_MHZ;
        }
    } else {
        mode = GPIO_MODE_INPUT;
    }
    // hardware setting code here

    if (ports) {
        int i;

        hw_gpio_open_ports(ports ^ (ports & port_inused));
        for (i = 0; i < cfg->pin_num; i++) {
            hw_gpio_setup_pin(cfg->pins[i], mode, cnf);
        }

        port_inused |= ports;
    }
    gpio_cfgs[grp] = cfg;

    return 0;
}

static int hw_gpio_config_clr(int grp)
{
    int i;
    hw_gpio_conf_t *conf;

    port_inused = 0;
    for (i = 0; i < GPIO_GROUP_MAX; i++) {
        if (i == grp) {
            continue;
        }
        conf = gpio_cfgs[i];
        if (conf) {
            port_inused |= hw_gpio_port_used(conf->pin_num, conf->pins);
        }
    }

    conf = gpio_cfgs[grp];
    if (conf) {
        uint8_t ports = hw_gpio_port_used(conf->pin_num, conf->pins);
        hw_gpio_close_ports(ports ^ (ports & port_inused));
        for (i = 0; i < conf->pin_num; i++) {
            hw_gpio_setup_pin(conf->pins[i], GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT);
        }
        gpio_cfgs[grp] = NULL;
    }

    return 0;
}

int hw_gpio_setup(void)
{
    int i;

    port_inused = 0;
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

    //console_puts("gpio pull\r\n");
    for (i = 0; i < GPIO_GROUP_MAX; i++) {
        hw_gpio_group_t *grp = gpio_grps + i;

        if (grp->listen) {
            uint32_t d;
            if (hw_gpio_read(i, &d) > 0) {
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

int hw_gpio_group_release(int grp)
{
    if (hw_gpio_group_check(grp)) {
        gpio_grps[grp].inused = 0;
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

int hw_gpio_enable(int grp, hw_gpio_conf_t *cfg)
{
    if (hw_gpio_group_check(grp)) {
        return -1;
    }

    if (!gpio_grps[grp].enable) {
        if (0 == hw_gpio_config_set(grp, cfg)) {
            gpio_grps[grp].enable = 1;
            return 0;
        }
    }
    return -1;
}

int hw_gpio_disable(int grp)
{
    if (hw_gpio_group_check(grp)) {
        return -1;
    }

    if (gpio_grps[grp].enable) {
        if (0 == hw_gpio_config_clr(grp)) {
            gpio_grps[grp].enable = 0;
            return 0;
        }
    }
    return -1;
}

int hw_gpio_write(int grp, uint32_t data)
{
    hw_gpio_conf_t *conf;
    int i;

    if (hw_gpio_group_check(grp)) {
        return -1;
    }
    conf = gpio_cfgs[grp];

    for (i = 0; i < conf->pin_num; i++) {
        hw_gpio_write_pin(conf->pins[i], data & 1);
        data >>= 1;
    }

    return 1;
}

int hw_gpio_read(int grp, uint32_t *data)
{
    hw_gpio_conf_t *conf;
    int i;
    uint32_t d;

    if (hw_gpio_group_check(grp)) {
        return -1;
    }
    conf = gpio_cfgs[grp];
    d = 0;

    for (i = conf->pin_num - 1; i >= 0; i--) {
        d <<= 1;
        if (hw_gpio_read_pin(conf->pins[i])) {
            d |= 1;
        }
    }
    *data = d;

    return 1;
}

int hw_gpio_event_enable(int grp, int event)
{
    if (hw_gpio_group_check(grp) || event >= GPIO_EVENT_MAX) {
        return -1;
    }

    gpio_grps[grp].listen |= (1 << event);

    return 0;
}

int hw_gpio_event_disable(int grp, int event)
{
    if (hw_gpio_group_check(grp) || event >= GPIO_EVENT_MAX) {
        return -1;
    }

    gpio_grps[grp].listen &= ~(1 << event);

    return 0;
}
