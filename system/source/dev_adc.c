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

#if 0
#define _TRACE(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define _TRACE(fmt, ...)    //
#endif

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
    "ready", "error", "data"
};

static int adc_channel_add(adc_ctrl_t *control, val_t *v) {
    int chn = val_2_integer(v);

    if (chn >= ADC_CHANNEL_MAX) {
        return -1;
    }

    if (control->conf.chn_num >= ADC_CHANNEL_MAX) {
        return 0;
    }

    control->conf.chn_seq[control->conf.chn_num++] = chn;
    return 1;
}

static int adc_set_channel(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    adc_ctrl_t *control = (adc_ctrl_t*)dev->data;

    (void) env;

    control->conf.chn_num = 0;
    if (val_is_number(setting)) {
        return (1 == adc_channel_add(control, setting)) ? 0 : CUPKEE_EINVAL;
    } else
    if (val_is_array(setting)) {
        val_t *v;
        int i = 0;

        while (i < ADC_CHANNEL_MAX && NULL != (v = _array_element(setting, i++))) {
            if (!val_is_number(v) || 0 > adc_channel_add(control, v)) {
                control->conf.chn_num = 0;
                return -CUPKEE_EINVAL;
            }
        }

        return 0;
    }

    return -CUPKEE_EINVAL;
}

static val_t adc_get_channel(cupkee_device_t *dev, env_t *env)
{
    adc_ctrl_t *control = (adc_ctrl_t*)dev->data;

    if (control) {
        array_t *a = _array_create(env, control->conf.chn_num);
        int i;

        if (!a) {
            return VAL_UNDEFINED;
        }

        for (i = 0; i < control->conf.chn_num; i++) {
            val_set_number(_array_elem(a, i), control->conf.chn_seq[i]);
        }

        return val_mk_array(a);
    } else {
        return VAL_UNDEFINED;
    }

}

static int adc_set_interval(cupkee_device_t *dev, env_t *env, val_t *setting)
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

static val_t adc_get_interval(cupkee_device_t *dev, env_t *env)
{
    adc_ctrl_t *control = (adc_ctrl_t*)dev->data;

    (void) env;

    if (!control) {
        return VAL_UNDEFINED;
    }

    return val_mk_number(control->conf.interval);
}

static device_config_handle_t config_handles[] = {
    {adc_set_channel,  adc_get_channel},
    {adc_set_interval, adc_get_interval},
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
    adc_ctrl_t *control;

    if (event_id >= ADC_EVENT_MAX) {
        return -CUPKEE_EINVAL;
    }

    control = (adc_ctrl_t*)dev->data;
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

    _TRACE("listen %s ok\n", adc_events[event_id]);
    return CUPKEE_OK;
}

static int adc_ignore(cupkee_device_t *dev, val_t *event)
{
    int event_id = cupkee_id(event, ADC_EVENT_MAX, adc_events);
    adc_ctrl_t *control;

    if (event_id >= ADC_EVENT_MAX) {
        return -CUPKEE_EINVAL;
    }

    control = (adc_ctrl_t*)dev->data;
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

static val_t adc_write(cupkee_device_t *dev, env_t *env, int ac, val_t *av)
{
    (void) dev;
    (void) env;
    (void) ac;
    (void) av;

    return VAL_FALSE;
}

static val_t adc_read_all(adc_ctrl_t *control, env_t *env)
{
    array_t *a = _array_create(env, control->conf.chn_num);
    int i;

    if (!a) {
        return VAL_UNDEFINED;
    }

    for (i = 0; i < control->conf.chn_num; i++) {
        uint32_t v;

        if (hw_adc_read(control->adc, control->conf.chn_seq[i], &v)) {
            if (v != HW_INVALID_VAL) {
                val_set_number(_array_elem(a, i), v);
                continue;
            }
        }
        val_set_undefined(_array_elem(a, i));
    }

    return val_mk_array(a);
}

static val_t adc_read(cupkee_device_t *dev, env_t *env, int ac, val_t *av)
{
    adc_ctrl_t *control = (adc_ctrl_t*)dev->data;
    uint32_t v;
    int off = -1;

    if (ac > 0 && val_is_number(av)) {
        off = val_2_integer(av);
    }

    if (off < 0) {
        return adc_read_all(control, env);
    }

    if (off < control->conf.chn_num && hw_adc_read(control->adc, control->conf.chn_seq[off], &v) > 0) {
        if (v != HW_INVALID_VAL) {
            return val_mk_number(v);
        }
    }
    return VAL_UNDEFINED;
}

static void adc_event_handle(env_t *env, uint8_t which, uint8_t event)
{
    _TRACE("get adc(%u) event: %u\n", which, event);

    if (which < ADC_MAX) {
        adc_ctrl_t *control = &controls[which];
        val_t param;

        switch(event) {
        case ADC_EVENT_READY:
            cupkee_do_callback(env, control->events_handle[ADC_EVENT_READY], 0, NULL);
            break;
        case ADC_EVENT_ERROR:
            val_set_number(&param, CUPKEE_ERROR);
            cupkee_do_callback(env, control->events_handle[ADC_EVENT_ERROR], 1, &param);
            break;
        case ADC_EVENT_DATA:
            param = adc_read_all(control, env);
            cupkee_do_callback(env, control->events_handle[ADC_EVENT_DATA], 1, &param);
            break;
        default:
            // TODO: 
            break;
        }
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

