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


static inline uint8_t hw_gpio_map_id(int port, int pin)
{
    return (port << 4) | (pin & 15);
}

static inline uint8_t hw_gpio_map_port(uint8_t map)
{
    return (map >> 4) & PORT_MAX;
}

static inline uint8_t hw_gpio_map_pin(uint8_t map)
{
    return map & PIN_MAX;
}

static void hw_led_setup(void)
{
    int port = hw_gpio_map_port(hw_gpio_pin_map[0]);
    int pin  = hw_gpio_map_pin(hw_gpio_pin_map[0]);

    hw_gpio_use(port, 1 << pin);

    gpio_set_mode(hw_gpio_port_base[port], GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, 1 << pin);
}

static void hw_led_reset(int port, int pin)
{
    hw_gpio_use(port, 1 << pin);

    gpio_set_mode(hw_gpio_port_base[port], GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, 1 << pin);

    // release origin pin
    port = hw_gpio_map_port(hw_gpio_pin_map[0]);
    pin  = hw_gpio_map_pin(hw_gpio_pin_map[0]);
    hw_gpio_release(port, 1 << pin);
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
    "dir"
};
static const char *pin_device_opt_names[] = {
    "out", "in", "dual"
};
static const hw_config_desc_t pin_device_config_descs[] = {
    {
        .type = HW_CONFIG_OPT,
        .opt_num = 3,
        .opt_start = 0,
    }
};

static uint8_t pin_device_used = 0;
static uint8_t pin_device_work = 0;
static uint8_t pin_device_event_settings[PIN_INSTANCE_NUM];
static uint32_t pin_device_value[PIN_INSTANCE_NUM];
static int pin_device_error[PIN_INSTANCE_NUM];
static int pin_device_config_settings[PIN_INSTANCE_NUM][PIN_CONFIG_NUM];
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
    (void) inst;
    uint8_t cnf, mod;

    switch(pin_device_config_settings[inst][0]) {
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
    default: return -1;
    }

    if (hw_gpio_use(2, GPIO2 | GPIO3)) {
        gpio_set_mode(GPIOC, mod, cnf, GPIO2 | GPIO3);
        return 1;
    } else {
        pin_device_set_error(inst, 1);
        return 0;
    }
}

static int pin_device_reset(int inst)
{
    (void) inst;

    return hw_gpio_release(2, GPIO2 | GPIO3);
}

static void pin_device_monitor(void)
{
    int i;

    for (i = 0; i < PIN_INSTANCE_NUM; i++) {
        if (pin_device_is_work(i)
            && (pin_device_event_settings[i] & (1 << DEVICE_EVENT_DATA))) {

            uint32_t v;
            pin_device_get(0, i, -1, &v);
            if (pin_device_value[i] != v) {
                pin_device_value[i] = v;
                devices_event_post(PIN_DEVICE_ID, i, DEVICE_EVENT_DATA);
            }
        }
    }
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
            pin_device_get(id, inst, -1, pin_device_value + inst);
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
    uint32_t data;

    (void) id;
    (void) inst;

    data = gpio_port_read(GPIOC);

    if (off < 0) {
        data = (data >> 2) & 0x3;
    } else {
        data = (data >> (2 + off)) & 1;
    }

    *v = data;
    return 1;
}

static int pin_device_set(int id, int inst, int off, uint32_t val)
{
    (void) id;
    (void) inst;

    if (off < 0) {
        uint16_t set = (val & 3) << 2;
        uint16_t clr = ((~val) & 3) << 2;
        if (set)
            gpio_set(GPIOC, set);
        if (clr)
            gpio_clear(GPIOC, clr);
    } else {
        uint16_t pin = 1 << off;
        if (val) {
            gpio_set(GPIOC, pin);
        } else {
            gpio_clear(GPIOC, pin);
        }
    }

    return 1;
}

static int pin_device_size(int id, int inst)
{
    (void) id;
    (void) inst;
    return 2;
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

/******************************************************************
 * GPIO device: KEY
 ******************************************************************/
static uint8_t key_device_used = 0;
static uint8_t key_device_work = 0;
static uint8_t key_device_event_settings[KEY_INSTANCE_NUM];
static uint32_t key_device_value[KEY_INSTANCE_NUM];
static int key_device_error[KEY_INSTANCE_NUM];
static int key_device_get(int id, int inst, int off, uint32_t *v);

static inline int key_device_is_inused(int inst) {
    return (inst < KEY_INSTANCE_NUM) ? key_device_used & (1 << inst) : 0;
}

static inline int key_device_is_work(int inst) {
    return (inst < KEY_INSTANCE_NUM) ?
        (key_device_used & key_device_work) & (1 << inst) : 0;
}

static inline void key_device_set_inused(int inst) {
    key_device_used |= (1 << inst);
}

static inline void key_device_set_work(int inst) {
    key_device_work |= (1 << inst);
}

static inline void key_device_clr_inused(int inst) {
    key_device_used &= ~(1 << inst);
}

static inline void key_device_clr_work(int inst) {
    key_device_work &= ~(1 << inst);
}

static void key_device_set_error(int inst, int error)
{
    key_device_error[inst] = error;

    if (key_device_event_settings[inst] & 1) {
        devices_event_post(KEY_DEVICE_ID, inst, 0);
    }
}

static int key_device_get_error(int id, int inst)
{
    (void) id;
    if (inst >= KEY_INSTANCE_NUM) {
        return 0;
    }

    return key_device_error[inst];
}

static int key_device_setup(int inst)
{
    (void) inst;
    uint8_t cnf, mod;

    mod = GPIO_MODE_OUTPUT_2_MHZ;
    cnf = GPIO_CNF_OUTPUT_PUSHPULL;

    if (hw_gpio_use(0, GPIO0)) {
        gpio_set_mode(GPIOA, mod, cnf, GPIO0);
        return 1;
    } else {
        key_device_set_error(inst, 1);
        return 0;
    }
}

static int key_device_reset(int inst)
{
    (void) inst;

    return hw_gpio_release(0, GPIO0);
}

static void key_device_monitor(void)
{
    int i;

    for (i = 0; i < KEY_INSTANCE_NUM; i++) {
        if (key_device_is_work(i)
            && (key_device_event_settings[i] & (1 << DEVICE_EVENT_DATA))) {
            uint32_t v;
            key_device_get(0, i, -1, &v);
            if (key_device_value[i] != v) {
                key_device_value[i] = v;
                devices_event_post(KEY_DEVICE_ID, i, DEVICE_EVENT_DATA);
            }
        }
    }
}

static int key_device_enable(int id, int inst)
{
    (void) id;
    if (key_device_is_inused(inst)) {
        if (!key_device_is_work(inst)) {
            key_device_set_work(inst);

            return key_device_setup(inst);
        }
        return 1;
    }
    return 0;
}

static int key_device_disable(int id, int inst)
{
    (void) id;
    if (key_device_is_inused(inst)) {
        if (key_device_is_work(inst)) {
            key_device_clr_work(inst);

            return key_device_reset(inst);
        } else {
            return 1;
        }
    }
    return 0;
}

// 0: fail
// 1: ok
static int key_device_request(int id, int inst)
{
    (void) id;

    if (inst < KEY_INSTANCE_NUM) {
        if (key_device_is_inused(inst)) {
            return 0;
        } else {
            key_device_error[inst] = 0;
            key_device_event_settings[inst] = 0;

            key_device_clr_work(inst);
            key_device_set_inused(inst);

            return 1;
        }
    }

    return 0;
}

// 0: fail
// other: ok
static int key_device_release(int id, int inst)
{
    if (key_device_is_inused(inst)) {
        key_device_disable(id, inst);
        key_device_clr_inused(inst);
        return 1;
    } else {
        return 0;
    }
}

static int key_device_config_set(int id, int inst, int which, int setting)
{
    (void) id;
    (void) inst;
    (void) which;
    (void) setting;
    return 0;
}

static int key_device_config_get(int id, int inst, int which, int *setting)
{
    (void) id;
    (void) inst;
    (void) which;
    (void) setting;
    return 0;
}

static void key_device_listen(int id, int inst, int event)
{
    (void) id;
    if (key_device_is_inused(inst) && event < KEY_EVENT_NUM) {
        key_device_event_settings[inst] |= 1 << event;
        if (event == DEVICE_EVENT_DATA) {
            key_device_get(id, inst, -1, key_device_value + inst);
        }
    } else {
    }
}

static void key_device_ignore(int id, int inst, int event)
{
    (void) id;
    if (key_device_is_inused(inst) && event < PIN_EVENT_NUM) {
        pin_device_event_settings[inst] &= ~(1 << event);
    }
}

static int key_device_get(int id, int inst, int off, uint32_t *v)
{
    uint32_t data;

    (void) id;
    (void) inst;
    (void) off;

    data = gpio_port_read(GPIOA);
    *v = data & 1;

    return 1;
}

static int key_device_set(int id, int inst, int off, uint32_t data)
{
    (void) id;
    (void) inst;
    (void) off;
    (void) data;

    return 0;
}

static int key_device_size(int id, int inst)
{
    (void) id;
    (void) inst;
    return 1;
}

const hw_driver_t hw_driver_key = {
    .request = key_device_request,
    .release = key_device_release,
    .get_err = key_device_get_error,
    .enable  = key_device_enable,
    .disable = key_device_disable,
    .config_set = key_device_config_set,
    .config_get = key_device_config_get,
    .listen = key_device_listen,
    .ignore = key_device_ignore,
    .io.map = {
        .get = key_device_get,
        .set = key_device_set,
        .size = key_device_size,
    }
};

const hw_device_t hw_device_key = {
    .name = KEY_DEVICE_NAME,
    .id   = KEY_DEVICE_ID,
    .type = HW_DEVICE_MAP,
    .inst_num   = KEY_INSTANCE_NUM,
    .conf_num   = KEY_CONFIG_NUM,
    .event_num  = KEY_EVENT_NUM,
    .conf_names = NULL,
    .conf_descs = NULL,
    .opt_names  = NULL,
};

int hw_setup_gpio(void)
{
    int i;
    for (i = 0; i < PORT_MAX; i++) {
        hw_gpio_pin_use[i] = 0;
    }

    hw_led_setup();
    pin_device_used = 0;
    pin_device_work = 0;
    key_device_used = 0;
    key_device_work = 0;

    return 0;
}

void hw_poll_gpio(void)
{
    pin_device_monitor();
    key_device_monitor();
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


