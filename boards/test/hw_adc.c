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

/*******************************************************************************
 * dbg field
*******************************************************************************/
#include "hardware.h"

static uint16_t dbg_adc_channels[18];
static uint16_t dbg_adc_flags = 0;

uint16_t hw_dbg_adc_get_channel(int channel)
{
    return dbg_adc_channels[channel];
}

void hw_dbg_adc_set_channel(int channel, uint16_t value)
{
    dbg_adc_channels[channel] = value;
}

void hw_dbg_adc_set_ready(void) {
    dbg_adc_flags |= 1;
}

void hw_dbg_adc_clr_ready(void) {
    dbg_adc_flags &= ~1;
}

int  hw_dbg_adc_test_ready(void) {
    return dbg_adc_flags & 1;
}

void hw_dbg_adc_set_eoc(void) {
    dbg_adc_flags |= 2;
}

void hw_dbg_adc_clr_eoc(void) {
    dbg_adc_flags &= ~2;
}

int  hw_dbg_adc_test_eoc(void) {
    return dbg_adc_flags & 2;
}

/*******************************************************************************
 * hw field
*******************************************************************************/
#include <bsp.h>

#if 0
#define _TRACE(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define _TRACE(fmt, ...)    //
#endif

#define ADC_INUSED      0x80
#define ADC_ENABLE      0x40

#define ADC_IDEL        0
#define ADC_READY       0x20
#define ADC_WIP         0x10      // work in process
#define ADC_STATE_MASK  0x30

#define ADC_EVENT_MASK  0x0f

#define ADC_INVALID_VAL HW_INVALID_VAL

typedef struct hw_adc_t{
    uint8_t flags;
    uint8_t channels[3];
    uint32_t interval;
    uint32_t count;
} hw_adc_t;

static hw_adc_t       adc_blks[ADC_MAX];
static uint32_t       adc_vals[ADC_CHANNEL_MAX];

static inline int hw_adc_check(int adc) {
    return (adc < ADC_MAX && (adc_blks[adc].flags & ADC_INUSED));
}

static inline void hw_adc_channel_set(hw_adc_t *blk, int channel)
{
    int sel = (channel / 8) & 3;
    int bit = channel & 7;

    blk->channels[sel] |= 1 << bit;
}

static inline int hw_adc_channel_test(hw_adc_t *blk, int channel)
{
    int sel = (channel / 8) & 3;
    int bit = channel & 7;

    return blk->channels[sel] & (1 << bit);
}

static void hw_adc_on(hw_adc_t *blk)
{
    (void) blk;

    _TRACE("... adc on\n");
    // hardware setting code here
}

static inline int hw_adc_wait_ready(hw_adc_t *blk)
{
    (void) blk;


    if (hw_dbg_adc_test_ready()) {
        if (blk->flags & (1 << ADC_EVENT_READY)) {
            devices_event_post(ADC_DEVICE_ID, blk - adc_blks, ADC_EVENT_READY);
        }
        return 1;
    } else {
        _TRACE("... wait adc ready\n");
        return 0;
    }
}

static void hw_adc_off(hw_adc_t *blk)
{
    (void) blk;

    // hardware setting code here

    _TRACE("... adc off\n");
    blk->flags &= ~ADC_STATE_MASK;
}

static int hw_adc_start(hw_adc_t *blk)
{
    if (blk->count == 0) {
        // hardware setting code here
        _TRACE("... adc convert start\n");
        return blk->channels[0] || blk->channels[1] || blk->channels[2];
    } else {
        _TRACE("... adc interval: %u\n", blk->count);
        blk->count--;
        return 0;
    }
}

static void hw_adc_load(hw_adc_t *blk)
{
    int i, converted = 0;

    for (i = 0; i < ADC_CHANNEL_MAX; i++) {
        if (hw_adc_channel_test(blk, i)) {
            // hardware read convert value
            adc_vals[i] = hw_dbg_adc_get_channel(i);
            converted++;
        }
    }

    if (converted && (blk->flags & (1 << ADC_EVENT_DATA))) {
        _TRACE("post adc event: data\n");
        devices_event_post(ADC_DEVICE_ID, blk - adc_blks, ADC_EVENT_DATA);
    }

    blk->count = blk->interval;
}

static inline int hw_adc_wait_convert(hw_adc_t *blk)
{
    if (hw_dbg_adc_test_eoc()) {
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

    blk->interval = cfg->interval;
    for (i = 0; i < cfg->chn_num; i++) {
        hw_adc_channel_set(blk, cfg->chn_seq[i]);
    }
    blk->count = blk->interval;

    hw_adc_on(blk);

    return 0;
}

static int hw_adc_config_clr(hw_adc_t *blk)
{
    int i;

    hw_adc_off(blk);

    for (i = 0; i < ADC_CHANNEL_MAX; i++) {
        if (hw_adc_channel_test(blk, i)) {
            adc_vals[i] = ADC_INVALID_VAL;
        }
    }

    blk->interval = 0;
    blk->channels[0] = 0;
    blk->channels[1] = 0;
    blk->channels[2] = 0;

    return 0;
}

int hw_adc_setup(void)
{
    int i;

    // hardware setup code here
    for (i = 0; i < ADC_MAX; i++) {
        adc_blks[i].flags = 0;
        adc_blks[i].channels[0] = 0;
        adc_blks[i].channels[1] = 0;
        adc_blks[i].channels[2] = 0;
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
                if (hw_adc_wait_ready(blk)) {
                    blk->flags |= ADC_READY;
                }
                break;
            case ADC_READY:
                if (hw_adc_start(blk)) {
                    blk->flags |= ADC_WIP;
                }
                break;
            case (ADC_READY | ADC_WIP):
                if (hw_adc_wait_convert(blk)) {
                    blk->flags &= ~ADC_WIP;
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
            adc_blks[i].channels[0] = 0;
            adc_blks[i].channels[1] = 0;
            adc_blks[i].channels[2] = 0;
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

int hw_adc_read(int adc, int off, uint32_t *data)
{
    if (!hw_adc_check(adc)) {
        return -1;
    }

    if (off < ADC_CHANNEL_MAX) {
        *data = adc_vals[off];
        return 1;
    } else {
        return 0;
    }
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

