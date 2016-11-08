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

#include "device.h"
#include "misc.h"
#include "dev_adc.h"

typedef struct adc_ctrl_t {
    int           adc;
    val_t         *events_handle[ADC_EVENT_MAX];
    hw_adc_conf_t conf;
} adc_ctrl_t;

static adc_ctrl_t controls[ADC_MAX];
static const char *adc_confs[] = {
    "channel", "interval"
};
static const char *adc_events[] = {
    "data"
};

static int set_channel(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    adc_ctrl_t *control = (adc_ctrl_t*)dev->data;

    (void) env;

    if (val_is_number(setting)) {
        int chn = val_2_integer(setting);

        control->conf.channels = chn;
        return 0;
    }

    return -CUPKEE_EINVAL;
}

static val_t get_channel(cupkee_device_t *dev, env_t *env)
{
    adc_ctrl_t *control = (adc_ctrl_t*)dev->data;

    (void) env;

    if (!control) {
        return VAL_UNDEFINED;
    }

    return val_mk_number(control->conf.channels);
}

static int set_interval(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    adc_ctrl_t *control = (adc_ctrl_t*)dev->data;

    (void) env;

    if (val_is_number(setting)) {
        int interval = val_2_integer(setting);

        control->conf.interval = interval;
        return 0;
    }

    return -CUPKEE_EINVAL;
}

static val_t get_interval(cupkee_device_t *dev, env_t *env)
{
    adc_ctrl_t *control = (adc_ctrl_t*)dev->data;

    (void) env;

    if (!control) {
        return VAL_UNDEFINED;
    }

    return val_mk_number(control->conf.interval);
}

static device_config_handle_t config_handles[] = {
    {set_channel,  get_channel},
    {set_interval, get_interval},
};

/************************************************************************************
 * xxx driver interface
 ***********************************************************************************/
static int adc_init(cupkee_device_t *dev)
{
    int adc = hw_adc_alloc();

    if (adc >= 0 && adc < ADC_MAX) {
        adc_ctrl_t *control = &controls[adc];
        int i;

        hw_adc_conf_reset(&control->conf);
        control->adc = adc;
        for (i = 0; i < ADC_EVENT_MAX; i++) {
            control->events_handle[i] = NULL;
        }

        dev->data = control;
        return CUPKEE_OK;
    }
    return -CUPKEE_ERESOURCE;
}

static int adc_deinit(cupkee_device_t *dev)
{
    adc_ctrl_t *control= (adc_ctrl_t*)dev->data;

    if (!control) {
        return -CUPKEE_EINVAL;
    }

    hw_adc_disable(control->adc);
    dev->data = NULL;

    return CUPKEE_OK;
}

static int adc_enable(cupkee_device_t *dev)
{
    adc_ctrl_t *control= (adc_ctrl_t*)dev->data;
    return hw_adc_enable(control->adc, &control->conf);
}

static int adc_disable(cupkee_device_t *dev)
{
    adc_ctrl_t *control= (adc_ctrl_t*)dev->data;
    return hw_adc_disable(control->adc);
}

static int adc_listen(cupkee_device_t *dev, val_t *event, val_t *callback)
{
    int event_id = cupkee_id(event, ADC_EVENT_MAX, adc_events);

    if (event_id >= ADC_EVENT_MAX) {
        return -CUPKEE_EINVAL;
    }
    adc_ctrl_t *control= (adc_ctrl_t*)dev->data;

    if (hw_adc_event_enable(control->adc, event_id)) {
        return -CUPKEE_ERROR;
    }

    if (control->events_handle[event_id] == NULL) {
        val_t *ref = reference_create(callback);

        if (!ref) {
            return -CUPKEE_ERESOURCE;
        }
        control->events_handle[event_id] = ref;
    } else {
        *control->events_handle[event_id] = *callback;
    }

    return CUPKEE_OK;
}

static int adc_ignore(cupkee_device_t *dev, val_t *event)
{
    int event_id = cupkee_id(event, ADC_EVENT_MAX, adc_events);

    if (event_id >= ADC_EVENT_MAX) {
        return -CUPKEE_EINVAL;
    }
    adc_ctrl_t *control= (adc_ctrl_t*)dev->data;

    if (hw_adc_event_disable(control->adc, event_id)) {
        return -CUPKEE_ERROR;
    }

    if (control->events_handle[event_id]) {
        reference_release(control->events_handle[event_id]);
        control->events_handle[event_id] = NULL;
    }

    return CUPKEE_OK;
}

static device_config_handle_t *adc_config(cupkee_device_t *dev, val_t *name)
{
    int id;

    (void) dev;

    if (!name) {
        return NULL;
    }

    id = cupkee_id(name, CFG_ADC_MAX, adc_confs);
    if (id < CFG_ADC_MAX) {
        return &config_handles[id];
    } else {
        return NULL;
    }
}

static val_t adc_write(cupkee_device_t *dev, val_t *data)
{
    (void) dev;
    (void) data;

    return VAL_FALSE;
}

static val_t adc_read(cupkee_device_t *dev, int off)
{
    adc_ctrl_t *control= (adc_ctrl_t*)dev->data;

    (void) control;
    (void) off;

    return VAL_UNDEFINED;
}

static void adc_event_handle(env_t *env, uint8_t which, uint8_t event)
{
    if (which < ADC_MAX && event < ADC_EVENT_MAX) {
        cupkee_do_callback(env, controls[which].events_handle[ADC_EVENT_DATA], 0, NULL);
    }
}

/************************************************************************************
 * xxx driver exports
 ***********************************************************************************/
const cupkee_driver_t cupkee_driver_adc = {
    .init    = adc_init,
    .deinit  = adc_deinit,
    .enable  = adc_enable,
    .disable = adc_disable,

    .config  = adc_config,
    .read    = adc_read,
    .write   = adc_write,

    .listen  = adc_listen,
    .ignore  = adc_ignore,
    .event_handle = adc_event_handle,
};

/************************************************************************************
 * xxx native
 ***********************************************************************************/

/************************************************************************************
 * xxx setup
 ***********************************************************************************/
int dev_setup_adc(void)
{
    return 0;
}

