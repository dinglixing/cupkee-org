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
#include "dev_usart.h"

#if 0
#define _TRACE(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define _TRACE(fmt, ...)    //
#endif

typedef struct usart_ctrl_t {
    int           instance;
    val_t         *events_handle[USART_EVENT_MAX];
    hw_usart_conf_t conf;
} usart_ctrl_t;

static usart_ctrl_t controls[USART_INSTANCE_MAX];
static const char *usart_confs[] = {
    "baudrate", "databits", "stopbits", "parity"
};
static const char *usart_parity_opts[] = {
    "none", "odd", "even"
};
static const char *usart_events[] = {
    "data", "drain", "error"
};

static int usart_baudrate_set(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    usart_ctrl_t *control = (usart_ctrl_t *) dev->data;
    hw_usart_conf_t *conf = &control->conf;

    (void) env;

    if (!val_is_number(setting)) {
        return -CUPKEE_EINVAL;
    }

    conf->baudrate = val_2_integer(setting);

    return CUPKEE_OK;
}

static val_t usart_baudrate_get(cupkee_device_t *dev, env_t *env)
{
    usart_ctrl_t *control = (usart_ctrl_t *) dev->data;
    hw_usart_conf_t *conf = &control->conf;

    (void) env;

    return val_mk_number(conf->baudrate);
}

static int usart_databits_set(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    usart_ctrl_t *control = (usart_ctrl_t *) dev->data;
    hw_usart_conf_t *conf = &control->conf;

    (void) env;

    if (!val_is_number(setting)) {
        return -CUPKEE_EINVAL;
    }

    conf->databits = val_2_integer(setting);

    return CUPKEE_OK;
}

static val_t usart_databits_get(cupkee_device_t *dev, env_t *env)
{
    usart_ctrl_t *control = (usart_ctrl_t *) dev->data;
    hw_usart_conf_t *conf = &control->conf;

    (void) env;

    return val_mk_number(conf->databits);
}

static int usart_stopbits_set(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    usart_ctrl_t *control = (usart_ctrl_t *) dev->data;
    hw_usart_conf_t *conf = &control->conf;

    (void) env;

    if (!val_is_number(setting)) {
        return -CUPKEE_EINVAL;
    }

    conf->stopbits = val_2_integer(setting);

    return CUPKEE_OK;
}

static val_t usart_stopbits_get(cupkee_device_t *dev, env_t *env)
{
    usart_ctrl_t *control = (usart_ctrl_t *) dev->data;
    hw_usart_conf_t *conf = &control->conf;

    (void) env;

    return val_mk_number(conf->stopbits);
}

static int usart_parity_set(cupkee_device_t *dev, env_t *env, val_t *setting)
{
    usart_ctrl_t *control = (usart_ctrl_t *) dev->data;
    hw_usart_conf_t *conf = &control->conf;
    int parity = cupkee_id(setting, OPT_USART_PARITY_MAX, usart_parity_opts);

    (void) env;

    if (parity >= OPT_USART_PARITY_MAX) {
        return -CUPKEE_EINVAL;
    }

    conf->parity = parity;

    return CUPKEE_OK;
}

static val_t usart_parity_get(cupkee_device_t *dev, env_t *env)
{
    usart_ctrl_t *control = (usart_ctrl_t *) dev->data;
    hw_usart_conf_t *conf = &control->conf;

    (void) env;

    return val_mk_foreign_string((intptr_t)usart_parity_opts[conf->parity]);
}

static device_config_handle_t config_handles[] = {
    {usart_baudrate_set,  usart_baudrate_get},
    {usart_databits_set,  usart_databits_get},
    {usart_stopbits_set,  usart_stopbits_get},
    {usart_parity_set,  usart_parity_get},
};

/************************************************************************************
 * xxx driver interface
 ***********************************************************************************/
static int usart_init(cupkee_device_t *dev)
{
    int instance = hw_usart_alloc();

    if (instance >= 0 && instance < ADC_MAX) {
        usart_ctrl_t *control = &controls[instance];
        int i;

        hw_usart_conf_reset(&control->conf);
        control->instance = instance;
        for (i = 0; i < USART_EVENT_MAX; i++) {
            control->events_handle[i] = NULL;
        }

        dev->data = control;
        return CUPKEE_OK;
    }
    return -CUPKEE_ERESOURCE;
}

static int usart_deinit(cupkee_device_t *dev)
{
    usart_ctrl_t *control= (usart_ctrl_t*)dev->data;

    if (!control) {
        return -CUPKEE_EINVAL;
    }

    hw_usart_disable(control->instance);
    dev->data = NULL;

    return CUPKEE_OK;
}

static int usart_enable(cupkee_device_t *dev)
{
    usart_ctrl_t *control= (usart_ctrl_t*)dev->data;
    return hw_usart_enable(control->instance, &control->conf);
}

static int usart_disable(cupkee_device_t *dev)
{
    usart_ctrl_t *control= (usart_ctrl_t*)dev->data;
    return hw_usart_disable(control->instance);
}

static int usart_listen(cupkee_device_t *dev, val_t *event, val_t *callback)
{
    int event_id = cupkee_id(event, USART_EVENT_MAX, usart_events);
    usart_ctrl_t *control;

    if (event_id >= USART_EVENT_MAX) {
        return -CUPKEE_EINVAL;
    }

    control = (usart_ctrl_t*)dev->data;
    if (hw_usart_event_enable(control->instance, event_id)) {
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

    _TRACE("listen %s ok\n", usart_events[event_id]);
    return CUPKEE_OK;
}

static int usart_ignore(cupkee_device_t *dev, val_t *event)
{
    int event_id = cupkee_id(event, USART_EVENT_MAX, usart_events);
    usart_ctrl_t *control;

    if (event_id >= USART_EVENT_MAX) {
        return -CUPKEE_EINVAL;
    }

    control = (usart_ctrl_t*)dev->data;
    if (hw_usart_event_disable(control->instance, event_id)) {
        return -CUPKEE_ERROR;
    }

    if (control->events_handle[event_id]) {
        reference_release(control->events_handle[event_id]);
        control->events_handle[event_id] = NULL;
    }

    return CUPKEE_OK;
}

static device_config_handle_t *usart_config(cupkee_device_t *dev, val_t *name)
{
    int id;

    (void) dev;

    if (!name) {
        return NULL;
    }

    id = cupkee_id(name, CFG_USART_MAX, usart_confs);
    if (id < CFG_USART_MAX) {
        return &config_handles[id];
    } else {
        return NULL;
    }
}

static val_t usart_write(cupkee_device_t *dev, env_t *env, int ac, val_t *av)
{
    usart_ctrl_t *control = (usart_ctrl_t*)dev->data;
    val_t cb_param[3];
    void *addr;
    int   size, n;
    int   err = 0;

    if (device_param_stream(ac, av, &addr, &size)) {
        cb_param[2] = *av++; ac--;
    } else {
        err = CUPKEE_EINVAL;
        goto DO_ERROR;
    }

    if (device_param_int(ac, av, &n)) {
        ac--; av++;
        if (n < 0) {
            err = CUPKEE_EINVAL;
            goto DO_ERROR;
        } else
        if (n > size) {
            n = size;
        }
    } else {
        n = size;
    }

    n = hw_usart_send(control->instance, n, addr);
    if (n < 0) {
        err = CUPKEE_EHARDWARE;
        goto DO_ERROR;
    }

    if (ac > 0) {
        val_set_undefined(cb_param);
        val_set_number(cb_param + 1, n);
        cupkee_do_callback(env, av, 3, cb_param);
    }
    return val_mk_number(n);

DO_ERROR:
    if (ac) {
        cupkee_do_callback_error(env, av, err);
    }
    return VAL_FALSE;
}

static void usart_read_data(int instance, env_t *env, val_t *cb, int n)
{
    type_buffer_t *buffer = buffer_create(env, n);

    if (!buffer) {
        cupkee_do_callback_error(env, cb, CUPKEE_ERESOURCE);
        return;
    } else {
        val_t cb_param[2];

        hw_usart_recv_load(instance, n, buffer->buf);

        val_set_undefined(cb_param);
        val_set_buffer(cb_param + 1, buffer);
        cupkee_do_callback(env, cb, 2, cb_param);
    }
}

static void usart_post_data(int instance, env_t *env, val_t *cb)
{
    int n = hw_usart_recv_len(instance);
    type_buffer_t *buffer = buffer_create(env, n);

    if (buffer) {
        val_t param;

        hw_usart_recv_load(instance, n, buffer->buf);

        val_set_buffer(&param, buffer);
        cupkee_do_callback(env, cb, 1, &param);
    }
}

static val_t usart_read(cupkee_device_t *dev, env_t *env, int ac, val_t *av)
{
    usart_ctrl_t *control = (usart_ctrl_t*)dev->data;
    int   size, n, err;

    size = hw_usart_recv_len(control->instance);
    if (size < 0) {
        err = CUPKEE_EHARDWARE;
        goto DO_ERROR;
    }

    if (device_param_int(ac, av, &n)) {
        ac--; av++;
        if (n < 0) {
            err = CUPKEE_EINVAL;
            goto DO_ERROR;
        }
    } else {
        n = 1;
    }

    if (ac && val_is_function(av)) {
        if (n > size) {
            n = size;
        }
        usart_read_data(control->instance, env, av, n);
    } else {
        hw_usart_recv_load(control->instance, n, NULL);
    }
    return VAL_TRUE;

DO_ERROR:
    if (ac) {
        cupkee_do_callback_error(env, av, err);
    }
    return VAL_FALSE;
}

static void usart_event_handle(env_t *env, uint8_t which, uint8_t event)
{
    _TRACE("get usart(%u) event: %u\n", which, event);

    if (which < USART_INSTANCE_MAX) {
        usart_ctrl_t *control = &controls[which];
        val_t param;

        switch(event) {
        case USART_EVENT_DATA:
            usart_post_data(which, env, control->events_handle[USART_EVENT_DATA]);
            break;
        case USART_EVENT_DRAIN:
            cupkee_do_callback(env, control->events_handle[USART_EVENT_DRAIN], 0, NULL);
            break;
        case USART_EVENT_ERROR:
            val_set_number(&param, CUPKEE_ERROR);
            cupkee_do_callback(env, control->events_handle[USART_EVENT_ERROR], 1, &param);
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
const cupkee_driver_t cupkee_driver_usart = {
    .init    = usart_init,
    .deinit  = usart_deinit,
    .enable  = usart_enable,
    .disable = usart_disable,

    .config  = usart_config,
    .read    = usart_read,
    .write   = usart_write,

    .listen  = usart_listen,
    .ignore  = usart_ignore,
    .event_handle = usart_event_handle,
};

/************************************************************************************
 * xxx native
 ***********************************************************************************/

/************************************************************************************
 * xxx setup
 ***********************************************************************************/
int dev_setup_usart(void)
{
    return 0;
}

