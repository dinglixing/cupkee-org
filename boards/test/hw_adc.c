/*******************************************************************************
 * dbg field
*******************************************************************************/
#include "hardware.h"

static uint16_t dbg_adc_channels[18];

uint16_t hw_dbg_adc_get_channel(int channel)
{
    return dbg_adc_channels[channel];
}

void hw_dbg_adc_set_channel(int channel, uint16_t value)
{
    dbg_adc_channels[channel] = value;
}

/*******************************************************************************
 * hw field
*******************************************************************************/
#include <bsp.h>

typedef struct hw_adc_t{
    uint8_t inused;
    uint8_t enable;
    uint8_t listen;
    uint8_t last;
} hw_adc_t;

static hw_adc_t       adc_blks[ADC_MAX];
static hw_adc_conf_t *adc_cfgs[ADC_MAX];

static inline int hw_adc_check(int adc) {
    return (adc < ADC_MAX && adc_blks[adc].inused);
}

static int hw_adc_config_set(int adc, hw_adc_conf_t *cfg)
{
    // hardware setting code here
    adc_cfgs[adc] = cfg;
    return 0;
}

static int hw_adc_config_clr(int adc)
{
    // hardware clear setting code here
    adc_cfgs[adc] = NULL;
    return 0;
}

int hw_adc_setup(void)
{
    int i;

    // hardware setup code here
    for (i = 0; i < ADC_MAX; i++) {
        adc_blks[i].inused = 0;
        adc_blks[i].enable = 0;
        adc_blks[i].listen = 0;
        adc_blks[i].last   = 0;
    }

    return 0;
}

void hw_adc_poll(void)
{
    int i;
    for (i = 0; i < ADC_MAX; i++) {
    }
}

int hw_adc_alloc(void)
{
    int i;

    for (i = 0; i < ADC_MAX; i++) {
        if (adc_blks[i].inused == 0) {

            adc_blks[i].inused = 1;
            adc_blks[i].enable = 0;
            adc_blks[i].last = 0;

            return i;
        }
    }

    return -1;
}

int hw_adc_release(int adc)
{
    if (hw_adc_check(adc)) {
        adc_blks[adc].inused = 0;
        return 0;
    }
    return -1;
}

void hw_adc_conf_reset(hw_adc_conf_t *conf)
{
    conf->channels = 0;
    conf->interval = 0;
}

int hw_adc_enable(int adc, hw_adc_conf_t *cfg)
{
    if (!hw_adc_check(adc)) {
        return -1;
    }

    if (!adc_blks[adc].enable) {
        if (0 == hw_adc_config_set(adc, cfg)) {
            adc_blks[adc].enable = 1;
            return 0;
        }
    }

    return -1;
}

int hw_adc_disable(int adc)
{
    if (!hw_adc_check(adc)) {
        return -1;
    }

    if (adc_blks[adc].enable) {
        if (0 == hw_adc_config_clr(adc)) {
            adc_blks[adc].enable = 0;
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

    //hw_adc_conf_t *conf = adc_cfgs[adc];

    return 0;
}

int hw_adc_event_enable(int adc, int event)
{
    if (!hw_adc_check(adc) || event >= ADC_EVENT_MAX) {
        return -1;
    }

    adc_blks[adc].listen |= (1 << event);

    return 0;
}

int hw_adc_event_disable(int adc, int event)
{
    if (!hw_adc_check(adc) || event >= ADC_EVENT_MAX) {
        return -1;
    }

    adc_blks[adc].listen &= ~(1 << event);

    return 0;
}
