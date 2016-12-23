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

#include "device.h"
#include "util.h"
#include "misc.h"

#if 0
#define _TRACE(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define _TRACE(fmt, ...)    //
#endif

#define DEVICE_MAX          (16)

static int device_is_true(intptr_t id);

static void device_op_prop(void *env, intptr_t devid, val_t *name, val_t *prop);
static void device_op_elem(void *env, intptr_t devid, val_t *name, val_t *prop);

const char *device_categorys[] = {
    "map", "stream", "block"
};

const char *device_opt_dir[] = {
    "in", "out", "dual"
};

const char *device_opt_polarity[] = {
    "positive", "negative", "edge"
};

const char *device_opt_parity[] = {
    "none", "odd", "even",
};

const char *device_opt_stopbits[] = {
    "1bit", "2bit", "0.5bit", "1.5bit"
};

const char *device_pin_conf_names[] = {
    "pinNum", "pinStart", "dir"
};

const char *device_adc_conf_names[] = {
    "channel", "interval"
};

const char *device_dac_conf_names[] = {
    "channel", "interval"
};

const char *device_pwm_pulse_timer_counter_conf_names[] = {
    "channel", "polarity", "period"
};

const char *device_uart_conf_names[] = {
    "baudrate", "dataBits", "stopBits", "parity"
};

static const char *device_event_names[] = {
    "error", "data", "drain", "ready"
};

static const cupkee_device_desc_t device_pin = {
    .name = "pin",
    .type = DEVICE_TYPE_PIN,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 3,
    .event_mask = 0x3,
    .conf_names = device_pin_conf_names,
    .set = device_pin_set,
    .get = device_pin_get,
};

static const cupkee_device_desc_t device_adc = {
    .name = "adc",
    .type = DEVICE_TYPE_ADC,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 2,
    .event_mask = 0x3,
    .conf_names = device_adc_conf_names,
    .set = device_adc_set,
    .get = device_adc_get
};

static const cupkee_device_desc_t device_pwm = {
    .name = "pwm",
    .type = DEVICE_TYPE_PWM,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 3,
    .event_mask = 0x3,
    .conf_names = device_pwm_pulse_timer_counter_conf_names,
    .set = device_pwm_set,
    .get = device_pwm_get
};

static const cupkee_device_desc_t device_pulse = {
    .name = "pulse",
    .type = DEVICE_TYPE_PULSE,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 2,
    .event_mask = 0x1,
    .conf_names = device_pwm_pulse_timer_counter_conf_names,
    .set = device_pulse_set,
    .get = device_pulse_get
};

static const cupkee_device_desc_t device_timer = {
    .name = "timer",
    .type = DEVICE_TYPE_TIMER,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 2,
    .event_mask = 0x3,
    .conf_names = device_pwm_pulse_timer_counter_conf_names,
    .set = device_timer_set,
    .get = device_timer_get
};

static const cupkee_device_desc_t device_counter = {
    .name = "counter",
    .type = DEVICE_TYPE_COUNTER,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 2,
    .event_mask = 0x3,
    .conf_names = device_pwm_pulse_timer_counter_conf_names,
    .set = device_counter_set,
    .get = device_counter_get
};

static const cupkee_device_desc_t device_uart = {
    .name = "uart",
    .type = DEVICE_TYPE_UART,
    .category = DEVICE_CATEGORY_STREAM,
    .conf_num = 4,
    .event_mask = 0x7,
    .conf_names = device_uart_conf_names,
    .def = device_uart_def,
    .set = device_uart_set,
    .get = device_uart_get,
};

static const cupkee_device_desc_t *device_entrys[] = {
    &device_pin,
    &device_adc,
    &device_pwm,
    &device_pulse,
    &device_timer,
    &device_counter,
    &device_uart,
};

static cupkee_device_t device_heap[DEVICE_MAX];
static cupkee_device_t *device_free = NULL;
static cupkee_device_t *device_work = NULL;
static uint16_t device_magic_factor = 1;
static const val_foreign_op_t device_op = {
    .is_true = device_is_true,
    .prop = device_op_prop,
    .elem = device_op_elem,
};

static inline int device_is_enabled(cupkee_device_t *device)
{
    return device->flags & DEVICE_FL_ENABLE;
}

static int device_is_true(intptr_t devid)
{
    int magic;

    magic = devid & 0xffff;
    devid >>= 16 & 0xff;

    if (devid >= DEVICE_MAX || device_heap[devid].magic != magic) {
        return 0;
    } else {
        return 1;
    }
}

static inline intptr_t device_id_gen(cupkee_device_t *device) {
    return ((device - device_heap) << 16) | device->magic;
}

static inline cupkee_device_t *device_id_control(intptr_t devid)
{
    int magic;

    magic = devid & 0xffff;
    devid >>= 16 & 0xff;

    if (devid >= DEVICE_MAX || device_heap[devid].magic != magic) {
        return NULL;
    } else {
        return &device_heap[devid];
    }
}

static cupkee_device_t *device_val_control(val_t *d)
{
    val_foreign_t *vf;

    if (val_is_foreign(d)) {
        vf = (val_foreign_t *)val_2_intptr(d);
        if (vf->op == &device_op) {
            return device_id_control(vf->data);
        }
    }
    return NULL;
}

static cupkee_device_t *device_block_alloc(void)
{
    cupkee_device_t *device = device_free;

    if (device) {
        int i;
        device_free = device->next;

        for (i = 0; i < DEVICE_EVENT_MAX; i++) {
            device->event_handle[i] = NULL;
        }

        device->flags = 0;
        device->event = 0;

        device->magic = (device_magic_factor + 1) ^ (device_magic_factor << 8);
        device_magic_factor++;
    }
    return device;
}

static int device_block_release(cupkee_device_t *device)
{
    if (device) {
        memset(device, 0, sizeof(cupkee_device_t));
        device->next = device_free;
        device_free = device;
        return 0;
    }

    return -1;
}

static const cupkee_device_desc_t *device_desc_get_by_name(const char *name)
{
    int i, max = sizeof(device_entrys) / sizeof(cupkee_device_desc_t *);

    for (i = 0; i < max; i++) {
        if (!strcmp(device_entrys[i]->name, name)) {
            return device_entrys[i];
        }
    }

    return NULL;
}

static void device_show(void)
{
    int i, max = sizeof(device_entrys) / sizeof(cupkee_device_desc_t *);

    hw_console_sync_puts("DEVICE TYPE:CATEGORY CONF INST\r\n");
    for (i = 0; i < max; i++) {
        const cupkee_device_desc_t *desc = device_entrys[i];
        char buf[32];
        int n;

        // name
        n = strlen(desc->name);
        hw_console_sync_puts(desc->name);
        while(n++ < 8) {
            hw_console_sync_putc(' ');
        }

        // id
        snprintf(buf, 16, "%2d ", desc->type);
        hw_console_sync_puts(buf);

        // type
        if (desc->category == DEVICE_CATEGORY_MAP) {
            hw_console_sync_puts("  M  ");
        } else
        if (desc->category == DEVICE_CATEGORY_STREAM) {
            hw_console_sync_puts("  S  ");
        } else
        if (desc->category == DEVICE_CATEGORY_BLOCK) {
            hw_console_sync_puts("  B  ");
        } else {
            hw_console_sync_puts("  ?  ");
        }

        // conf
        snprintf(buf, 32, " %2d\t%2d\r\n", desc->conf_num, hw_device_instances(desc->type));
        hw_console_sync_puts(buf);
    }
}

static void device_work_list_join(cupkee_device_t *device)
{
    device->next = device_work;
    device_work = device;
}

static void device_work_list_drop(cupkee_device_t *device)
{
    cupkee_device_t *cur = device_work;

    if (cur == device) {
        device_work = cur->next;
        return;
    }

    while(cur) {
        cupkee_device_t *next = cur->next;

        if (next == device) {
            cur->next = device->next;
            return;
        }

        cur = next;
    }
}

static int device_enable(cupkee_device_t *device)
{
    if (!device_is_enabled(device)) {
        int devid = device - device_heap;
        int err;

        err = device->driver->setup(device->inst, devid, &device->conf);
        if (!err) {
            device->flags |= DEVICE_FL_ENABLE;
            device_work_list_join(device);
        }
        return err;
    }

    return 0;
}

static void device_disable(cupkee_device_t *device)
{
    if (device->flags & DEVICE_FL_ENABLE) {
        device->driver->reset(device->inst);
        device->flags &= ~DEVICE_FL_ENABLE;
        device->error = 0; // clean error

        device_work_list_drop(device);
    }
}

static cupkee_device_t *device_create(const char *name, int instance)
{
    cupkee_device_t  *device;
    const cupkee_device_desc_t *desc;
    const hw_driver_t *driver;

    if (!name || instance < 0) {
        return NULL;
    }

    desc = device_desc_get_by_name(name);
    if (!desc) {
        return NULL;
    }

    driver = hw_device_request(desc->type, instance);
    if (!driver) {
        return NULL;
    }

    device = device_block_alloc();
    if (device) {
        device->inst = instance;
        device->desc = desc;
        device->driver = driver;

        if (device->desc->def) {
            device->desc->def(&device->conf);
        } else {
            memset(&device->conf, 0, sizeof(hw_config_t));
        }
    } else {
        driver->release(instance);
    }

    return device;
}

static int device_destroy(cupkee_device_t *device)
{
    device_disable(device);
    device->driver->release(device->inst);
    return device_block_release(device);
}

static val_t device_config_set(cupkee_device_t *device, env_t *env, val_t *name, val_t *val)
{
    int which = device_string_map_var(name, device->desc->conf_num,
                                            device->desc->conf_names);

    if (which >= 0 && which < device->desc->conf_num) {
        if (CUPKEE_OK == device->desc->set(env, &device->conf, which, val)) {
            return VAL_TRUE;
        } else {
            return VAL_FALSE;
        }
    }

    return VAL_FALSE;
}

static int device_config_set_all(cupkee_device_t *device, env_t *env, val_t *settings)
{
    object_iter_t it;
    const char *key;
    val_t *val;

    if (object_iter_init(&it, settings)) {
        return -CUPKEE_EINVAL;
    }

    while (object_iter_next(&it, &key, &val)) {
        int which = device_string_map(key, device->desc->conf_num,
                                           device->desc->conf_names);

        if (which < 0) {
            // skip unknowned config items
            continue;
        }

        if (device->desc->set(env, &device->conf, which, val)) {
            return -CUPKEE_EINVAL;
        }
    }

    return CUPKEE_OK;
}

static val_t device_config_get(cupkee_device_t *device, env_t *env, val_t *name)
{
    int which = device_string_map_var(name, device->desc->conf_num,
                                            device->desc->conf_names);

    if (which >= 0 && which < device->desc->conf_num) {
        val_t setting;
        if (CUPKEE_OK == device->desc->get(env, &device->conf, which, &setting)) {
            return setting;
        }
    }

    return VAL_UNDEFINED;
}

static val_t device_config_get_all(cupkee_device_t *device)
{
    (void) device;
    return VAL_UNDEFINED;
}

static val_t device_native_destroy(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;

    (void) env;

    if (ac && (device = device_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_FALSE;
    }

    if (0 != device_destroy(device)) {
        return VAL_FALSE;
    } else {
        return VAL_TRUE;
    }
}

static val_t device_native_config(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;
    val_t *name = NULL;
    val_t *setting;

    (void) env;

    if (ac && (device = device_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (val_is_number(av) || val_is_string(av)) {
        name = av++; ac--;
    }
    setting = ac > 0 ? av : NULL;

    if (setting) {
        // config set is forbidden, if device is enabled
        if (device_is_enabled(device)) {
            return VAL_FALSE;
        }

        return name ? device_config_set(device, env, name, setting) :
                      device_config_set_all(device, env, setting) ? VAL_FALSE : VAL_TRUE;
    } else {
        return name ? device_config_get(device, env, name) :
                      device_config_get_all(device);
    }
}

static val_t device_native_enable(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;
    val_t *setting, *hnd;
    int err = 0;

    (void) env;

    if (ac && (device = device_val_control(av)) != NULL) {
        hnd = av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    setting = ac > 0 ? av : NULL;
    if (setting && val_is_object(setting)) {
        if (device_is_enabled(device)) {
            err = -CUPKEE_EENABLED;
        } else {
            err = device_config_set_all(device, env, setting);
        }
        ac--; av++;
    }

    if (!err) {
        err = device_enable(device);
    }

    if (ac && val_is_function(av)) {
        val_t args[2];

        args[0] = err ? val_mk_number(err) : VAL_UNDEFINED;
        args[1] = *hnd;

        cupkee_do_callback(env, av, 2, args);
    }

    return (err == 0) ? VAL_TRUE : VAL_FALSE;
}

static val_t device_native_disable(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;

    (void) env;

    if (ac == 0 || (device = device_val_control(av)) == NULL) {
        return VAL_UNDEFINED;
    }

    device_disable(device);
    return  device_is_enabled(device) ? VAL_FALSE : VAL_TRUE;
}

static val_t device_native_is_enabled(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;

    (void) env;

    if (ac == 0 || (device = device_val_control(av)) == NULL) {
        return VAL_UNDEFINED;
    } else {
        return device_is_enabled(device) ? VAL_TRUE : VAL_FALSE;
    }
}

static val_t device_native_listen(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;
    val_t *callback;
    int event_id;
    uint8_t event_bit;

    (void) env;

    if (ac >= 3 && (device = device_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    callback = av + 1;
    event_id = device_string_map_var(av, DEVICE_EVENT_MAX, device_event_names);
    if ( event_id < 0 || !val_is_function(callback)) {
        return VAL_FALSE;
    }
    event_bit = 1 << event_id;

    if (!(event_bit & device->desc->event_mask)) {
        return VAL_FALSE;
    }

    if (device->event_handle[event_id] == NULL) {
        val_t *ref = reference_create(callback);

        if (!ref) {
            return VAL_FALSE;
        }
        device->event_handle[event_id] = ref;
    } else {
        // origion ref released, automic
        *device->event_handle[event_id] = *callback;
    }
    device->event |= event_bit;

    return VAL_TRUE;
}

static val_t device_native_ignore(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;
    int event_id;
    uint8_t event_bit;

    (void) env;

    if (ac >= 2 && (device = device_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    event_id = device_string_map_var(av, DEVICE_EVENT_MAX, device_event_names);
    if (0 > event_id) {
        return VAL_FALSE;
    }
    event_bit = 1 << event_id;

    if (event_bit & device->event) {
        reference_release(device->event_handle[event_id]);
        device->event_handle[event_id] = NULL;

        device->event &= ~event_bit;
    }

    return VAL_TRUE;
}

static void device_read_map_all(cupkee_device_t *device, env_t *env, val_t *result)
{
    uint32_t data;

    if (device->driver->io.map.get(device->inst, -1, &data) > 0) {
        // support combine read
        val_set_number(result, data);
    } else {
        int i, size;
        array_t *a;

        size = device->driver->io.map.size(device->inst);
        a = _array_create(env, size);
        if (!a) {
            env_set_error(env, -CUPKEE_ERESOURCE);
            return;
        }

        for (i = 0; i < size; i++) {
            if (device->driver->io.map.get(device->inst, i, &data) > 0) {
                val_set_number(_array_elem(a, i), data);
            } else {
                val_set_undefined(_array_elem(a, i));
            }
        }

        val_set_array(result, (intptr_t) a);
    }
}

static void device_read_map_elem(cupkee_device_t *device, env_t *env, int offset, val_t *res)
{
    uint32_t data;

    if (offset >= 0) {
        if (device->driver->io.map.get(device->inst, offset, &data) > 0) {
            val_set_number(res, data);
        } else {
            *res = VAL_UNDEFINED;
        }
    } else {
        device_read_map_all(device, env, res);
    }
}

static val_t device_read_map(cupkee_device_t *device, env_t *env, int ac, val_t *av)
{
    val_t result;
    int offset;

    if (ac && val_is_number(av)) {
        offset = val_2_integer(av);
    } else {
        offset = -1;
    }

    device_read_map_elem(device, env, offset, &result);

    return result;
}

static int device_read_stream_n(cupkee_device_t *device, env_t *env, int n, val_t *res)
{
    type_buffer_t *buffer;
    int err;

    buffer = buffer_create(env, n);
    if (!buffer) {
        err = -CUPKEE_ERESOURCE;
    } else {
        err = device->driver->io.stream.recv(device->inst, n, buffer->buf);
    }

    if (err >= 0) {
        val_set_buffer(res, buffer);
    }

    return err;
}

static val_t device_read_stream(cupkee_device_t *device, env_t *env, int ac, val_t *av)
{
    int n, max, err;
    val_t args[2];

    max = device->driver->io.stream.received(device->inst);
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

    err = device_read_stream_n(device, env, n, &args[1]);
    if (ac && val_is_function(av)) {
        if (err < 0) {
            cupkee_do_callback_error(env, av, err);
        } else {
            val_set_undefined(args);
            cupkee_do_callback(env, av, 2, args);
        }
    }

    return err < 0 ? VAL_FALSE : VAL_TRUE;
}

static val_t device_read_block(cupkee_device_t *device, env_t *env, int ac, val_t *av)
{
    (void) device;
    (void) env;
    (void) ac;
    (void) av;
    return VAL_UNDEFINED;
}

static val_t device_write_map(cupkee_device_t *device, env_t *env, int ac, val_t *av)
{
    int offset, err = 0;
    uint32_t data;

    (void) env;

    if (ac < 1) {
        // Do nothing
        return VAL_TRUE;
    }

    if (ac == 1) {
        if (val_is_number(av)) {
            data = val_2_integer(av);
            offset = -1;
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
        err = device->driver->io.map.set(device->inst, offset, data);
    }
    return err > 0 ? VAL_TRUE : VAL_FALSE;
}

static val_t device_write_stream(cupkee_device_t *device, env_t *env, int ac, val_t *av)
{
    void *addr;
    int   size, offset = 0, send = 0, err = 0;
    val_t *data;

    (void) env;

    if (ac < 1 || !device_convert_data(av, &addr, &size)) {
        err = -CUPKEE_EINVAL;
    } else {
        data = av; ac--; av++;
        // [offset, n]
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

    if (err && ac) {
        cupkee_do_callback_error(env, av, err);
        return VAL_FALSE;
    }

    if (offset < size) {
        if (offset + send > size) {
            send = size - offset;
        }

        send = device->driver->io.stream.send(device->inst, send, addr + offset);
    } else {
        send = 0;
    }

    if (ac) {
        val_t args[3];

        val_set_undefined(args);
        val_set_number(args + 2, send);
        args[1] = *data;
        cupkee_do_callback(env, av, 3, args);
    }

    return VAL_TRUE;
}

static val_t device_write_block(cupkee_device_t *device, env_t *env, int ac, val_t *av)
{
    (void) device;
    (void) env;
    (void) ac;
    (void) av;
    return VAL_UNDEFINED;
}

static val_t device_native_read(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;

    if (ac && (device = device_val_control(av))) {
        av++; ac--;
    } else {
        return VAL_UNDEFINED;
    }

    if (device_is_enabled(device)) {
        switch (device->desc->category) {
        case DEVICE_CATEGORY_MAP:    return device_read_map   (device, env, ac, av);
        case DEVICE_CATEGORY_STREAM: return device_read_stream(device, env, ac, av);
        case DEVICE_CATEGORY_BLOCK:  return device_read_block (device, env, ac, av);
        default: return VAL_UNDEFINED;
        }
    } else {
        switch (device->desc->category) {
        case DEVICE_CATEGORY_STREAM:
        case DEVICE_CATEGORY_BLOCK: {
            int i;

            for (i = 0; i < ac; i++) {
                if (val_is_function(av + i)) {
                    cupkee_do_callback_error(env, av + i, -CUPKEE_EENABLED);
                }
            }

            return VAL_FALSE;
            }
        default: return VAL_UNDEFINED;
        }
    }
}

static val_t device_native_write(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device;

    if (ac && (device = device_val_control(av)) != NULL) {
        av++; ac--;
    } else {
        return VAL_FALSE;
    }

    if (device_is_enabled(device)) {
        switch (device->desc->category) {
        case DEVICE_CATEGORY_MAP:    return device_write_map(device, env, ac, av);
        case DEVICE_CATEGORY_STREAM: return device_write_stream(device, env, ac, av);
        case DEVICE_CATEGORY_BLOCK:  return device_write_block(device, env, ac, av);
        default: return VAL_UNDEFINED;
        }
    } else {
        int i;

        for (i = 0; i < ac; i++) {
            if (val_is_function(av + i)) {
                cupkee_do_callback_error(env, av + i, -CUPKEE_EENABLED);
            }
        }
        return VAL_FALSE;
    }
}

static void device_element(cupkee_device_t *device, env_t *env, int index, val_t *elem)
{
    if (device->desc->category == DEVICE_CATEGORY_MAP) {
        device_read_map_elem(device, env, index, elem);
    } else {
        val_set_undefined(elem);
    }
}

static void device_op_prop(void *env, intptr_t devid, val_t *name, val_t *prop)
{
    cupkee_device_t *device = device_id_control(devid);
    const char *prop_name = val_2_cstring(name);

    (void) env;

    if (device && prop_name) {
        if (!strcmp(prop_name, "read")) {
            val_set_native(prop, (intptr_t)device_native_read);
            return;
        } else
        if (!strcmp(prop_name, "write")) {
            val_set_native(prop, (intptr_t)device_native_write);
            return;
        } else
        if (!strcmp(prop_name, "received")) {
            if (device->desc->category == DEVICE_CATEGORY_STREAM) {
                val_set_number(prop, device->driver->io.stream.received(device->inst));
            } else {
                val_set_undefined(prop);
            }
            return;
        } else
        if (!strcmp(prop_name, "config")) {
            val_set_native(prop, (intptr_t)device_native_config);
            return;
        } else
        if (!strcmp(prop_name, "enable")) {
            val_set_native(prop, (intptr_t)device_native_enable);
            return;
        } else
        if (!strcmp(prop_name, "disable")) {
            val_set_native(prop, (intptr_t)device_native_disable);
            return;
        } else
        if (!strcmp(prop_name, "listen")) {
            val_set_native(prop, (intptr_t)device_native_listen);
            return;
        } else
        if (!strcmp(prop_name, "ignore")) {
            val_set_native(prop, (intptr_t)device_native_ignore);
            return;
        } else
        if (!strcmp(prop_name, "isEnabled")) {
            val_set_native(prop, (intptr_t)device_native_is_enabled);
            return;
        } else
        if (!strcmp(prop_name, "error")) {
            val_set_number(prop, device->error);
            return;
        } else
        if (!strcmp(prop_name, "instance")) {
            val_set_number(prop, device->inst);
            return;
        } else
        if (!strcmp(prop_name, "type")) {
            val_set_foreign_string(prop, (intptr_t) device->desc->name);
            return;
        } else
        if (!strcmp(prop_name, "category")) {
            val_set_foreign_string(prop, (intptr_t) device_categorys[device->desc->category]);
            return;
        } else
        if (!strcmp(prop_name, "destroy")) {
            val_set_native(prop, (intptr_t)device_native_destroy);
            return;
        }
    }

    val_set_undefined(prop);
}

static void device_op_elem(void *env, intptr_t devid, val_t *which, val_t *elem)
{
    if (val_is_number(which)) {
        cupkee_device_t *device = device_id_control(devid);

        device_element(device, env, val_2_integer(which), elem);
    } else {
        device_op_prop(env, devid, which, elem);
    }
}

static void device_data_proc_map(cupkee_device_t *device, env_t *env)
{
    val_t data;

    device_read_map_all(device, env, &data);

    cupkee_do_callback(env, device->event_handle[DEVICE_EVENT_DATA], 1, &data);
}

static void device_data_proc_stream(cupkee_device_t *device, env_t *env)
{
    val_t data;
    int err, n;

    n = device->driver->io.stream.received(device->inst);
    if (n > 0) {
        err = device_read_stream_n(device, env, n, &data);
        if (err < 0) {
            cupkee_do_callback_error(env, device->event_handle[DEVICE_EVENT_ERR], err);
        } else {
            cupkee_do_callback(env, device->event_handle[DEVICE_EVENT_DATA], 1, &data);
        }
    }
}

static void device_data_proc_block(cupkee_device_t *device, env_t *env)
{
    (void) device;
    (void) env;
}

static inline void device_do_error(cupkee_device_t *device, env_t *env)
{
    if (device->error) {
        cupkee_do_callback_error(env, device->event_handle[DEVICE_EVENT_ERR], device->error);
    }
}

static inline void device_do_data(cupkee_device_t *device, env_t *env)
{
    switch(device->desc->category) {
    case DEVICE_CATEGORY_MAP:     device_data_proc_map   (device, env); break;
    case DEVICE_CATEGORY_STREAM:  device_data_proc_stream(device, env); break;
    case DEVICE_CATEGORY_BLOCK:   device_data_proc_block (device, env); break;
    default:                break;
    }
}

static inline void device_do_drain(cupkee_device_t *device, env_t *env)
{
    cupkee_do_callback(env, device->event_handle[DEVICE_EVENT_DRAIN], 0, NULL);
}

static inline void device_do_ready(cupkee_device_t *device, env_t *env)
{
    (void) device;
    (void) env;
}

void device_error_post(uint8_t dev_id, int16_t code)
{
    cupkee_device_t *device;

    if (dev_id >= DEVICE_MAX) {
        return;
    }

    device = &device_heap[dev_id];
    if (!device_is_enabled(device)) {
        return;
    }

    device->error = -code;

    if (device->event & (1 << DEVICE_EVENT_ERR)) {
        devices_event_post(dev_id, DEVICE_EVENT_ERR);
    }
}

void device_data_post(uint8_t dev_id)
{
    cupkee_device_t *device;

    if (dev_id >= DEVICE_MAX) {
        return;
    }

    device = &device_heap[dev_id];
    if (!device_is_enabled(device)) {
        return;
    }

    if (device->event & (1 << DEVICE_EVENT_DATA)) {
        devices_event_post(dev_id, DEVICE_EVENT_DATA);
    }
}

void device_drain_post(uint8_t dev_id)
{
    cupkee_device_t *device;

    if (dev_id >= DEVICE_MAX) {
        return;
    }

    device = &device_heap[dev_id];
    if (!device_is_enabled(device)) {
        return;
    }

    if (device->event & (1 << DEVICE_EVENT_DRAIN)) {
        devices_event_post(dev_id, DEVICE_EVENT_DRAIN);
    }
}

void device_poll(void)
{
    cupkee_device_t *device = device_work;

    while (device) {
        if (device->driver->poll)
            device->driver->poll(device->inst);

        device = device->next;
    }
}

void device_sync(uint32_t systicks)
{
    cupkee_device_t *device = device_work;

    while (device) {
        if (device->driver->sync)
            device->driver->sync(device->inst, systicks);

        device = device->next;
    }
}

typedef void (*device_event_handle_t)(cupkee_device_t *, env_t *);
static const device_event_handle_t device_event_handles[] = {
    device_do_error,
    device_do_data,
    device_do_drain,
    device_do_ready
};

void device_event_proc(env_t *env, int event)
{
    cupkee_device_t *device;
    uint8_t devid;

    devid = EVENT_PARAM1(event);
    event = EVENT_PARAM2(event);

    if (event >= DEVICE_EVENT_MAX || devid >= DEVICE_MAX) {
        return;
    }

    device = &device_heap[devid];
    if (!device_is_enabled(device)) {
        return;
    }

    device_event_handles[event](device, env);
}

void device_init(void)
{
    int i;

    device_free = NULL;
    device_work = NULL;

    for (i = 0; i < DEVICE_MAX; i++) {
        cupkee_device_t *device = &device_heap[i];

        device->magic = 0;
        device->flags = 0;
        device->event = 0;
        device->next = device_free;

        device_free = device;
    }

    return;
}

val_t device_native_create(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device = NULL;
    const char *name;
    int instance;

    if (ac == 0) {
        device_show();
        return VAL_UNDEFINED;
    }

    name = val_2_cstring(av);
    if (!name) {
        return VAL_UNDEFINED;
    }

    if (ac > 1 && val_is_number(av + 1)) {
        instance = val_2_integer(av + 1);
    } else {
        instance = 0;
    }

    device = device_create(name, instance);
    if (device) {
        return val_create(env, &device_op, device_id_gen(device));
    } else {
        return VAL_UNDEFINED;
    }
}

val_t device_native_pin_map(env_t *env, int ac, val_t *av)
{
    int id, port, pin;
    (void) env;

    if (ac < 3 || !val_is_number(av) || !val_is_number(av + 1) || !val_is_number(av + 2)) {
        return VAL_FALSE;
    }

    id   = val_2_integer(av);
    port = val_2_integer(av + 1);
    pin  = val_2_integer(av + 2);

    if (CUPKEE_OK == hw_pin_map(id, port, pin)) {
        return VAL_TRUE;
    } else {
        return VAL_FALSE;
    }
}

val_t device_native_led_map(env_t *env, int ac, val_t *av)
{
    int port, pin;

    (void) env;

    if (ac < 2 || !val_is_number(av) || !val_is_number(av + 1)) {
        return VAL_FALSE;
    }

    port = val_2_integer(av);
    pin  = val_2_integer(av + 1);

    if (CUPKEE_OK == hw_led_map(port, pin)) {
        return VAL_TRUE;
    } else {
        return VAL_FALSE;
    }
}

val_t device_native_led(env_t *env, int ac, val_t *av)
{
    (void) env;

    if (ac > 0) {
        if (val_is_true(av)) {
            hw_led_set();
        } else {
            hw_led_clear();
        }
    } else {
        hw_led_toggle();
    }
    return VAL_UNDEFINED;
}

