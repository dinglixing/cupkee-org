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

#include "hardware.h"

#define PORT_MAX                7
#define PIN_MASK                15
#define PIN_MAX                 16
#define PIN_MAP_MAX             16

#define PIN_DEVICE_MOD          0
#define PIN_DEVICE_MOD_OUT      0
#define PIN_DEVICE_MOD_IN       1
#define PIN_DEVICE_MOD_DUAL     2

static const uint32_t hw_gpio_port_rcc[PORT_MAX] = {
    RCC_GPIOA, RCC_GPIOB, RCC_GPIOC, RCC_GPIOD, RCC_GPIOE, RCC_GPIOF, RCC_GPIOF
};
static const uint32_t hw_gpio_port_base[PORT_MAX] = {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG
};
static uint16_t hw_gpio_pin_use[PORT_MAX];
static uint8_t  hw_gpio_pin_map[PIN_MAP_MAX];

static inline int hw_gpio_map_is_valid(uint8_t map)
{
    return map != 0xff;
}

static inline uint8_t hw_gpio_map_id(int port, int pin)
{
    return (port << 4) | (pin & PIN_MASK);
}

static inline int hw_gpio_map_port(uint8_t map)
{
    return (map >> 4) & PORT_MAX;
}

static inline int hw_gpio_map_pin(uint8_t map)
{
    return map & PIN_MASK;
}

static inline void hw_gpio_pin_write(int port, int pin, int v) {
    if (v)
        GPIO_BSRR(hw_gpio_port_base[port]) = 1 << pin;
    else
        GPIO_BSRR(hw_gpio_port_base[port]) = 1 << (pin + 16);
}

static inline int hw_gpio_pin_read(int port, int pin) {
    return (GPIO_IDR(hw_gpio_port_base[port]) & (1 << pin)) != 0;
}

static void hw_led_setup(void)
{
    if (hw_gpio_map_is_valid(hw_gpio_pin_map[0])) {
        int port = hw_gpio_map_port(hw_gpio_pin_map[0]);
        int pin  = hw_gpio_map_pin(hw_gpio_pin_map[0]);

        hw_gpio_use(port, 1 << pin);

        gpio_set_mode(hw_gpio_port_base[port], GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, 1 << pin);
    }
}

static void hw_led_reset(int port, int pin)
{
    if (hw_gpio_map_is_valid(hw_gpio_pin_map[0])) {
        // release origin map
        hw_gpio_release(hw_gpio_map_port(hw_gpio_pin_map[0]),
                        1 << hw_gpio_map_pin(hw_gpio_pin_map[0]));
    }

    hw_gpio_use(port, 1 << pin);
    gpio_set_mode(hw_gpio_port_base[port], GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, 1 << pin);
}

void hw_led_set(void)
{
    gpio_set(GPIOA, GPIO8);
}

void hw_led_clear(void)
{
    gpio_clear(GPIOA, GPIO8);
}

void hw_led_toggle(void)
{
    gpio_toggle(GPIOA, GPIO8);
}

/******************************************************************
 * GPIO device: PIN
 ******************************************************************/
static const char *pin_device_config_names[] = {
    "pinStart", "pinNum", "dir"
};
static const char *pin_device_opt_names[] = {
    "out", "in", "dual"
};
static const hw_config_desc_t pin_device_config_descs[] = {
    {
        .type = HW_CONFIG_NUM,
    },
    {
        .type = HW_CONFIG_NUM,
    },
    {
        .type = HW_CONFIG_OPT,
        .opt_num = 3,
        .opt_start = 0,
    }
};

static uint8_t pin_device_used = 0;
static uint8_t pin_device_work = 0;
static uint8_t pin_device_event_settings[PIN_INSTANCE_NUM];
static uint8_t pin_device_config_settings[PIN_INSTANCE_NUM][PIN_CONFIG_NUM];
static uint32_t pin_device_data[PIN_INSTANCE_NUM];
static int pin_device_error[PIN_INSTANCE_NUM];
static int pin_device_get(int id, int inst, int off, uint32_t *v);

static inline int pin_device_is_inused(int inst) {
    return (inst < PIN_INSTANCE_NUM) ? pin_device_used & (1 << inst) : 0;
}

static inline int pin_device_is_work(int inst) {
    return (inst < PIN_INSTANCE_NUM) ?
        (pin_device_used & pin_device_work) & (1 << inst) : 0;
}

static inline void pin_device_set_inused(int inst) {
    pin_device_used |= (1 << inst);
}

static inline void pin_device_set_work(int inst) {
    pin_device_work |= (1 << inst);
}

static inline void pin_device_clr_inused(int inst) {
    pin_device_used &= ~(1 << inst);
}

static inline void pin_device_clr_work(int inst) {
    pin_device_work &= ~(1 << inst);
}

static void pin_device_set_error(int inst, int error)
{
    pin_device_error[inst] = error;

    if (pin_device_event_settings[inst] & 1) {
        devices_event_post(PIN_DEVICE_ID, inst, 0);
    }
}

static int pin_device_get_error(int id, int inst)
{
    (void) id;
    if (inst >= PIN_INSTANCE_NUM) {
        return 0;
    }

    return pin_device_error[inst];
}

static int pin_device_setup(int inst)
{
    int pin_bgn = pin_device_config_settings[inst][0];
    int pin_num = pin_device_config_settings[inst][1];
    uint8_t cnf, mod;
    int i = 0;

    (void) inst;

    if (   pin_bgn == 0                         // PinMap[0] use as led, always
        || pin_bgn >= PIN_MAP_MAX
        || pin_num == 0
        || (pin_bgn + pin_num) >= PIN_MAP_MAX) {
        goto DO_ERROR;
    }

    switch(pin_device_config_settings[inst][2]) {
    case PIN_DEVICE_MOD_OUT:
        mod = GPIO_MODE_OUTPUT_2_MHZ;
        cnf = GPIO_CNF_OUTPUT_PUSHPULL;
        break;
    case PIN_DEVICE_MOD_IN:
        mod = GPIO_MODE_INPUT;
        cnf = GPIO_CNF_INPUT_PULL_UPDOWN;
        break;
    case PIN_DEVICE_MOD_DUAL:
        mod = GPIO_MODE_OUTPUT_2_MHZ;
        cnf = GPIO_CNF_OUTPUT_OPENDRAIN;
        break;
    default: goto DO_ERROR;
    }

    while (i < pin_num) {
        uint8_t map = hw_gpio_pin_map[pin_bgn + i++];

        if (!hw_gpio_map_is_valid(map)) {
            goto DO_ERROR;
        }

        hw_gpio_use(hw_gpio_map_port(map), 1 << hw_gpio_map_pin(map));
        gpio_set_mode(hw_gpio_port_base[hw_gpio_map_port(map)], mod, cnf, 1 << hw_gpio_map_pin(map));
    }
    return 1;

DO_ERROR:
    while(i >= 0) {
        uint8_t map = hw_gpio_pin_map[pin_bgn + i--];

        hw_gpio_release(hw_gpio_map_port(map), 1 << hw_gpio_map_pin(map));
    }
    pin_device_set_error(inst, CUPKEE_ESETTINGS);

    return 0;
}

static int pin_device_reset(int inst)
{
    uint8_t pin_bgn = pin_device_config_settings[inst][0];
    uint8_t pin_num = pin_device_config_settings[inst][1];
    int i = 0;

    if (   pin_bgn == 0                         // PinMap[0] use as led, always
        || pin_num == 0
        || (pin_bgn + pin_num) < PIN_MAP_MAX) {
        return 0;
    }

    while (i < pin_num) {
        uint8_t map = hw_gpio_pin_map[pin_bgn + i++];

        if (hw_gpio_map_is_valid(map)) {
            hw_gpio_release(hw_gpio_map_port(map), 1 << hw_gpio_map_pin(map));
        }
    }
    return 0;
}

static int pin_device_enable(int id, int inst)
{
    (void) id;
    if (pin_device_is_inused(inst)) {
        if (!pin_device_is_work(inst)) {
            pin_device_set_work(inst);

            return pin_device_setup(inst);
        }
        return 1;
    }
    return 0;
}

static int pin_device_disable(int id, int inst)
{
    (void) id;
    if (pin_device_is_inused(inst)) {
        if (pin_device_is_work(inst)) {
            pin_device_clr_work(inst);

            return pin_device_reset(inst);
        } else {
            return 1;
        }
    }
    return 0;
}

// 0: fail
// 1: ok
static int pin_device_request(int id, int inst)
{
    (void) id;
    if (inst < PIN_INSTANCE_NUM) {
        if (pin_device_is_inused(inst)) {
            return 0;
        } else {
            int c;

            pin_device_error[inst] = 0;
            pin_device_event_settings[inst] = 0;
            pin_device_set_inused(inst);
            pin_device_clr_work(inst);

            for (c = 0; c < PIN_CONFIG_NUM; c++) {
                pin_device_config_settings[inst][c] = 0;
            }

            return 1;
        }
    }

    return 0;
}

// 0: fail
// other: ok
static int pin_device_release(int id, int inst)
{
    if (pin_device_is_inused(inst)) {
        pin_device_disable(id, inst);
        pin_device_clr_inused(inst);
        return 1;
    } else {
        return 0;
    }
}

static int pin_device_config_set(int id, int inst, int which, int setting)
{
    (void) id;
    if (pin_device_is_inused(inst) && which < PIN_CONFIG_NUM) {
        pin_device_config_settings[inst][which] = setting;
        return 1;
    }
    return 0;
}

static int pin_device_config_get(int id, int inst, int which, int *setting)
{
    (void) id;
    if (pin_device_is_inused(inst) && which < PIN_CONFIG_NUM && setting) {
        *setting = pin_device_config_settings[inst][which];
        return 1;
    }
    return 0;
}

static void pin_device_listen(int id, int inst, int event)
{
    (void) id;
    if (pin_device_is_inused(inst)
        && event < PIN_EVENT_NUM
        && (pin_device_config_settings[inst][PIN_DEVICE_MOD] >= PIN_DEVICE_MOD_IN)) {

        pin_device_event_settings[inst] |= 1 << event;
        if (event == DEVICE_EVENT_DATA) {
            pin_device_get(id, inst, -1, pin_device_data + inst);
        }
    }
}

static void pin_device_ignore(int id, int inst, int event)
{
    (void) id;
    if (pin_device_is_inused(inst) && event < PIN_EVENT_NUM) {
        pin_device_event_settings[inst] &= ~(1 << event);
    }
}

static int pin_device_get(int id, int inst, int off, uint32_t *v)
{
    int pin_bgn = pin_device_config_settings[inst][0];
    int pin_num = pin_device_config_settings[inst][1];
    uint32_t data;

    (void) id;

    if (off >= pin_num) {
        return 0;
    } else
    if (off >= 0) {
        uint8_t map = hw_gpio_pin_map[pin_bgn + off];
        data = hw_gpio_pin_read(hw_gpio_map_port(map), hw_gpio_map_pin(map));
    } else {
        int i;

        data = 0;
        for (i = pin_num - 1; i >= 0; i--) {
            uint8_t map = hw_gpio_pin_map[pin_bgn + i];

            data <<= 1;
            data |= hw_gpio_pin_read(hw_gpio_map_port(map), hw_gpio_map_pin(map));
        }
    }

    *v = data;
    return 1;
}

static int pin_device_set(int id, int inst, int off, uint32_t val)
{
    int pin_bgn = pin_device_config_settings[inst][0];
    int pin_num = pin_device_config_settings[inst][1];

    (void) id;

    if (off >= pin_num) {
        return 0;
    } else
    if (off < 0) {
        int i;

        for (i = 0; i < pin_num; i++) {
            uint8_t map = hw_gpio_pin_map[pin_bgn + i];
            hw_gpio_pin_write(hw_gpio_map_port(map), hw_gpio_map_pin(map), val & (1 << i));
        }
    } else {
        if (off < pin_num) {
            uint8_t map = hw_gpio_pin_map[pin_bgn + off];
            hw_gpio_pin_write(hw_gpio_map_port(map), hw_gpio_map_pin(map), val);
        }
    }

    return 1;
}

static int pin_device_size(int id, int inst)
{
    uint8_t pin_bgn = pin_device_config_settings[inst][0];
    uint8_t pin_num = pin_device_config_settings[inst][1];

    (void) id;

    if (   pin_bgn == 0                         // PinMap[0] use as led, always
        || pin_num == 0
        || (pin_bgn + pin_num) < PIN_MAP_MAX) {
        return 0;
    }

    return pin_num;
}

const hw_driver_t hw_driver_pin = {
    .request = pin_device_request,
    .release = pin_device_release,
    .get_err = pin_device_get_error,
    .enable  = pin_device_enable,
    .disable = pin_device_disable,
    .config_set = pin_device_config_set,
    .config_get = pin_device_config_get,
    .listen = pin_device_listen,
    .ignore = pin_device_ignore,
    .io.map = {
        .get = pin_device_get,
        .set = pin_device_set,
        .size = pin_device_size,
    }
};

const hw_device_t hw_device_pin = {
    .name = PIN_DEVICE_NAME,
    .id   = PIN_DEVICE_ID,
    .type = HW_DEVICE_MAP,
    .inst_num   = PIN_INSTANCE_NUM,
    .conf_num   = PIN_CONFIG_NUM,
    .event_num  = PIN_EVENT_NUM,
    .conf_names = pin_device_config_names,
    .conf_descs = pin_device_config_descs,
    .opt_names  = pin_device_opt_names,
};

int hw_setup_gpio(void)
{
    int i;

    for (i = 0; i < PORT_MAX; i++) {
        hw_gpio_pin_use[i] = 0;
    }

    for (i = 0; i < PIN_MAP_MAX; i++) {
        // set invalid value to all pin map
        hw_gpio_pin_map[i] = 0xff;
    }

    hw_led_setup();

    pin_device_used = 0;
    pin_device_work = 0;

    return 0;
}

void hw_poll_gpio(void)
{
    int i;

    for (i = 0; i < PIN_INSTANCE_NUM; i++) {
        if (pin_device_is_work(i)
            && (pin_device_event_settings[i] & (1 << DEVICE_EVENT_DATA))) {

            uint32_t v;
            if (pin_device_get(PIN_DEVICE_ID, i, -1, &v) && pin_device_data[i] != v) {
                pin_device_data[i] = v;
                devices_event_post(PIN_DEVICE_ID, i, DEVICE_EVENT_DATA);
            }
        }
    }
}

int hw_pin_map(int id, int port, int pin)
{
    if (id >= PIN_MAP_MAX || port >= PORT_MAX || pin >= PIN_MAP_MAX) {
        return CUPKEE_ERROR;
    }

    if (id == 0) {
        hw_led_reset(port, pin);
    }
    hw_gpio_pin_map[id] = hw_gpio_map_id(port, pin);

    return CUPKEE_OK;
}

int hw_gpio_use(int port, uint16_t pins)
{
    if (port >= PORT_MAX || (hw_gpio_pin_use[port] & pins)) {
        return 0;
    }
    if (!hw_gpio_pin_use[port]) {
        rcc_periph_clock_enable(hw_gpio_port_rcc[port]);
    }
    hw_gpio_pin_use[port] |= pins;
    return 1;
}

int hw_gpio_release(int port, uint16_t pins)
{
    if (port >= PORT_MAX) {
        return 0;
    }

    hw_gpio_pin_use[port] &= ~pins;
    if (!hw_gpio_pin_use[port]) {
        rcc_periph_clock_disable(hw_gpio_port_rcc[port]);
    }
    return 1;
}


