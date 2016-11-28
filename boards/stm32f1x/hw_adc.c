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

#define ADC_IDLE        0
#define ADC_READY       1
#define ADC_BUSY        2      // work in process
#define ADC_EVENT_MASK  0x0f

#define DEVICE_CHANNEL_NUM 2

#define ADC_CHANNEL     0
#define ADC_INTERVAL    1
#define ADC_INVALID_VAL 0xffff

typedef struct hw_adc_t{
    uint8_t flags;
    uint8_t event;
    uint8_t chn_cur;
    uint8_t chn_num;
    uint8_t  chn_seq[DEVICE_CHANNEL_NUM];
    uint16_t chn_val[DEVICE_CHANNEL_NUM];
    int      error;
    int      conf[ADC_CONFIG_NUM];
    uint32_t sleep;
} hw_adc_t;


static const char *device_config_names[] = {
    "channel", "interval"
};
static const char *device_opt_names[] = {
    "chn0", "chn1", "all"
};
static const hw_config_desc_t device_config_descs[] = {
    {
        .type = HW_CONFIG_OPT,
        .opt_num = 3,
        .opt_start = 0,
    },
    {
        .type = HW_CONFIG_NUM,
    }
};

static uint8_t device_used = 0;
static uint8_t device_work = 0;
static hw_adc_t device_control[ADC_INSTANCE_NUM];

static inline int hw_adc_ready_test(void)
{
    return 1;
}

static inline int hw_adc_convert_test(void)
{
    return adc_eoc(ADC1);
}

static inline int hw_adc_ready_check(hw_adc_t *control)
{
    if (hw_adc_ready_test()) {
        adc_set_regular_sequence(ADC1, control->chn_num, control->chn_seq);
        return 1;
    } else {
        _TRACE("... wait adc ready\n");
        return 0;
    }
}

static int hw_adc_start(hw_adc_t *control)
{
    if (control->sleep > 0) {
        _TRACE("... adc wait: %u\n", control->sleep);
        control->sleep--;
        return 0;
    }

    _TRACE("... adc convert start\n");
    adc_start_conversion_direct(ADC1);

    return 1;
}

static void hw_adc_load(hw_adc_t *control)
{
    control->chn_val[control->chn_cur++] = adc_read_regular(ADC1);

    if (control->chn_cur >= control->chn_num) {
        control->chn_cur = 0;
        if (control->event & (1 << DEVICE_EVENT_DATA)) {
            _TRACE("post adc event: data\n");
            devices_event_post(ADC_DEVICE_ID, control - device_control, DEVICE_EVENT_DATA);
        }
    }

    control->sleep = control->conf[ADC_INTERVAL];
}

static inline int hw_adc_wait_convert(hw_adc_t *control)
{
    if (hw_adc_convert_test()) {
        hw_adc_load(control);
        _TRACE("... adc convert complete\n");
        return 1;
    } else {
        _TRACE("... adc convert wait\n");
        return 0;
    }
}

static inline int device_is_inused(int inst) {
    if (inst < ADC_INSTANCE_NUM) {
        return device_used & (1 << inst);
    } else {
        return 0;
    }
}

static inline int device_is_work(int inst) {
    if (inst < ADC_INSTANCE_NUM) {
        return (device_used & device_work) & (1 << inst);
    } else {
        return 0;
    }
}

static void device_set_error(int id, int inst, int error)
{
    device_control[inst].error = error;

    (void) id;

    if (device_control[inst].event & (1 << DEVICE_EVENT_ERR)) {
        devices_event_post(ADC_DEVICE_ID, inst, DEVICE_EVENT_ERR);
    }
}

static int device_get_error(int id, int inst)
{
    if (inst >= ADC_INSTANCE_NUM) {
        return 0;
    }

    (void) id;

    return device_control[inst].error;
}

static int device_setup(int inst)
{
    hw_adc_t *control = &device_control[inst];
    int i;

    _TRACE("... adc on\n");

    control->flags = ADC_IDLE;
    control->sleep = control->conf[ADC_INTERVAL];
    control->chn_cur = 0;
    for (i = 0; i < DEVICE_CHANNEL_NUM; i++) {
        control->chn_val[i] = ADC_INVALID_VAL;
    }

    rcc_periph_clock_enable(RCC_ADC1);
    if (control->conf[ADC_CHANNEL] < 2) {
        control->chn_num = 1;
        if (control->conf[ADC_CHANNEL] == 0) {
            hw_gpio_use(1, GPIO0);
            control->chn_seq[0] = 8;
            gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0);
        } else {
            hw_gpio_use(1, GPIO1);
            control->chn_seq[0] = 9;
            gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO1);
        }
    } else {
        control->chn_num = 2;
        hw_gpio_use(1, GPIO1 | GPIO0);
        control->chn_seq[0] = 8;
        control->chn_seq[1] = 9;
        gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0);
        gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO1);
    }

    adc_power_off(ADC1);

    adc_disable_scan_mode(ADC1);
    adc_disable_external_trigger_regular(ADC1);

    adc_set_right_aligned(ADC1);
    adc_set_single_conversion_mode(ADC1);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_28DOT5CYC);

    adc_power_on(ADC1);

    adc_reset_calibration(ADC1);

    adc_calibrate(ADC1);

    return 1;
}

static int device_reset(int inst)
{
    hw_adc_t *control = &device_control[inst];

    adc_power_off(ADC1);
    if (control->conf[ADC_CHANNEL] < 2) {
        if (control->conf[ADC_CHANNEL] == 0) {
            hw_gpio_release(1, GPIO0);
        } else {
            hw_gpio_release(1, GPIO1);
        }
    } else {
        hw_gpio_release(1, GPIO1 | GPIO0);
    }
    rcc_periph_clock_disable(RCC_ADC1);

    _TRACE("... adc off\n");

    return 1;
}

static int device_enable(int id, int inst)
{
    (void) id;
    if (device_is_inused(inst)) {
        uint8_t b = 1 << inst;

        if (!(device_work & b)) {
            device_work |= b;
            return device_setup(inst);
        }
        return 1;
    }
    return 0;
}

static int device_disable(int id, int inst)
{
    (void) id;
    if (device_is_inused(inst)) {
        uint8_t b = 1 << inst;

        if (device_work & b) {
            device_work &= ~b;
            return device_reset(inst);
        } else {
            return 1;
        }
    }
    return 0;
}

// 0: fail
// 1: ok
static int device_request(int id, int inst)
{
    (void) id;

    if (inst < ADC_INSTANCE_NUM) {
        int used = device_used & (1 << inst);

        if (!used) {
            hw_adc_t *control = &device_control[inst];
            int c;

            control->error = 0;
            control->event = 0;
            control->chn_num = 0;

            device_used |= 1 << inst;
            device_work &= ~(1 << inst);
            for (c = 0; c < ADC_CONFIG_NUM; c++) {
                control->conf[c] = 0;
            }

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
    if (device_is_inused(inst) && which < ADC_CONFIG_NUM) {
        device_control[inst].conf[which] = setting;
        return 1;
    }
    return 0;
}

static int device_config_get(int id, int inst, int which, int *setting)
{
    (void) id;
    if (device_is_inused(inst) && which < ADC_CONFIG_NUM && setting) {
        *setting = device_control[inst].conf[which];
        return 1;
    }
    return 0;
}

static void device_listen(int id, int inst, int event)
{
    (void) id;
    if (device_is_inused(inst) && event < ADC_EVENT_NUM) {
        device_control[inst].event |= 1 << event;
    }
}

static void device_ignore(int id, int inst, int event)
{
    (void) id;
    if (device_is_inused(inst) && event < ADC_EVENT_NUM) {
        device_control[inst].event &= ~(1 << event);
    }
}

static int device_get(int id, int inst, int off, uint32_t *v)
{
    (void) id;

    if (device_is_work(inst) && off >= 0 && off < device_control[inst].chn_num) {
        *v = device_control[inst].chn_val[off];
        return 1;
    }

    return 0;
}

static int device_set(int id, int inst, int off, uint32_t val)
{
    (void) id;
    (void) inst;
    (void) off;
    (void) val;

    return 0;
}

static int device_size(int id, int inst)
{
    (void) id;
    if (device_is_inused(inst)) {
        return device_control[inst].chn_num;
    }
    return 0;
}

const hw_driver_t hw_driver_adc = {
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
        .set = device_set,
        .get  = device_get,
        .size = device_size,
    }
};

const hw_device_t hw_device_adc = {
    .name = ADC_DEVICE_NAME,
    .id   = ADC_DEVICE_ID,
    .type = HW_DEVICE_MAP,
    .inst_num   = ADC_INSTANCE_NUM,
    .conf_num   = ADC_CONFIG_NUM,
    .event_num  = ADC_EVENT_NUM,
    .conf_names = device_config_names,
    .conf_descs = device_config_descs,
    .opt_names  = device_opt_names,
};

int hw_setup_adc(void)
{
    device_used = 0;
    device_work = 0;
    return 0;
}

void hw_poll_adc(void)
{
    int i;

    for (i = 0; i < ADC_INSTANCE_NUM; i++) {
        if (device_is_work(i)) {
            hw_adc_t *control = &device_control[i];

            switch(control->flags) {
            case ADC_IDLE:
                if (hw_adc_ready_check(control)) {
                    control->flags = ADC_READY;
                }
                break;
            case ADC_READY:
                if (hw_adc_start(control)) {
                    control->flags = ADC_BUSY;
                }
                break;
            case ADC_BUSY:
                if (hw_adc_wait_convert(control)) {
                    control->flags = ADC_READY;
                }
                break;
            default:
                control->flags = ADC_IDLE;
                // Todo: error process here
                device_set_error(ADC_DEVICE_ID, i, 1);
                break;
            }
        }
    }
}

