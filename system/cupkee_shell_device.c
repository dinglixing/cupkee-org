/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016-2017 Lixing Ding <ding.lixing@gmail.com>

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

#include <cupkee.h>

#include "cupkee_shell_misc.h"
#include "cupkee_shell_device.h"

typedef union device_handle_set_t {
    intptr_t param;
    uint8_t  handles[DEVICE_EVENT_MAX];
} device_handle_set_t;

static int device_is_true(intptr_t ptr);
static void device_op_prop(void *env, intptr_t ptr, val_t *name, val_t *prop);
static void device_op_elem(void *env, intptr_t devid, val_t *which, val_t *elem);

static const char *category_names[3] = {
    "MAP", "STREAM", "BLOCK"
};
static const char *device_opt_dir[] = {
    "in", "out", "dual"
};

static const char *device_opt_polarity[] = {
    "positive", "negative", "edge"
};

static const char *device_opt_parity[] = {
    "none", "odd", "even",
};

static const char *device_opt_stopbits[] = {
    "1bit", "2bit", "0.5bit", "1.5bit"
};

static const char *device_event_names[] = {
    "error", "data", "drain", "ready"
};

static const val_foreign_op_t device_op = {
    .is_true = device_is_true,
    .prop = device_op_prop,
    .elem = device_op_elem,
};

static const char *device_category_name(uint8_t category)
{
    if (category < DEVICE_CATEGORY_MAX) {
        return category_names[category];
    } else {
        return "?";
    }
}

static intptr_t device_id_gen(cupkee_device_t *dev)
{
    int id = cupkee_device_id(dev);

    return dev->magic + (id << 8);
}

static cupkee_device_t *device_id_block(intptr_t id)
{
    cupkee_device_t *dev;
    uint8_t magic = (uint8_t) id;

    dev = cupkee_device_block(id >> 8);
    if (dev && dev->magic == magic) {
        return dev;
    } else {
        return NULL;
    }
}

static cupkee_device_t *device_val_block(val_t *v)
{
    val_foreign_t *vf;

    if (val_is_foreign(v)) {
        vf = (val_foreign_t *)val_2_intptr(v);
        if (vf->op == &device_op) {
            return device_id_block(vf->data);
        }
    }
    return NULL;
}

static void device_list(void)
{
    const cupkee_device_desc_t *desc;
    int i;

    console_log_sync("\r\n%8s%6s%6s%6s:%s\r\n", "DEVICE", "CONF", "INST", "TYPE", "CATEGORY");
    for (i = 0, desc = device_entrys[0]; desc; desc = device_entrys[++i]) {
        console_log_sync("%8s%6d%6d%6d:%s\r\n",
                desc->name,
                desc->conf_num,
                hw_device_instances(desc->type),
                desc->type,
                device_category_name(desc->category));
    }
}

static int device_is_true(intptr_t ptr)
{
    (void) ptr;

    return 0;
}

static void device_op_prop(void *env, intptr_t id, val_t *name, val_t *prop)
{
    cupkee_device_t *dev = device_id_block(id);
    const char *prop_name = val_2_cstring(name);

    (void) env;

    if (dev && prop_name) {
        if (!strcmp(prop_name, "read")) {
            val_set_native(prop, (intptr_t)native_device_read);
            return;
        } else
        if (!strcmp(prop_name, "write")) {
            val_set_native(prop, (intptr_t)native_device_write);
            return;
        } else
        if (!strcmp(prop_name, "get")) {
            val_set_native(prop, (intptr_t)native_device_get);
            return;
        } else
        if (!strcmp(prop_name, "set")) {
            val_set_native(prop, (intptr_t)native_device_set);
            return;
        } else
        if (!strcmp(prop_name, "config")) {
            val_set_native(prop, (intptr_t)native_device_config);
            return;
        } else
        if (!strcmp(prop_name, "enable")) {
            val_set_native(prop, (intptr_t)native_device_enable);
            return;
        } else
        if (!strcmp(prop_name, "disable")) {
            val_set_native(prop, (intptr_t)native_device_disable);
            return;
        } else
        if (!strcmp(prop_name, "listen")) {
            val_set_native(prop, (intptr_t)native_device_listen);
            return;
        } else
        if (!strcmp(prop_name, "ignore")) {
            val_set_native(prop, (intptr_t)native_device_ignore);
            return;
        } else
        if (!strcmp(prop_name, "isEnabled")) {
            val_set_native(prop, (intptr_t)native_device_is_enabled);
            return;
        } else
        if (!strcmp(prop_name, "destroy")) {
            val_set_native(prop, (intptr_t)native_device_destroy);
            return;
        } else
        if (!strcmp(prop_name, "error")) {
            val_set_number(prop, dev->error);
            return;
        } else
        if (!strcmp(prop_name, "instance")) {
            val_set_number(prop, dev->instance);
            return;
        } else
        if (!strcmp(prop_name, "type")) {
            val_set_foreign_string(prop, (intptr_t) dev->desc->name);
            return;
        } else
        if (!strcmp(prop_name, "category")) {
            val_set_foreign_string(prop, (intptr_t) device_category_name(dev->desc->category));
            return;
        } else
        if (!strcmp(prop_name, "received")) {
            val_set_number(prop, (intptr_t) cupkee_device_received(dev));
            return;
        }
    }
    val_set_undefined(prop);
}

static void device_op_elem(void *env, intptr_t id, val_t *which, val_t *elem)
{
    if (val_is_number(which)) {
        cupkee_device_t *dev = device_id_block(id);
        uint32_t val;

        if (dev && 0 < cupkee_device_get(dev, val_2_integer(which), &val)) {
            val_set_number(elem, val);
        } else {
            val_set_undefined(elem);
        }
    } else {
        device_op_prop(env, id, which, elem);
    }
}

static void device_get_option(val_t *opt, int i, int max, const char **opt_list)
{
    if (i >= max) {
        i = 0;
    }

    val_set_foreign_string(opt, (intptr_t) opt_list[i]);
}

static void device_get_sequence(env_t *env, val_t *val, uint8_t n, uint8_t *seq)
{
    array_t *a;
    int i;

    a = _array_create(env, n);
    if (a) {
        for (i = 0; i < n; i++) {
            val_set_number(_array_elem(a, i), seq[i]);
        }

        val_set_array(val, (intptr_t) a);
    }
}

static int device_set_uint8(val_t *val, uint8_t *conf)
{
    if (val_is_number(val)) {
        *conf = val_2_integer(val);
        return CUPKEE_OK;
    }
    return -CUPKEE_EINVAL;
}

static int device_set_uint16(val_t *val, uint16_t *conf)
{
    if (val_is_number(val)) {
        *conf = val_2_integer(val);
        return CUPKEE_OK;
    }
    return -CUPKEE_EINVAL;
}

static int device_set_uint32(val_t *val, uint32_t *conf)
{
    if (val_is_number(val)) {
        *conf = val_2_integer(val);
        return CUPKEE_OK;
    }
    return -CUPKEE_EINVAL;
}

static int device_set_option(val_t *val, uint8_t *conf, int max, const char **opt_list)
{
    int opt = shell_val_id(val, max, opt_list);

    if (opt >= 0) {
        *conf = opt;
        return CUPKEE_OK;
    }
    return -CUPKEE_EINVAL;
}

static int device_set_sequence(val_t *val, int max, uint8_t *n, uint8_t *seq)
{
    int i, len;
    val_t *elems;

    if (val_is_array(val)) {
        array_t *array = (array_t *)val_2_intptr(val);

        len   = array_len(array);
        if (len > max) {
            return -CUPKEE_EINVAL;
        }
        elems = array_values(array);
    } else
    if (val_is_number(val)) {
        len = 1;
        elems = val;
    } else{
        return -CUPKEE_EINVAL;
    }

    for (i = 0; i < len; i++) {
        val_t *cur = elems + i;
        int    num;

        if (!val_is_number(cur)) {
            return -CUPKEE_EINVAL;
        }

        num = val_2_integer(cur);
        if (num > 255) {
            return -CUPKEE_EINVAL;
        }
        seq[i] = num;
    }

    if (len) {
        *n = len;
        return CUPKEE_OK;
    } else {
        return -CUPKEE_EINVAL;
    }
}

static int device_convert_data(val_t *data, void **addr)
{
    int size;

    if (val_is_buffer(data)) {
        size = _val_buffer_size(data);
        *addr = _val_buffer_addr(data);
    } else
    if ((size = string_len(data)) > 0) {
        *addr = (void *) val_2_cstring(data);
    } else {
        *addr = NULL;
    }
    return size;
}

static int device_pin_config_get(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_pin_t *pin = (hw_config_pin_t *) conf;

    (void) env;

    switch (which) {
    case DEVICE_PIN_CONF_NUM:   val_set_number(val, pin->num);   break;
    case DEVICE_PIN_CONF_START: val_set_number(val, pin->start); break;
    case DEVICE_PIN_CONF_DIR:   device_get_option(val, pin->dir, DEVICE_OPT_DIR_MAX, device_opt_dir); break;
    default:                    return -CUPKEE_EINVAL;
    }

    return CUPKEE_OK;
}

static int device_pin_config_set(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_pin_t *pin = (hw_config_pin_t *) conf;

    (void) env;

    switch (which) {
    case DEVICE_PIN_CONF_NUM:   return device_set_uint8(val, &pin->num);
    case DEVICE_PIN_CONF_START: return device_set_uint8(val, &pin->start);
    case DEVICE_PIN_CONF_DIR:   return device_set_option(val, &pin->dir, DEVICE_OPT_DIR_MAX, device_opt_dir);
    default:                    return -CUPKEE_EINVAL;
    }
}

static int device_adc_config_get(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_adc_t *adc = (hw_config_adc_t *) conf;

    switch (which) {
    case DEVICE_ADC_CONF_CHANNELS: device_get_sequence(env, val, adc->chn_num, adc->chn_seq);   break;
    case DEVICE_ADC_CONF_INTERVAL: val_set_number(val, adc->interval); break;
    default:                       return -CUPKEE_EINVAL;
    }

    return CUPKEE_OK;
}

static int device_adc_config_set(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_adc_t *adc = (hw_config_adc_t *) conf;

    (void) env;

    switch (which) {
    case DEVICE_ADC_CONF_CHANNELS: return device_set_sequence(val, HW_CHN_MAX_ADC, &adc->chn_num, adc->chn_seq);
    case DEVICE_ADC_CONF_INTERVAL: return device_set_uint16(val, &adc->interval);
    default:                       return -CUPKEE_EINVAL;
    }
}

static int device_timer_config_get(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_timer_t *timer = (hw_config_timer_t *) conf;

    switch (which) {
    case DEVICE_TIMER_CONF_CHANNELS: device_get_sequence(env, val, timer->chn_num, timer->chn_seq);   break;
    case DEVICE_TIMER_CONF_POLARITY: device_get_option(val, timer->polarity, DEVICE_OPT_POLARITY_MAX, device_opt_polarity); break;
    default:                       return -CUPKEE_EINVAL;
    }
    return CUPKEE_OK;
}

static int device_timer_config_set(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_timer_t *timer = (hw_config_timer_t *) conf;

    (void) env;
    switch (which) {
    case DEVICE_TIMER_CONF_CHANNELS: return device_set_sequence(val, HW_CHN_MAX_TIMER, &timer->chn_num, timer->chn_seq);
    case DEVICE_TIMER_CONF_POLARITY: return device_set_option(val, &timer->polarity, DEVICE_OPT_POLARITY_MAX, device_opt_polarity); break;
    default:                       return -CUPKEE_EINVAL;
    }
}

static int device_pwm_config_get(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_pwm_t *pwm = (hw_config_pwm_t *) conf;

    switch (which) {
    case DEVICE_PWM_CONF_CHANNELS: device_get_sequence(env, val, pwm->chn_num, pwm->chn_seq);   break;
    case DEVICE_PWM_CONF_POLARITY: device_get_option(val, pwm->polarity, DEVICE_OPT_POLARITY_MAX, device_opt_polarity); break;
    case DEVICE_PWM_CONF_PERIOD:   val_set_number(val, pwm->period);   break;
    default:                       return -CUPKEE_EINVAL;
    }
    return CUPKEE_OK;
}

static int device_pwm_config_set(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_pwm_t *pwm = (hw_config_pwm_t *) conf;

    (void) env;
    switch (which) {
    case DEVICE_PWM_CONF_CHANNELS: return device_set_sequence(val, HW_CHN_MAX_PWM, &pwm->chn_num, pwm->chn_seq);
    case DEVICE_PWM_CONF_POLARITY: return device_set_option(val, &pwm->polarity, DEVICE_OPT_POLARITY_MAX, device_opt_polarity); break;
    case DEVICE_PWM_CONF_PERIOD:   return device_set_uint16(val, &pwm->period);   break;
    default:                       return -CUPKEE_EINVAL;
    }
}

static int device_pulse_config_get(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_pulse_t *pulse= (hw_config_pulse_t *) conf;

    switch (which) {
    case DEVICE_PULSE_CONF_CHANNELS: device_get_sequence(env, val, pulse->chn_num, pulse->chn_seq);   break;
    case DEVICE_PULSE_CONF_POLARITY: device_get_option(val, pulse->polarity, DEVICE_OPT_POLARITY_MAX - 1, device_opt_polarity); break;
    default:                         return -CUPKEE_EINVAL;
    }
    return CUPKEE_OK;
}

static int device_pulse_config_set(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_pulse_t *pulse= (hw_config_pulse_t *) conf;

    (void) env;

    switch (which) {
    case DEVICE_PULSE_CONF_CHANNELS: return device_set_sequence(val, HW_CHN_MAX_PULSE, &pulse->chn_num, pulse->chn_seq);
    case DEVICE_PULSE_CONF_POLARITY: return device_set_option(val, &pulse->polarity, DEVICE_OPT_POLARITY_MAX - 1, device_opt_polarity);
    default:                         return -CUPKEE_EINVAL;
    }
    return CUPKEE_OK;
}

static int device_counter_config_get(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_counter_t *counter = (hw_config_counter_t *) conf;

    switch (which) {
    case DEVICE_COUNTER_CONF_CHANNELS: device_get_sequence(env, val, counter->chn_num, counter->chn_seq);   break;
    case DEVICE_COUNTER_CONF_POLARITY: device_get_option(val, counter->polarity, DEVICE_OPT_POLARITY_MAX, device_opt_polarity); break;
    case DEVICE_COUNTER_CONF_PERIOD:   val_set_number(val, counter->period);   break;
    default:                       return -CUPKEE_EINVAL;
    }
    return CUPKEE_OK;
}

static int device_counter_config_set(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_counter_t *counter = (hw_config_counter_t *) conf;

    (void) env;
    switch (which) {
    case DEVICE_COUNTER_CONF_CHANNELS: return device_set_sequence(val, HW_CHN_MAX_COUNTER, &counter->chn_num, counter->chn_seq);
    case DEVICE_COUNTER_CONF_POLARITY: return device_set_option(val, &counter->polarity, DEVICE_OPT_POLARITY_MAX, device_opt_polarity); break;
    case DEVICE_COUNTER_CONF_PERIOD:   return device_set_uint16(val, &counter->period);   break;
    default:                       return -CUPKEE_EINVAL;
    }
}

static int device_uart_config_get(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_uart_t *uart = (hw_config_uart_t *) conf;

    (void) env;

    switch (which) {
    case DEVICE_UART_CONF_BAUDRATE: val_set_number(val, uart->baudrate);  break;
    case DEVICE_UART_CONF_DATABITS: val_set_number(val, uart->data_bits); break;
    case DEVICE_UART_CONF_STOPBITS:
        device_get_option(val, uart->stop_bits, DEVICE_OPT_STOPBITS_MAX, device_opt_stopbits);
        break;
    case DEVICE_UART_CONF_PARITY:
        device_get_option(val, uart->parity, DEVICE_OPT_PARITY_MAX, device_opt_parity);
        break;
    default: break;
    }

    return CUPKEE_OK;
}

static int device_uart_config_set(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_uart_t *uart = (hw_config_uart_t *) conf;

    (void) env;

    switch (which) {
    case DEVICE_UART_CONF_BAUDRATE: return device_set_uint32(val, &uart->baudrate);  break;
    case DEVICE_UART_CONF_DATABITS: return device_set_uint8(val, &uart->data_bits); break;
    case DEVICE_UART_CONF_STOPBITS:
        return device_set_option(val, &uart->stop_bits, DEVICE_OPT_STOPBITS_MAX, device_opt_stopbits);
    case DEVICE_UART_CONF_PARITY:
        return device_set_option(val, &uart->parity, DEVICE_OPT_PARITY_MAX, device_opt_parity);
    default: break;
    }

    return CUPKEE_EINVAL;
}

static int device_config_set(cupkee_device_t *dev, env_t *env, int index, val_t *val)
{
    if (index >= 0 && index < dev->desc->conf_num) {
        switch (dev->desc->type) {
        case DEVICE_TYPE_PIN:      return device_pin_config_set  (env, &dev->config, index, val);
        case DEVICE_TYPE_ADC:      return device_adc_config_set  (env, &dev->config, index, val);
        case DEVICE_TYPE_DAC:      break;
        case DEVICE_TYPE_PWM:      return device_pwm_config_set  (env, &dev->config, index, val);
        case DEVICE_TYPE_PULSE:    return device_pulse_config_set(env, &dev->config, index, val);
        case DEVICE_TYPE_TIMER:    return device_timer_config_set(env, &dev->config, index, val);
        case DEVICE_TYPE_COUNTER:  return device_counter_config_set(env, &dev->config, index, val);
        case DEVICE_TYPE_UART:     return device_uart_config_set (env, &dev->config, index, val);
        case DEVICE_TYPE_USART:
        case DEVICE_TYPE_SPI:
        case DEVICE_TYPE_I2C:
        case DEVICE_TYPE_USB_CDC:
        default: break;
        }
        return -CUPKEE_EIMPLEMENT;
    }

    return -CUPKEE_EINVAL;
}

static int device_config_get(cupkee_device_t *dev, env_t *env, int index, val_t *val)
{
    if (index >= 0 && index < dev->desc->conf_num) {
        switch (dev->desc->type) {
        case DEVICE_TYPE_PIN:      return device_pin_config_get(env, &dev->config, index, val);
        case DEVICE_TYPE_ADC:      return device_adc_config_get(env, &dev->config, index, val);
        case DEVICE_TYPE_DAC:      return -CUPKEE_EIMPLEMENT; break;
        case DEVICE_TYPE_PWM:      return device_pwm_config_get(env, &dev->config, index, val);
        case DEVICE_TYPE_PULSE:    return device_pulse_config_get(env, &dev->config, index, val);
        case DEVICE_TYPE_TIMER:    return device_timer_config_get(env, &dev->config, index, val);
        case DEVICE_TYPE_COUNTER:  return device_counter_config_get(env, &dev->config, index, val);
        case DEVICE_TYPE_UART:     return device_uart_config_get(env, &dev->config, index, val);
        case DEVICE_TYPE_USART:
        case DEVICE_TYPE_SPI:
        case DEVICE_TYPE_I2C:
        case DEVICE_TYPE_USB_CDC:
        default: return -CUPKEE_EIMPLEMENT;
        }
    }

    return -CUPKEE_EINVAL;
}

static val_t device_config_set_one(cupkee_device_t *dev, env_t *env, val_t *which, val_t *val)
{
    int index = shell_val_id(which, dev->desc->conf_num, dev->desc->conf_names);

    return device_config_set(dev, env, index, val) == CUPKEE_OK ? VAL_TRUE : VAL_FALSE;
}

static val_t device_config_get_one(cupkee_device_t *dev, env_t *env, val_t *which)
{
    int index = shell_val_id(which, dev->desc->conf_num, dev->desc->conf_names);
    val_t val;

    if (CUPKEE_OK == device_config_get(dev, env, index, &val)) {
        return val;
    } else {
        return VAL_UNDEFINED;
    }
}

static int device_config_set_all(cupkee_device_t *dev, env_t *env, val_t *settings)
{
    object_iter_t it;
    const char *key;
    val_t *val;

    if (object_iter_init(&it, settings)) {
        return -CUPKEE_EINVAL;
    }

    while (object_iter_next(&it, &key, &val)) {
        int index = shell_str_id(key, dev->desc->conf_num, dev->desc->conf_names);

        if (index < 0) {
            // skip unknowned config items
            continue;
        }

        if (device_config_set(dev, env, index, val)) {
            return -CUPKEE_EINVAL;
        }
    }

    return CUPKEE_OK;
}

static val_t device_config_get_all(cupkee_device_t *dev)
{
    (void) dev;
    return VAL_UNDEFINED;
}

static void device_get_all(cupkee_device_t *dev, env_t *env, val_t *result)
{
    int status;
    uint32_t data;

    status = cupkee_device_get(dev, -1, &data);
    if (status > 0) {
        // support combine read
        val_set_number(result, data);
        return;
    } else
    if (status == 0 && dev->driver->size){
        int i, size;
        array_t *list;

        size = dev->driver->size(dev->instance);
        list = _array_create(env, size);
        if (list) {
            for (i = 0; i < size; i++) {
                if (cupkee_device_get(dev, i, &data) > 0) {
                    val_set_number(_array_elem(list, i), data);
                } else {
                    val_set_undefined(_array_elem(list, i));
                }
            }

            val_set_array(result, (intptr_t) list);
            return;
        }
    }

    val_set_undefined(result);
}

static void device_get_elem(cupkee_device_t *dev, env_t *env, int offset, val_t *res)
{
    uint32_t data;

    if (offset >= 0) {
        if (cupkee_device_get(dev, offset, &data) > 0) {
            val_set_number(res, data);
        } else {
            *res = VAL_UNDEFINED;
        }
    } else {
        device_get_all(dev, env, res);
    }
}

static int device_read_stream_n(cupkee_device_t *dev, env_t *env, int n, val_t *res)
{
    type_buffer_t *buffer;
    int err;

    buffer = buffer_create(env, n);
    if (!buffer) {
        err = -CUPKEE_ERESOURCE;
    } else {
        err = cupkee_device_recv(dev, n, buffer->buf);
    }

    if (err >= 0) {
        val_set_buffer(res, buffer);
    }

    return err;
}

static val_t device_read_stream(cupkee_device_t *dev, env_t *env, int ac, val_t *av)
{
    int n, max, err;
    val_t args[2];

    max = cupkee_device_received(dev);
    if (ac && val_is_number(av)) {
        n = val_2_integer(av);
        if (n < 0) {
            n = 0;
        } else
        if (n > max) {
            n = max;
        }
        ac--; av++;
    } else {
        n = max;
    }

    err = device_read_stream_n(dev, env, n, &args[1]);
    if (ac && val_is_function(av)) {
        if (err < 0) {
            shell_do_callback_error(env, av, err);
        } else {
            val_set_undefined(&args[0]);
            shell_do_callback(env, av, 2, args);
        }
    }

    return err < 0 ? VAL_FALSE : VAL_TRUE;
}

static val_t device_write_stream(cupkee_device_t *dev, env_t *env, int ac, val_t *av)
{
    void *addr;
    int   size, offset = 0, send = 0, err = 0;
    val_t *data;

    (void) env;

    if (ac < 1 || (size = device_convert_data(av, &addr)) < 0) {
        err = -CUPKEE_EINVAL;
    } else {
        data = av; ac--; av++;

        if (ac && val_is_number(av)) {
            send = val_2_integer(av);
            ac--; av++;
        } else {
            send = size;
        }

        if (ac && val_is_number(av)) {
            offset = send;
            send = val_2_integer(av);
            ac--; av++;
        }

        if (offset < 0 || send < 0) {
            err = -CUPKEE_EINVAL;
        }
    }

    if (err) {
        if (ac) {
            shell_do_callback_error(env, av, err);
        }
        return VAL_FALSE;
    }

    if (send > 0 && offset < size) {
        if (offset + send > size) {
            send = size - offset;
        }
        send = cupkee_device_send(dev, send, addr + offset);
    } else {
        send = 0;
    }

    if (ac) {
        val_t args[3];

        val_set_undefined(args);
        args[1] = *data;
        val_set_number(args + 2, send);

        shell_do_callback(env, av, 3, args);
    }

    return VAL_TRUE;
}

static val_t device_read_block(cupkee_device_t *dev, env_t *env, int ac, val_t *av)
{
    (void) dev;
    (void) env;
    (void) ac;
    (void) av;
    return VAL_FALSE;
}

static val_t device_write_block(cupkee_device_t *dev, env_t *env, int ac, val_t *av)
{
    (void) dev;
    (void) env;
    (void) ac;
    (void) av;
    return VAL_FALSE;
}

static int device_event_handle_set(cupkee_device_t *dev, int event, val_t *cb)
{
    device_handle_set_t *set = (device_handle_set_t *)&dev->handle_param;
    uint8_t ref_id;

    ref_id = set->handles[event];
    if (ref_id) {
        val_t *ref = shell_reference_ptr(ref_id);
        if (!ref) {
            return -CUPKEE_ERROR;
        }
        *ref = *cb;
    } else {
        val_t *ref = shell_reference_create(cb);
        if (!ref) {
            return -CUPKEE_ERESOURCE;
        }
        set->handles[event] = shell_reference_id(ref);
    }

    return CUPKEE_OK;
}

static val_t *device_event_handle_get(cupkee_device_t *dev, int event)
{
    device_handle_set_t *set = (device_handle_set_t *)&dev->handle_param;

    return shell_reference_ptr(set->handles[event]);
}

static void device_event_handle_release(cupkee_device_t *dev)
{
    device_handle_set_t *set = (device_handle_set_t *)&dev->handle_param;
    int i;

    for (i = 0; i < DEVICE_EVENT_MAX; i++) {
        val_t *ref = shell_reference_ptr(set->handles[i]);

        if (ref) {
            shell_reference_release(ref);
        }
    }
}

static void device_map_data_proc(cupkee_device_t *dev, env_t *env, val_t *handle)
{
    val_t info;

    device_get_all(dev, env, &info);

    shell_do_callback(env, handle, 1, &info);
}

static void device_stream_data_proc(cupkee_device_t *dev, env_t *env, val_t *handle)
{
    int err, n;

    n = cupkee_device_received(dev);
    if (n > 0) {
        val_t data;

        err = device_read_stream_n(dev, env, n, &data);
        if (err < 0) {
            shell_do_callback_error(env, handle, err);
        } else {
            shell_do_callback(env, handle, 1, &data);
        }
    }
}

static void device_block_data_proc(cupkee_device_t *dev, env_t *env, val_t *handle)
{
    (void) dev;
    (void) env;
    (void) handle;
}

static void device_event_handle_wrap(cupkee_device_t *dev, uint8_t code, intptr_t param)
{
    env_t *env = cupkee_shell_env();
    val_t *handle = device_event_handle_get(dev, code);

    (void) param;

    if (!handle) {
        return;
    }

    if (code == DEVICE_EVENT_ERR) {
        shell_do_callback_error(env, handle, dev->error);
    } else
    if (code == DEVICE_EVENT_DATA) {
        switch(dev->desc->category) {
        case DEVICE_CATEGORY_MAP:     device_map_data_proc(dev, env, handle); break;
        case DEVICE_CATEGORY_STREAM:  device_stream_data_proc(dev, env, handle); break;
        case DEVICE_CATEGORY_BLOCK:   device_block_data_proc(dev, env, handle); break;
        default:                break;
        }
    } else
    if (code == DEVICE_EVENT_DRAIN) {
        shell_do_callback(env, handle, 0, NULL);
    } else
    if (code == DEVICE_EVENT_READY) {
        // Todo:
    } else {
        // What happen ?
    }
}

val_t native_device_destroy(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    (void) env;

    if (ac && (dev = device_val_block(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_FALSE;
    }

    device_event_handle_release(dev);
    if (CUPKEE_OK == cupkee_device_release(dev)) {
        return VAL_TRUE;
    } else {
        return VAL_FALSE;
    }
}

val_t native_device_config(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;
    val_t *which = NULL;
    val_t *setting;

    if (ac && (dev = device_val_block(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (val_is_number(av) || val_is_string(av)) {
        which = av++; ac--;
    }
    setting = ac > 0 ? av : NULL;

    if (setting) {
        // config set is forbidden, if device is enabled
        if (cupkee_device_is_enabled(dev)) {
            return VAL_FALSE;
        }

        return which ? device_config_set_one(dev, env, which, setting) :
                       device_config_set_all(dev, env, setting) ? VAL_FALSE : VAL_TRUE;
    } else {
        return which ? device_config_get_one(dev, env, which) :
                       device_config_get_all(dev);
    }
}

val_t native_device_get(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;
    val_t result;
    int offset;

    if (ac && (dev= device_val_block(av))) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (ac && val_is_number(av)) {
        offset = val_2_integer(av);
    } else {
        offset = -1;
    }

    device_get_elem(dev, env, offset, &result);
    return result;
}

val_t native_device_set(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;
    int offset, err = 0;
    uint32_t data;

    (void) env;

    if (ac && (dev = device_val_block(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_FALSE;
    }

    if (ac < 1) {
        // Do nothing
        return VAL_TRUE;
    } else
    if (ac == 1) {
        if (val_is_number(av)) {
            offset = -1; // set all
            data = val_2_integer(av);
        } else {
            err = -CUPKEE_EINVAL;
        }
    } else {
        if (val_is_number(av) && val_is_number(av + 1)) {
            offset = val_2_integer(av);
            data = val_2_integer(av + 1);
        } else {
            err = -CUPKEE_EINVAL;
        }
    }

    if (!err) {
        err = cupkee_device_set(dev, offset, data);
    }

    return err > 0 ? VAL_TRUE : VAL_FALSE;
}

val_t native_device_write(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    if (ac && (dev = device_val_block(av))) {
        av++; ac--;
    } else {
        return VAL_FALSE;
    }

    switch (dev->desc->category) {
    case DEVICE_CATEGORY_STREAM: return device_write_stream(dev, env, ac, av);
    case DEVICE_CATEGORY_BLOCK:  return device_write_block (dev, env, ac, av);
    default: return VAL_FALSE;
    }
}

val_t native_device_read(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    if (ac && (dev = device_val_block(av))) {
        av++; ac--;
    } else {
        return VAL_FALSE;
    }

    switch (dev->desc->category) {
    case DEVICE_CATEGORY_STREAM: return device_read_stream(dev, env, ac, av);
    case DEVICE_CATEGORY_BLOCK:  return device_read_block (dev, env, ac, av);
    default: return VAL_FALSE;
    }
}

val_t native_device_listen(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;
    val_t *callback;
    int event_id;

    (void) env;

    if (ac >= 3 && (dev = device_val_block(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_FALSE;
    }

    event_id = shell_val_id(av, DEVICE_EVENT_MAX, device_event_names);
    callback = av + 1;
    if (event_id < 0 || event_id > DEVICE_EVENT_MAX || !val_is_function(callback)) {
        return VAL_FALSE;
    }

    if (CUPKEE_OK == device_event_handle_set(dev, event_id, callback)) {
        return VAL_TRUE;
    } else {
        return VAL_FALSE;
    }
}

val_t native_device_ignore(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;
    int event_id;

    (void) env;

    if (ac >= 2 && (dev = device_val_block(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    event_id = shell_val_id(av, DEVICE_EVENT_MAX, device_event_names);
    if (0 <= event_id && event_id < DEVICE_EVENT_MAX) {
        device_handle_set_t *set = (device_handle_set_t *)&dev->handle_param;
        uint8_t ref_id = set->handles[event_id];

        set->handles[event_id] = 0;
        shell_reference_release(shell_reference_ptr(ref_id));

        return VAL_TRUE;
    } else {
        return VAL_FALSE;
    }
}

val_t native_device_is_enabled(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    (void) env;

    if (ac == 0 || (dev = device_val_block(av)) == NULL) {
        return VAL_UNDEFINED;
    } else {
        return cupkee_device_is_enabled(dev) ? VAL_TRUE : VAL_FALSE;
    }
}

val_t native_device_enable(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;
    val_t *setting, *hnd;
    int err = 0;

    (void) env;

    if (ac && (dev = device_val_block(av)) != NULL) {
        hnd = av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    setting = ac > 0 ? av : NULL;
    if (setting && val_is_object(setting)) {
        if (cupkee_device_is_enabled(dev)) {
            err = -CUPKEE_EENABLED;
        } else {
            err = device_config_set_all(dev, env, setting);
        }
        ac--; av++;
    }

    if (!err) {
        err = cupkee_device_enable(dev);
    }

    if (ac && val_is_function(av)) {
        val_t args[2];

        args[0] = err ? val_mk_number(err) : VAL_UNDEFINED;
        args[1] = *hnd;

        shell_do_callback(env, av, 2, args);
    }

    return (err == CUPKEE_OK) ? VAL_TRUE : VAL_FALSE;
}

val_t native_device_disable(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;

    (void) env;

    if (ac == 0 || (dev = device_val_block(av)) == NULL) {
        return VAL_UNDEFINED;
    }
    cupkee_device_disable(dev);

    return  cupkee_device_is_enabled(dev) ? VAL_FALSE : VAL_TRUE;
}

val_t native_device_create(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *dev;
    const char *name;
    int inst;

    if (ac == 0) {
        device_list();
        return VAL_UNDEFINED;
    }

    name = val_2_cstring(av);
    if (!name) {
        return VAL_UNDEFINED;
    }

    if (ac > 1 && val_is_number(av + 1)) {
        inst= val_2_integer(av + 1);
    } else {
        inst= 0;
    }

    dev = cupkee_device_request(name, inst);
    if (dev) {
        dev->handle = device_event_handle_wrap;
        dev->handle_param = 0;
        return val_create(env, &device_op, device_id_gen(dev));
    } else {
        return VAL_UNDEFINED;
    }
}

