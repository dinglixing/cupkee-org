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

#include <bsp.h>
#include "hardware.h"

#define ADC_INUSED      0x80
#define ADC_ENABLE      0x40

#define ADC_IDEL        0
#define ADC_READY       0x20
#define ADC_BUSY        0x10      // work in process
#define ADC_STATE_MASK  0x30

#define ADC_EVENT_MASK  0x0f

#define ADC_INVALID_VAL 0xffff

typedef struct hw_adc_t{
    uint8_t flags;
    uint8_t chn_cur;
    uint8_t chn_num;
    uint8_t chn_seq[5];
    uint32_t interval;
    uint32_t sleep;
} hw_adc_t;

static hw_adc_t       adc_blks[ADC_MAX];
static uint16_t       adc_vals[ADC_CHANNEL_MAX];

static inline int hw_adc_check(int adc) {
    return (adc < ADC_MAX && (adc_blks[adc].flags & ADC_INUSED));
}

static inline int hw_adc_ready_test(void)
{
    return 1;
}

static inline int hw_adc_convert_test(void)
{
    return adc_eoc(ADC1);
}

static inline int hw_adc_ready_check(hw_adc_t *blk)
{
    (void) blk;

    if (hw_adc_ready_test()) {

        adc_set_regular_sequence(ADC1, blk->chn_num, blk->chn_seq);

        if (blk->flags & (1 << ADC_EVENT_READY)) {
            devices_event_post(ADC_DEVICE_ID, blk - adc_blks, ADC_EVENT_READY);
        }
        return 1;
    } else {
        _TRACE("... wait adc ready\n");
        return 0;
    }
}

static void hw_adc_on(hw_adc_t *blk)
{
    int i;

    _TRACE("... adc on\n");

    blk->chn_cur = ADC_CHANNEL_MAX;

    rcc_periph_clock_enable(RCC_ADC1);
    // hardware setting code here
    for (i = 0; i < blk->chn_num; i++) {
        int ch = blk->chn_seq[i];

        if (ch == 8) {
            rcc_periph_clock_enable(RCC_GPIOB);
            gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0);
        }
        if (ch == 9) {
            rcc_periph_clock_enable(RCC_GPIOB);
            gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO1);
        }
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

}

static void hw_adc_off(hw_adc_t *blk)
{
    (void) blk;

    /* hardware code here */
    adc_power_on(ADC1);
    rcc_periph_clock_disable(RCC_ADC1);

    _TRACE("... adc off\n");
    blk->flags &= ~ADC_STATE_MASK;
}

static int hw_adc_start(hw_adc_t *blk)
{
    if (blk->sleep != 0) {
        _TRACE("... adc wait: %u\n", blk->sleep);
        blk->sleep--;
        return 0;
    }

    _TRACE("... adc convert start\n");
    adc_start_conversion_direct(ADC1);

    return 1;
}

static void hw_adc_load(hw_adc_t *blk)
{
    adc_vals[blk->chn_seq[blk->chn_cur++]] = adc_read_regular(ADC1);

    if (blk->chn_cur >= blk->chn_num) {
        if (blk->flags & (1 << ADC_EVENT_DATA)) {
            _TRACE("post adc event: data\n");
            devices_event_post(ADC_DEVICE_ID, blk - adc_blks, ADC_EVENT_DATA);
        }
        blk->chn_cur = 0;
    }

    blk->sleep = blk->interval;
}

static inline int hw_adc_wait_convert(hw_adc_t *blk)
{
    if (hw_adc_convert_test()) {
        _TRACE("... adc convert complete\n");
        hw_adc_load(blk);
        return 1;
    } else {
        _TRACE("... adc convert wait\n");
        return 0;
    }
}

static int hw_adc_config_set(hw_adc_t *blk, hw_adc_conf_t *cfg)
{
    int i;

    blk->chn_cur = 0;
    blk->chn_num = cfg->chn_num;
    for (i = 0; i < cfg->chn_num; i++) {
        blk->chn_seq[i] = cfg->chn_seq[i];
    }

    blk->interval = cfg->interval;
    blk->sleep = blk->interval;

    hw_adc_on(blk);

    return 0;
}

static int hw_adc_config_clr(hw_adc_t *blk)
{
    hw_adc_off(blk);

    blk->chn_num = 0;
    blk->interval = 0;

    return 0;
}

int hw_adc_setup(void)
{
    int i;

    // hardware setup code here
    for (i = 0; i < ADC_MAX; i++) {
        adc_blks[i].flags = 0;
        adc_blks[i].chn_cur = 0;
        adc_blks[i].chn_num = 0;
    }

    for (i = 0; i < ADC_CHANNEL_MAX; i++) {
        adc_vals[i] = ADC_INVALID_VAL;
    }

    return 0;
}

void hw_adc_poll(void)
{
    int i;

    for (i = 0; i < ADC_MAX; i++) {
        hw_adc_t *blk = adc_blks + i;

        if (blk->flags & ADC_INUSED && blk->flags & ADC_ENABLE) {
            switch(blk->flags & ADC_STATE_MASK) {
            case ADC_IDEL:
                if (hw_adc_ready_check(blk)) {
                    blk->flags |= ADC_READY;
                }
                break;
            case ADC_READY:
                if (hw_adc_start(blk)) {
                    blk->flags |= ADC_BUSY;
                }
                break;
            case (ADC_READY | ADC_BUSY):
                if (hw_adc_wait_convert(blk)) {
                    blk->flags &= ~ADC_BUSY;
                }
                break;
            default:
                // Todo: error process here
                break;
            }
        }
    }
}

int hw_adc_alloc(void)
{
    int i;

    for (i = 0; i < ADC_MAX; i++) {
        if (!(adc_blks[i].flags & ADC_INUSED)) {
            adc_blks[i].flags = ADC_INUSED;
            return i;
        }
    }

    return -1;
}

int hw_adc_release(int adc)
{
    if (hw_adc_check(adc)) {
        adc_blks[adc].flags = 0;
        return 0;
    }
    return -1;
}

void hw_adc_conf_reset(hw_adc_conf_t *conf)
{
    conf->interval = 0;
    conf->chn_num = 0;
}

int hw_adc_enable(int adc, hw_adc_conf_t *cfg)
{
    hw_adc_t *blk;
    if (!hw_adc_check(adc)) {
        return -1;
    }

    blk = &adc_blks[adc];
    if (!(blk->flags & ADC_ENABLE)) {
        if (0 == hw_adc_config_set(blk, cfg)) {
            blk->flags |= ADC_ENABLE;
            return 0;
        }
    }

    return -1;
}

int hw_adc_disable(int adc)
{
    hw_adc_t *blk;
    if (!hw_adc_check(adc)) {
        return -1;
    }

    blk = adc_blks + adc;
    if (blk->flags & ADC_ENABLE) {
        if (0 == hw_adc_config_clr(blk)) {
            blk->flags &= ~ADC_ENABLE;
            return 0;
        }
    }
    return -1;
}

int hw_adc_read(int adc, int ch, uint32_t *data)
{
    hw_adc_t *blk;
    if (!hw_adc_check(adc)) {
        return -1;
    }

    blk = adc_blks + adc;
    if (ch < blk->chn_num && adc_vals[blk->chn_seq[ch]] != ADC_INVALID_VAL) {
        *data = adc_vals[blk->chn_seq[ch]];
        return 1;
    }

    return 0;
}

int hw_adc_event_enable(int adc, int event)
{
    if (!hw_adc_check(adc) || event >= ADC_EVENT_MAX) {
        return -1;
    }

    adc_blks[adc].flags |= (1 << event);

    return 0;
}

int hw_adc_event_disable(int adc, int event)
{
    if (!hw_adc_check(adc) || event >= ADC_EVENT_MAX) {
        return -1;
    }

    adc_blks[adc].flags &= ~(1 << event);

    return 0;
}

