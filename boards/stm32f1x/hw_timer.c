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

#define INSTANCE_NUM    4       // instance of all timer devices

#define CONFIG_CHN      0       // channel selected
#define CONFIG_POLARITY 1       // output polarity
#define CONFIG_PERIOD   2       // output polarity
#define CONFIG_NUM      3       // config number of all timer devies

#define MODE_OUT        0
#define MODE_COUNT      1
#define MODE_TIMER      2

#define FLAG_ACTIVE     4
#define FLAG_EDGE_UP    1
#define FLAG_EDGE_DOWN  2

#define VAL_POSI_20US   0
#define VAL_POSI_S      1
#define VAL_NEGA_20US   2
#define VAL_NEGA_S      3
#define CHANNEL_NUM     4

static uint8_t device_used = 0;
static uint8_t device_work = 0;

static const uint32_t device_base[] = {TIM2, TIM3, TIM4, TIM5};
static const uint32_t device_irq[] = {NVIC_TIM2_IRQ, NVIC_TIM3_IRQ, NVIC_TIM4_IRQ, NVIC_TIM5_IRQ};

static int device_error[INSTANCE_NUM];
static int device_data[INSTANCE_NUM][CHANNEL_NUM];
static int device_settings[INSTANCE_NUM][CONFIG_NUM];
static uint8_t device_flag[INSTANCE_NUM];
static uint8_t device_type[INSTANCE_NUM];
static uint8_t device_events[INSTANCE_NUM];

static const char *device_config_names[] = {
    "channel", "polarity", "period"
};
static const char *device_opt_names[] = {
    "positive", "negative", "edge", "chn1", "chn2", "chn3", "chn4"
};
static const hw_config_desc_t pulse_pwm_count_config_descs[] = {
    {
        .type = HW_CONFIG_NUM,
    },
    {
        .type = HW_CONFIG_OPT,
        .opt_num = 2,
        .opt_start = 0,
    },
    {
        .type = HW_CONFIG_NUM,
    }
};
static const hw_config_desc_t timer_config_descs[] = {
    {
        .type = HW_CONFIG_OPT,
        .opt_num = 2,
        .opt_start = 3,
    },
    {
        .type = HW_CONFIG_OPT,
        .opt_num = 3,
        .opt_start = 0,
    }
};

static inline int device_is_inused(int inst) {
    if (inst < INSTANCE_NUM) {
        return device_used & (1 << inst);
    } else {
        return 0;
    }
}

static inline int device_is_work(int inst) {
    return (device_used & device_work) & (1 << inst);
}

static inline void device_set_work(int inst) {
    device_work |= (1 << inst);
}

static inline void device_clr_work(int inst) {
    device_work |= (1 << inst);
}

static inline void device_clr_error(int inst) {
    device_error[inst] = 0;
}

static inline void device_clr_event(int inst) {
    device_events[inst] = 0;
}

static void device_set_error(int inst, int error)
{
    device_error[inst] = error;

    if (device_events[inst] & 1) {
        devices_event_post(device_type[inst], inst, DEVICE_EVENT_ERR);
    }
}

static int device_get_error(int id, int inst)
{
    (void) id;

    if (device_is_inused(inst)) {
        return device_error[inst];
    }
    return 0;
}

static inline uint32_t device_conf_channel(int inst) {
    uint32_t channel = device_settings[inst][CONFIG_CHN] & 0xf;

    return channel ? channel : 0xf; // 0 as all channels
}

static inline uint32_t device_timer_conf_channel(int inst) {
    uint32_t channel = device_settings[inst][CONFIG_CHN];

    // a: use channel 1
    // b: use channel 2
    return channel ? 2 : 1;
}

static inline int device_conf_polarity(int inst) {
    return device_settings[inst][CONFIG_POLARITY];
}

static inline uint16_t device_conf_period(int inst) {
    uint16_t period = device_settings[inst][CONFIG_PERIOD];

    // 1 - 1000 ms
    if (period < 1) {
        period = 1;
    } else
    if (period > 1000) {
        period = 1000;
    }

    return period * 50 - 1;
}

static void device_isr(int inst, uint32_t base)
{
    uint32_t status = TIM_SR(base);

    if (device_type[inst] == COUNT_DEVICE_ID) {
        if (status & TIM_SR_CC1IF) {
            device_data[inst][0]++;
            device_flag[inst]++;
        }
        if (status & TIM_SR_CC2IF) {
            device_data[inst][1]++;
            device_flag[inst]++;
        }
        if (status & TIM_SR_CC3IF) {
            device_data[inst][2]++;
            device_flag[inst]++;
        }
        if (status & TIM_SR_CC4IF) {
            device_data[inst][3]++;
            device_flag[inst]++;
        }
    }

    if (device_type[inst] == TIMER_DEVICE_ID) {
        TIM_CNT(base) = 0;
        if (status & TIM_SR_CC1IF) {
            if (device_flag[inst]) {
                device_data[inst][VAL_NEGA_20US] = TIM_CCR1(base);
                device_flag[inst] = FLAG_ACTIVE | FLAG_EDGE_UP;
            } else {
                device_flag[inst] = FLAG_EDGE_UP;
            }
            device_data[inst][VAL_POSI_S] = 0;
        }
        if (status & TIM_SR_CC2IF) {
            if (device_flag[inst]) {
                device_data[inst][VAL_POSI_20US] = TIM_CCR2(base);
                device_flag[inst] = FLAG_ACTIVE | FLAG_EDGE_DOWN;
            } else {
                device_flag[inst] = FLAG_EDGE_DOWN;
            }
            device_data[inst][VAL_NEGA_S] = 0;
        }

        if (status & TIM_SR_UIF) {
            if (device_flag[inst] & FLAG_EDGE_UP) {
                device_data[inst][VAL_POSI_S]++;
            }
            if (device_flag[inst] & FLAG_EDGE_DOWN) {
                device_data[inst][VAL_NEGA_S]++;
            }
        }
    }

    TIM_SR(base) = 0;
}

static void device_setup_mode_out(int inst, uint32_t base, uint32_t channel)
{
    uint32_t enable = 0;

    if (channel & 1) {
        TIM_CCMR1(base) &= 0xff00;
        TIM_CCMR1(base) |= TIM_CCMR1_OC1M_PWM1 | TIM_CCMR1_OC1PE;
        TIM_CCR1(base) = 0;
        enable |= TIM_CCER_CC1P | TIM_CCER_CC1E;
    }
    if (channel & 2) {
        TIM_CCMR1(base) &= 0xff;
        TIM_CCMR1(base) |= TIM_CCMR1_OC2M_PWM1 | TIM_CCMR1_OC2PE;
        TIM_CCR2(base) = 0;
        enable |= TIM_CCER_CC2P | TIM_CCER_CC2E;
    }
    if (channel & 4) {
        TIM_CCMR2(base) &= 0xff00;
        TIM_CCMR2(base) |= TIM_CCMR2_OC3M_PWM1 | TIM_CCMR2_OC3PE;
        TIM_CCR3(base) = 0;
        enable |= TIM_CCER_CC3P | TIM_CCER_CC3E;
    }
    if (channel & 8) {
        TIM_CCMR2(base) &= 0xff;
        TIM_CCMR2(base) |= TIM_CCMR2_OC4M_PWM1 | TIM_CCMR2_OC4PE;
        TIM_CCR4(base) = 0;
        enable |= TIM_CCER_CC4P | TIM_CCER_CC4E;
    }

    if (device_conf_polarity(inst)) {
        enable &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P | TIM_CCER_CC3P | TIM_CCER_CC4P);
    }

    TIM_CCER(base) = enable;

    TIM_CR1(base) = TIM_CR1_CKD_CK_INT | TIM_CR1_CMS_EDGE;
}

static void device_setup_mode_count(int inst, uint32_t base, uint32_t channel)
{
    uint32_t enable = 0;

    TIM_CCMR1(base) = 0;
    TIM_CCMR2(base) = 0;

    if (channel & 1) {
        TIM_CCMR1(base) |= TIM_CCMR1_IC1F_CK_INT_N_8 | 1;
        enable |= TIM_CCER_CC1P | TIM_CCER_CC1E;
    }
    if (channel & 2) {
        TIM_CCMR1(base) |= TIM_CCMR1_IC2F_CK_INT_N_8 | (1 << 8);
        enable |= TIM_CCER_CC2P | TIM_CCER_CC2E;
    }
    if (channel & 4) {
        TIM_CCMR2(base) |= TIM_CCMR2_IC3F_CK_INT_N_8 | 1;
        enable |= TIM_CCER_CC3P | TIM_CCER_CC3E;
    }
    if (channel & 8) {
        TIM_CCMR2(base) |= TIM_CCMR2_IC4F_CK_INT_N_8 | (1 << 8);
        enable |= TIM_CCER_CC4P | TIM_CCER_CC4E;
    }

    if (device_conf_polarity(inst)) {
        enable &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P | TIM_CCER_CC3P | TIM_CCER_CC4P);
    }

    TIM_CCER(base) = enable;
    TIM_DIER(base) = (channel & 0x0f) << 1;
    TIM_SR(base) = 0;

    TIM_CR1(base) = TIM_CR1_CKD_CK_INT | TIM_CR1_CMS_EDGE;
}

static uint32_t device_channel_setup(int inst, int in_mode)
{
    uint32_t channel = device_conf_channel(inst);
    uint32_t base;
    uint8_t mod, cnf;

    if (in_mode) {
        mod = GPIO_MODE_INPUT;
        cnf = GPIO_CNF_INPUT_FLOAT;
    } else {
        mod = GPIO_MODE_OUTPUT_10_MHZ;
        cnf = GPIO_CNF_OUTPUT_ALTFN_PUSHPULL;
    }

    switch(inst) {
    case 0:
        if (!hw_gpio_use(0, channel)) {
            device_set_error(inst, 789);
            return 0;
        }
        gpio_set_mode(GPIOA, mod, cnf, channel);
        rcc_periph_clock_enable(RCC_TIM2);
        break;
    case 1: {
        uint16_t chn12 = (channel & 3) << 6;
        uint16_t chn34 = (channel >> 2) & 3;

        if (chn12) {
            if (hw_gpio_use(0, chn12)) {
                gpio_set_mode(GPIOA, mod, cnf, chn12);
            } else {
                device_set_error(inst, 789);
                return 0;
            }
        }
        if (chn34) {
            if (hw_gpio_use(1, chn34)) {
                gpio_set_mode(GPIOB, mod, cnf, chn34);
            } else {
                hw_gpio_release(1, chn12);
                device_set_error(inst, 789);
                return 0;
            }
        }
        rcc_periph_clock_enable(RCC_TIM3);
        break;
    }
    case 2:
        if (!hw_gpio_use(1, channel << 6)) {
            device_set_error(inst, 789);
            return 0;
        }
        gpio_set_mode(GPIOB, mod, cnf, channel << 6);
        rcc_periph_clock_enable(RCC_TIM4);
        break;
    case 3:
        if (!hw_gpio_use(0, channel)) {
            device_set_error(inst, 789);
            return 0;
        }
        gpio_set_mode(GPIOA, mod, cnf, channel);
        rcc_periph_clock_enable(RCC_TIM5);
        break;
    default: return 0;
    }

    base = device_base[inst];

    if (!in_mode) {
        device_setup_mode_out(inst, base, channel);
    } else
    if (in_mode == MODE_COUNT) {
        device_setup_mode_count(inst, base, channel);
    }

    return base;
}

static void device_channel_reset(int inst)
{
    uint32_t channel = device_conf_channel(inst);

    switch(inst) {
    case 0:
        hw_gpio_release(0, channel);
        rcc_periph_clock_disable(RCC_TIM2);
        break;
    case 1: {
        uint16_t chn12 = (channel & 3) << 6;
        uint16_t chn34 = (channel >> 2) & 3;

        if (chn12) {
            hw_gpio_release(0, chn12);
        }
        if (chn34) {
            hw_gpio_release(1, chn34);
        }
        rcc_periph_clock_disable(RCC_TIM3);
        break;
    }
    case 2:
        hw_gpio_release(1, channel << 6);
        rcc_periph_clock_disable(RCC_TIM4);
        break;
    case 3:
        hw_gpio_release(0, channel);
        rcc_periph_clock_disable(RCC_TIM5);
        break;
    default:
        break;
    }
}

static int device_channel_get(int inst, int off, uint32_t *v)
{
    uint32_t channel = device_conf_channel(inst);
    uint32_t base = device_base[inst];

    switch (off) {
    case 0: if (channel & 1) {*v = TIM_CCR1(base); return 1; } break;
    case 1: if (channel & 2) {*v = TIM_CCR2(base); return 1; } break;
    case 2: if (channel & 4) {*v = TIM_CCR3(base); return 1; } break;
    case 3: if (channel & 8) {*v = TIM_CCR4(base); return 1; } break;
    default: break;
    }
    return 0;
}

static int device_channel_set(int inst, int off, uint32_t v)
{
    uint32_t channel = device_conf_channel(inst);
    uint32_t base = device_base[inst];

    switch (off) {
    case 0: if (channel & 1) {TIM_CCR1(base) = v; return 1; } break;
    case 1: if (channel & 2) {TIM_CCR2(base) = v; return 1; } break;
    case 2: if (channel & 4) {TIM_CCR3(base) = v; return 1; } break;
    case 3: if (channel & 8) {TIM_CCR4(base) = v; return 1; } break;
    default: break;
    }
    return 0;
}

static int device_channel_set_all(int inst, uint32_t v)
{
    uint32_t channel = device_conf_channel(inst);
    uint32_t base = device_base[inst];

    if (channel & 1) TIM_CCR1(base) = v;
    if (channel & 2) TIM_CCR2(base) = v;
    if (channel & 4) TIM_CCR3(base) = v;
    if (channel & 8) TIM_CCR4(base) = v;

    return 1;
}

static int device_pulse_setup(int inst)
{
    return device_channel_setup(inst, MODE_OUT) ? 1 : 0;
}

static int device_pwm_setup(int inst)
{
    uint32_t base = device_channel_setup(inst, MODE_OUT);

    if (!base) {
        return 0;
    }

    TIM_PSC(base) = 1440;  // 20uS pre tick
    TIM_ARR(base) = device_conf_period(inst);
    TIM_CNT(base) = 0;
    TIM_EGR(base) = TIM_EGR_UG; // update to real register

    TIM_CR1(base) |= TIM_CR1_ARPE | TIM_CR1_CEN;

    return 1;
}

static int device_count_setup(int inst)
{
    uint32_t base = device_channel_setup(inst, MODE_COUNT);

    if (!base) {
        return 0;
    }

    device_flag[inst] = 0;
    device_data[inst][0] = 0;
    device_data[inst][1] = 0;
    device_data[inst][2] = 0;
    device_data[inst][3] = 0;

    TIM_CNT(base) = 0;
    TIM_PSC(base) = 720;     // 10uS pre tick
    TIM_ARR(base) = 65535;
    // update PSC,ARR to shadow register
    TIM_EGR(base) = TIM_EGR_UG;

    TIM_SR(base) = 0;
    nvic_enable_irq(device_irq[inst]);

    TIM_CR1(base) |= TIM_CR1_ARPE | TIM_CR1_CEN;

    return 1;
}

static int device_timer_setup(int inst)
{
    uint32_t base;
    uint32_t channel;

    // revise channel setting
    if (device_settings[inst][CONFIG_CHN]) {
        channel = device_settings[inst][CONFIG_CHN] = 2;
    } else {
        channel = device_settings[inst][CONFIG_CHN] = 1;
    }

    base = device_channel_setup(inst, MODE_TIMER);
    if (!base) {
        return 0;
    }

    device_data[inst][0] = 0;
    device_data[inst][1] = 0;

    TIM_CR1(base) = TIM_CR1_CKD_CK_INT | TIM_CR1_CMS_EDGE | TIM_CR1_DIR_UP;
    TIM_PSC(base) = 1440;     // 20uS pre tick
    TIM_ARR(base) = 50000;
    TIM_EGR(base) = TIM_EGR_UG; // update to real register

    if (channel & 1) {
        TIM_CCMR1(base) = TIM_CCMR1_CC1S_IN_TI1 | TIM_CCMR1_CC2S_IN_TI1 |
                          TIM_CCMR1_IC1F_CK_INT_N_8;
    } else {
        TIM_CCMR1(base) = TIM_CCMR1_CC1S_IN_TI2 | TIM_CCMR1_CC2S_IN_TI2 |
                          TIM_CCMR1_IC2F_CK_INT_N_8;
    }

    TIM_DIER(base) = 3 << 1;
    TIM_SR(base) = 0;

    nvic_enable_irq(device_irq[inst]);

    TIM_CCER(base) = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC2P;
    TIM_CR1(base) |= TIM_CR1_CEN;

    return 1;
}

static int device_enable(int id, int inst)
{
    int status;

    switch(id) {
    case PULSE_DEVICE_ID: status = device_pulse_setup(inst); break;
    case PWM_DEVICE_ID:   status = device_pwm_setup(inst);   break;
    case COUNT_DEVICE_ID: status = device_count_setup(inst); break;
    case TIMER_DEVICE_ID: status = device_timer_setup(inst); break;
    default: status = 0;
    }

    if (status) {
        device_set_work(inst);
    }

    return status;
}

static int device_disable(int id, int inst)
{
    (void) id;

    if (device_is_work(inst)) {
        uint32_t base = device_base[inst];

        nvic_disable_irq(device_irq[inst]);
        device_channel_reset(inst);

        TIM_DIER(base) = 0;
        TIM_CR2(base) = 0;
        TIM_CR1(base) = 0;

        device_clr_work(inst);
    }
    return 1;
}

// 0: fail
// 1: ok
static int device_request(int id, int inst)
{
    (void) id;
    if (inst < INSTANCE_NUM) {
        int used = device_used & (1 << inst);

        if (!used) {
            int c;

            device_clr_work(inst);
            device_clr_error(inst);
            device_clr_event(inst);

            device_used |= 1 << inst;
            for (c = 0; c < CONFIG_NUM; c++) {
                device_settings[inst][c] = 0;
            }

            device_type[inst] = id;

            return 1;
        }
    }

    return 0;
}

// 0: fail
// other: ok
static int device_release(int id, int inst)
{
    (void) id;
    if (device_is_inused(inst)) {
        device_disable(id, inst);
        device_used &= ~(1 << inst);
        return 1;
    } else {
        return 0;
    }
}

static int device_config_set(int id, int inst, int which, int setting)
{
    (void) id;
    if (device_is_inused(inst) && which < CONFIG_NUM) {
        device_settings[inst][which] = setting;
        return 1;
    }
    return 0;
}

static int device_config_get(int id, int inst, int which, int *setting)
{
    (void) id;
    if (device_is_inused(inst) && which < CONFIG_NUM && setting) {
        *setting = device_settings[inst][which];
        return 1;
    }
    return 0;
}

static void device_listen(int id, int inst, int event)
{
    (void) id;
    if (device_is_inused(inst) && event < DEVICE_EVENT_MAX) {
        device_events[inst] |= 1 << event;
    }
}

static void device_ignore(int id, int inst, int event)
{
    (void) id;
    if (device_is_inused(inst) && event < DEVICE_EVENT_MAX) {
        device_events[inst] &= ~(1 << event);
    }
}

static int device_size(int id, int inst)
{
    (void) id;
    (void) inst;

    return 4;
}

static int device_pulse_get(int id, int inst, int off, uint32_t *v)
{
    uint32_t base = device_base[inst];

    (void) id;
    (void) off;

    if (TIM_CR1(base) & TIM_CR1_CEN) {
        *v = 1;
    } else {
        *v = 0;
    }
    return 0;
}

static int device_pulse_set(int id, int inst, int off, uint32_t v)
{
    uint32_t base = device_base[inst];
    int n;

    (void) id;

    if (TIM_CR1(base) & TIM_CR1_CEN) {
        // Pulse busy
        return 0;
    }

    v = v / 10; // 10uS
    if (v < 1) {
        v = 1;
    } else
    if (v > 50000) {
        v = 50000; // 500ms is limit
    }

    if (off < 0) {
        n = device_channel_set_all(inst, 1);
    } else {
        n = device_channel_set(inst, off, 1);
    }

    if (n) {
        TIM_PSC(base) = 720;        // 10uS pre tick
        TIM_ARR(base) = v + 1;
        TIM_CNT(base) = 0;
        TIM_EGR(base) = TIM_EGR_UG;
        TIM_CR1(base) |= TIM_CR1_ARPE | TIM_CR1_OPM | TIM_CR1_CEN;
    }
    return n;
}

static int device_pwm_get(int id, int inst, int off, uint32_t *v)
{
    uint32_t duty, max = device_conf_period(inst);

    (void) id;

    if (!device_channel_get(inst, off, &duty)) {
        return 0;
    }
    *v = 1000 - duty * 1000 / max;

    return 1;
}

static int device_pwm_set(int id, int inst, int off, uint32_t v)
{
    int max = device_conf_period(inst);

    (void) id;

    if (v > 1000) {
        v = 1000;
    }
    v = max * (1000 - v) / 1000;

    if (off < 0) {
        return device_channel_set_all(inst, v);
    } else {
        return device_channel_set(inst, off, v);
    }
}

static int device_count_get(int id, int inst, int off, uint32_t *v)
{
    uint32_t channel = device_conf_channel(inst);

    (void) id;

    if (off < 0) {
        *v = device_data[inst][0] + device_data[inst][1] +
             device_data[inst][2] + device_data[inst][3];
        return 1;
    }

    if (channel & (1 << off)) {
        *v = device_data[inst][off];
        return 1;
    } else {
        return 0;
    }
}

static int device_count_set(int id, int inst, int off, uint32_t v)
{
    uint32_t channel = device_conf_channel(inst);

    (void) id;

    if (off < 0) {
        device_data[inst][0] = v;
        device_data[inst][1] = v;
        device_data[inst][2] = v;
        device_data[inst][3] = v;
        return 1;
    }

    if (channel & (1 << off)) {
        device_data[inst][off] = v;
        return 1;
    }

    return 0;
}

static int device_timer_get(int id, int inst, int off, uint32_t *v)
{
    int polarity = device_conf_polarity(inst);

    (void) id;

    if (off < 0 || off > 1) {
        return 0;
    }

    if (polarity == 0) {
        *v = device_data[inst][VAL_POSI_20US + off];
    } else
    if (polarity == 1) {
        *v = device_data[inst][VAL_NEGA_20US + off];
    } else {
        if (device_flag[inst] & FLAG_EDGE_UP) {
            *v = device_data[inst][VAL_NEGA_20US + off];
        } else {
            *v = device_data[inst][VAL_POSI_20US + off];
        }
    }

    if (off == 0) {
        *v *= 20;
    }

    return 1;
}

static int device_timer_set(int id, int inst, int off, uint32_t v)
{
    (void) id;
    (void) inst;
    (void) off;
    (void) v;

    return 0;
}

static int device_timer_size(int id, int inst)
{
    (void) id;
    (void) inst;

    if (device_is_work(inst)) {
        return 2;
    }

    return 0;
}

void hw_setup_timer(void)
{
    device_used = 0;
    device_work = 0;
}

void hw_poll_timer(void)
{
    int i;

    for (i = 0; i < INSTANCE_NUM; i++) {
        if (device_is_work(i) && (device_events[i] & (1 << DEVICE_EVENT_DATA))) {
            if (device_type[i] == COUNT_DEVICE_ID) {
                if (device_flag[i]) {
                    device_flag[i] = 0;
                    devices_event_post(COUNT_DEVICE_ID, i, DEVICE_EVENT_DATA);
                }
            }
            if (device_type[i] == TIMER_DEVICE_ID) {
                uint8_t flag = device_flag[i];
                if (flag & FLAG_ACTIVE) {
                    int polarity = device_conf_polarity(i);
                    if (polarity == 0) {
                        if (flag & FLAG_EDGE_DOWN) {
                            devices_event_post(TIMER_DEVICE_ID, i, DEVICE_EVENT_DATA);
                        }
                    } else
                    if (polarity == 1) {
                        char buf[64];
                        snprintf(buf, 64, "flag: %x\r\n", flag);
                        hw_console_sync_puts(buf);
                        if (flag & FLAG_EDGE_UP) {
                            devices_event_post(TIMER_DEVICE_ID, i, DEVICE_EVENT_DATA);
                        }
                    } else {
                        devices_event_post(TIMER_DEVICE_ID, i, DEVICE_EVENT_DATA);
                    }
                    device_flag[i] &= ~FLAG_ACTIVE;
                }
            }
        }
    }
}

const hw_driver_t hw_driver_pulse = {
    .request = device_request,
    .release = device_release,
    .get_err = device_get_error,
    .enable  = device_enable,
    .disable = device_disable,
    .config_set = device_config_set,
    .config_get = device_config_get,
    .listen = device_listen,
    .ignore = device_ignore,
    .io.map = {
        .get = device_pulse_get,
        .set = device_pulse_set,
        .size = device_size,
    }
};

const hw_device_t hw_device_pulse = {
    .name = PULSE_DEVICE_NAME,
    .id   = PULSE_DEVICE_ID,
    .type = HW_DEVICE_MAP,
    .inst_num   = PULSE_INSTANCE_NUM,
    .conf_num   = PULSE_CONFIG_NUM,
    .event_num  = PULSE_EVENT_NUM,
    .conf_names = device_config_names,
    .conf_descs = pulse_pwm_count_config_descs,
    .opt_names  = device_opt_names,
};

const hw_driver_t hw_driver_pwm = {
    .request = device_request,
    .release = device_release,
    .get_err = device_get_error,
    .enable  = device_enable,
    .disable = device_disable,
    .config_set = device_config_set,
    .config_get = device_config_get,
    .listen = device_listen,
    .ignore = device_ignore,
    .io.map = {
        .get = device_pwm_get,
        .set = device_pwm_set,
        .size = device_size,
    }
};

const hw_device_t hw_device_pwm = {
    .name = PWM_DEVICE_NAME,
    .id   = PWM_DEVICE_ID,
    .type = HW_DEVICE_MAP,
    .inst_num   = PWM_INSTANCE_NUM,
    .conf_num   = PWM_CONFIG_NUM,
    .event_num  = PWM_EVENT_NUM,
    .conf_names = device_config_names,
    .conf_descs = pulse_pwm_count_config_descs,
    .opt_names  = device_opt_names,
};

const hw_driver_t hw_driver_count = {
    .request = device_request,
    .release = device_release,
    .get_err = device_get_error,
    .enable  = device_enable,
    .disable = device_disable,
    .config_set = device_config_set,
    .config_get = device_config_get,
    .listen = device_listen,
    .ignore = device_ignore,
    .io.map = {
        .get = device_count_get,
        .set = device_count_set,
        .size = device_size,
    }
};

const hw_device_t hw_device_count= {
    .name = COUNT_DEVICE_NAME,
    .id   = COUNT_DEVICE_ID,
    .type = HW_DEVICE_MAP,
    .inst_num   = COUNT_INSTANCE_NUM,
    .conf_num   = COUNT_CONFIG_NUM,
    .event_num  = COUNT_EVENT_NUM,
    .conf_names = device_config_names,
    .conf_descs = pulse_pwm_count_config_descs,
    .opt_names  = device_opt_names,
};

const hw_driver_t hw_driver_timer = {
    .request = device_request,
    .release = device_release,
    .get_err = device_get_error,
    .enable  = device_enable,
    .disable = device_disable,
    .config_set = device_config_set,
    .config_get = device_config_get,
    .listen = device_listen,
    .ignore = device_ignore,
    .io.map = {
        .get = device_timer_get,
        .set = device_timer_set,
        .size = device_timer_size,
    }
};

const hw_device_t hw_device_timer = {
    .name = TIMER_DEVICE_NAME,
    .id   = TIMER_DEVICE_ID,
    .type = HW_DEVICE_MAP,
    .inst_num   = TIMER_INSTANCE_NUM,
    .conf_num   = TIMER_CONFIG_NUM,
    .event_num  = TIMER_EVENT_NUM,
    .conf_names = device_config_names,
    .conf_descs = timer_config_descs,
    .opt_names  = device_opt_names,
};

void tim2_isr(void)
{
    device_isr(0, TIM2);
}

void tim3_isr(void)
{
    device_isr(1, TIM3);
}

void tim4_isr(void)
{
    device_isr(2, TIM4);
}

void tim5_isr(void)
{
    device_isr(3, TIM5);
}

