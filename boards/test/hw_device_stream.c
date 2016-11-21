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
#define DEVICE_NAME          "stream"
#define DEVICE_ID            1
#define EVENT_NUM            3
#define CONFIG_NUM           3
#define INSTANCE_NUM         1

#define CONF_BAUDRATE        0
#define CONF_STOPBITS        1
#define CONF_PARITY          2

#define BUF_SEND             0
#define BUF_RECV             1
#define BUF_SIZE             32

typedef struct hw_buf_t{
    uint16_t bgn;
    uint16_t len;
    uint8_t buf[BUF_SIZE];
} hw_buf_t;

/*******************************************************************************
 * dbg field
*******************************************************************************/
static void device_set_error(int id, int inst, int error);

static void *dbg_input_data = NULL;
static int  dbg_input_len = 0;
static int  dbg_input_pos = 0;
static int  dbg_send_allow = 0;

void hw_dbg_stream_set_input(const char *data)
{
    dbg_input_len = strlen(data);
    dbg_input_pos = 0;
    dbg_input_data = (void *)data;
}

void hw_dbg_stream_set_send(int n)
{
    dbg_send_allow = n;
}

void hw_dbg_stream_set_error(int inst, int err)
{
    device_set_error(0, inst, err);
}

/*******************************************************************************
 * dbg field end
*******************************************************************************/
static const char *device_config_names[] = {
    "baudrate", "stopbits", "parity"
};
static const char *device_opt_names[] = {
    "1", "2", "none", "odd", "even"
};
static const hw_config_desc_t device_config_descs[] = {
    {
        .type = HW_CONFIG_NUM,
    },
    {
        .type = HW_CONFIG_OPT,
        .opt_num = 2,
        .opt_start = 0,
    },
    {
        .type = HW_CONFIG_OPT,
        .opt_num = 3,
        .opt_start = 2,
    }
};

static uint8_t device_used = 0;
static uint8_t device_work = 0;
static int device_error[INSTANCE_NUM];
static int device_config_settings[INSTANCE_NUM][CONFIG_NUM];
static int device_event_settings[INSTANCE_NUM];
static hw_buf_t device_buf[INSTANCE_NUM][2];

static inline void buf_init(hw_buf_t *b) {
    b->len = 0;
    b->bgn = 0;
}

static inline void buf_push(hw_buf_t *b, uint8_t d) {
    if (b->len < BUF_SIZE) {
        int tail = b->bgn + b->len++;
        if (tail >= BUF_SIZE) {
            tail -= BUF_SIZE;
        }
        b->buf[tail] = d;
    }
}

static inline uint8_t buf_shift(hw_buf_t *b) {
    uint8_t d = b->buf[b->bgn++];
    if (b->bgn >= BUF_SIZE) {
        b->bgn = 0;
    }
    b->len--;
    return d;
}

static inline int buf_gets(hw_buf_t *b, int n, void *buf) {
    if (n > b->len) {
        n = b->len;
    }

    if (n) {
        int tail = b->bgn + b->len;
        if (tail < BUF_SIZE) {
            memcpy(buf, b->buf + b->bgn, n);
            b->bgn = tail;
        } else {
            int wrap = tail - BUF_SIZE;
            int size = n;
            if (wrap) {
                size -= wrap;
                memcpy(buf + size, b->buf, wrap);
            }
            memcpy(buf, b->buf + b->bgn, size);
            b->bgn = wrap;
        }
        b->len -= n;
    }

    return n;
}

static inline int buf_puts(hw_buf_t *b, int n, void *buf) {
    if (n + b->len > BUF_SIZE) {
        n = BUF_SIZE - b->len;
    }

    if (n) {
        int tail = b->bgn + b->len;
        int wrap = 0;

        b->len += n;
        if (tail >= BUF_SIZE) {
            tail -= BUF_SIZE;
        } else
        if (tail + n >= BUF_SIZE) {
            wrap = tail + n - BUF_SIZE;
            n -= wrap;

            memcpy(b->buf, buf + n, wrap);
        }
        memcpy(b->buf + tail, buf, n);

        return n + wrap;
    }
    return 0;
}

static inline int device_is_inused(int inst) {
    if (inst < INSTANCE_NUM) {
        return device_used & (1 << inst);
    } else {
        return 0;
    }
}

static inline int device_is_work(int inst) {
    if (inst < INSTANCE_NUM) {
        return (device_used & device_work) & (1 << inst);
    } else {
        return 0;
    }
}

static inline int device_has_data(int inst) {
    (void) inst;
    return dbg_input_len - dbg_input_pos;
}

static inline int device_not_busy(int inst) {
    (void) inst;
    return dbg_send_allow-- > 0;
}

static inline uint8_t device_in(int inst) {
    (void) inst;
    if (dbg_input_len > dbg_input_pos) {
        uint8_t *buf = (uint8_t *)dbg_input_data;
        return buf[dbg_input_pos++];
    }
    return 0;
}

static inline void device_out(int inst, uint8_t d)
{
    (void) inst;
    (void) d;
}

static void device_check_recv(int i)
{
    hw_buf_t *b = &device_buf[i][BUF_RECV];
    int recv = 0;

    while(device_has_data(i)) {
        buf_push(b, device_in(i));
        recv++;
    }

    if (recv && (device_event_settings[i] & (1 << DEVICE_EVENT_DATA))) {
        devices_event_post(DEVICE_ID, i, DEVICE_EVENT_DATA);
    }
}

static void device_check_send(int i)
{
    hw_buf_t *b = &device_buf[i][BUF_SEND];
    int send = 0;

    while (device_not_busy(i)) {
        if (b->len <= 0) {
            if (send && (device_event_settings[i] & (1 << DEVICE_EVENT_DRAIN))) {
                devices_event_post(DEVICE_ID, i, DEVICE_EVENT_DRAIN);
            }
            return;
        }

        device_out(i, buf_shift(b));
        send++;
    }
}

static void device_set_error(int id, int inst, int error)
{
    (void) id;
    device_error[inst] = error;

    if (device_event_settings[inst] & 1) {
        devices_event_post(DEVICE_ID, inst, 0);
    }
}

static int device_get_error(int id, int inst)
{
    (void) id;
    if (inst >= INSTANCE_NUM) {
        return 0;
    }

    return device_error[inst];
}

static int device_setup(int inst)
{
    (void) inst;
    return 1;
}

static int device_reset(int inst)
{
    (void) inst;
    return 1;
}

static int device_enable(int id, int inst)
{
    (void) id;
    if (device_is_inused(inst)) {
        uint8_t b = 1 << inst;

        if (!(device_work & b)) {
            buf_init(&device_buf[inst][BUF_RECV]);
            buf_init(&device_buf[inst][BUF_SEND]);

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
    if (inst < INSTANCE_NUM) {
        int used = device_used & (1 << inst);

        if (!used) {
            int c;

            device_error[inst] = 0;
            device_event_settings[inst] = 0;
            device_used |= 1 << inst;
            device_work &= ~(1 << inst);
            for (c = 0; c < CONFIG_NUM; c++) {
                device_config_settings[inst][c] = 0;
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
    if (device_is_inused(inst) && which < CONFIG_NUM) {
        device_config_settings[inst][which] = setting;
        return 1;
    }
    return 0;
}

static int device_config_get(int id, int inst, int which, int *setting)
{
    (void) id;
    if (device_is_inused(inst) && which < CONFIG_NUM && setting) {
        *setting = device_config_settings[inst][which];
        return 1;
    }
    return 0;
}

static void device_listen(int id, int inst, int event)
{
    (void) id;
    if (device_is_inused(inst) && event < EVENT_NUM) {
        device_event_settings[inst] |= 1 << event;
    }
}

static void device_ignore(int id, int inst, int event)
{
    (void) id;
    if (device_is_inused(inst) && event < EVENT_NUM) {
        device_event_settings[inst] &= ~(1 << event);
    }
}

static int device_recv(int id, int inst, int n, void *buf)
{
    (void) id;
    if (device_is_work(inst)) {
        hw_buf_t *b = &device_buf[inst][BUF_RECV];

        return buf_gets(b, n, buf);
    }
    return 0;
}

static int device_send(int id, int inst, int n, void *data)
{
    (void) id;
    if (device_is_work(inst)) {
        hw_buf_t *b = &device_buf[inst][BUF_SEND];
        return buf_puts(b, n, data);
    }
    return 0;
}

static int device_received(int id, int inst)
{
    (void) id;
    if (device_is_work(inst)) {
        hw_buf_t *b = &device_buf[inst][BUF_RECV];
        return b->len;
    }
    return 0;
}

void hw_device_stream_setup(void)
{
    device_used = 0;
    device_work = 0;
}

void hw_device_stream_poll(void)
{
    int i;

    for (i = 0; i < INSTANCE_NUM; i++) {
        if (device_is_work(i)) {
            device_check_recv(i);
            device_check_send(i);
        }
    }
}

const hw_driver_t hw_driver_stream = {
    .request = device_request,
    .release = device_release,
    .get_err = device_get_error,
    .enable  = device_enable,
    .disable = device_disable,
    .config_set = device_config_set,
    .config_get = device_config_get,
    .listen = device_listen,
    .ignore = device_ignore,

    .io.stream.send = device_send,
    .io.stream.recv = device_recv,
    .io.stream.received = device_received,
};

const hw_device_t hw_device_stream = {
    .name = DEVICE_NAME,
    .id   = DEVICE_ID,
    .type = HW_DEVICE_STREAM,
    .inst_num   = INSTANCE_NUM,
    .conf_num   = CONFIG_NUM,
    .event_num  = EVENT_NUM,
    .conf_names = device_config_names,
    .conf_descs = device_config_descs,
    .opt_names  = device_opt_names,
};

