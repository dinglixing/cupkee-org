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

static const uint32_t device_base[] = {USART1, USART2, USART3, UART4, UART5};
static const uint32_t device_rcc[] = {RCC_USART1, RCC_USART2, RCC_USART3, RCC_UART4, RCC_UART5};

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
static int device_error[HW_UART_INSTANCE_NUM];
static uint32_t device_check_times[HW_UART_INSTANCE_NUM];
static int device_config_settings[HW_UART_INSTANCE_NUM][HW_UART_CONFIG_NUM];
static int device_event_settings[HW_UART_INSTANCE_NUM];
static hw_buf_t device_buf[HW_UART_INSTANCE_NUM][2];

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
    if (inst < HW_UART_INSTANCE_NUM) {
        return device_used & (1 << inst);
    } else {
        return 0;
    }
}

static inline int device_is_work(int inst) {
    if (inst < HW_UART_INSTANCE_NUM) {
        return (device_used & device_work) & (1 << inst);
    } else {
        return 0;
    }
}

static inline int device_has_data(int inst) {
    return USART_SR(device_base[inst]) & USART_SR_RXNE;
}

static inline int device_not_busy(int inst) {
    return USART_SR(device_base[inst]) & USART_SR_TXE;
}

static inline uint8_t device_in(int inst) {
    return USART_DR(device_base[inst]);
}

static inline void device_out(int inst, uint8_t d)
{
    USART_DR(device_base[inst]) = d;
}

static void device_check_recv(int i)
{
    hw_buf_t *b = &device_buf[i][BUF_RECV];

    if (device_has_data(i)) {
        buf_push(b, device_in(i));
        if (b->len > (BUF_SIZE * 2 / 3)) {
            devices_event_post(HW_UART_DEVICE_ID, i, DEVICE_EVENT_DATA);
        }
        device_check_times[i] = 0;
    } else {
        if (b->len) {
            uint32_t times = device_check_times[i]++;
            if (times > 500){
                devices_event_post(HW_UART_DEVICE_ID, i, DEVICE_EVENT_DATA);
                device_check_times[i] = 0;
            }
        }
    }
}

static void device_check_send(int i)
{
    hw_buf_t *b = &device_buf[i][BUF_SEND];

    if (b->len > 0) {
        if (device_not_busy(i)) {
            device_out(i, buf_shift(b));
            if (b->len == 0 && (device_event_settings[i] & (1 << DEVICE_EVENT_DRAIN))) {
                devices_event_post(HW_UART_DEVICE_ID, i, DEVICE_EVENT_DRAIN);
            }
        }
    }
}

static void device_set_error(int inst, int error)
{
    device_error[inst] = error;

    if (device_event_settings[inst] & 1) {
        devices_event_post(HW_UART_DEVICE_ID, inst, 0);
    }
}

static int device_get_error(int id, int inst)
{
    (void) id;
    if (inst >= HW_UART_INSTANCE_NUM) {
        return 0;
    }

    return device_error[inst];
}

static int device_setup_uart_gpio(int inst)
{
    uint32_t bank_rx, bank_tx;
    uint16_t gpio_rx, gpio_tx;
    int port_rx, port_tx;

    switch(inst) {
    case 0:
        gpio_rx = GPIO_USART1_RX; gpio_tx = GPIO_USART1_TX;
        bank_rx = bank_tx = GPIOA;
        port_rx = port_tx = 0;
        break;
    case 1:
        gpio_rx = GPIO_USART2_RX; gpio_tx = GPIO_USART2_TX;
        bank_rx = bank_tx = GPIOA;
        port_rx = port_tx = 0;
        break;
    case 2:
        gpio_rx = GPIO_USART3_RX; gpio_tx = GPIO_USART3_TX;
        bank_rx = bank_tx = GPIOB;
        port_rx = port_tx = 1;
        break;
    case 3:
        gpio_rx = GPIO_UART4_RX; gpio_tx = GPIO_UART4_TX;
        bank_rx = bank_tx = GPIOC;
        port_rx = port_tx = 2;
        break;
    case 4:
        gpio_rx = GPIO_UART5_RX; gpio_tx = GPIO_UART5_TX;
        bank_rx = GPIO_BANK_UART5_RX; bank_tx = GPIO_BANK_UART5_TX;
        port_rx = 3; port_tx = 2;
        break;
    default: return -1;
    }

    if (port_rx == port_tx) {
        if (!hw_gpio_use(port_rx, gpio_rx | gpio_tx)) {
            return -1;
        }
    } else {
        if (!hw_gpio_use(port_rx, gpio_rx)) {
            return -1;
        }
        if (!hw_gpio_use(port_tx, gpio_tx)) {
            hw_gpio_release(port_rx, gpio_rx);
            return -1;
        }
    }

    gpio_set_mode(bank_tx, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, gpio_tx);
    gpio_set_mode(bank_rx, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, gpio_rx);

    return 0;
}

static void device_reset_uart_gpio(int inst)
{
    switch(inst) {
    case 0:
        hw_gpio_release(0, GPIO_USART1_TX | GPIO_USART1_RX);
        break;
    case 1:
        hw_gpio_release(0, GPIO_USART2_TX | GPIO_USART2_RX);
        break;
    case 2:
        hw_gpio_release(1, GPIO_USART3_TX | GPIO_USART3_RX);
        break;
    case 3:
        hw_gpio_release(2, GPIO_UART4_TX | GPIO_UART4_RX);
        break;
    case 4:
        hw_gpio_release(2, GPIO_UART5_TX);
        hw_gpio_release(3, GPIO_UART5_RX);
        break;
    default: break;
    }
}

static int device_setup(int inst)
{
    uint32_t stopbits, parity;

    switch(device_config_settings[inst][CONF_STOPBITS]) {
    case 0: stopbits = USART_STOPBITS_1; break;
    case 1: stopbits = USART_STOPBITS_2; break;
    case 2: stopbits = USART_STOPBITS_0_5; break;
    case 3: stopbits = USART_STOPBITS_1_5; break;
    default: stopbits = USART_STOPBITS_1; break;
    }

    switch(device_config_settings[inst][CONF_PARITY]) {
    case 0: parity = USART_PARITY_NONE; break;
    case 1: parity = USART_PARITY_ODD; break;
    case 2: parity = USART_PARITY_EVEN; break;
    default: parity = USART_PARITY_NONE; break;
    }

    if (device_setup_uart_gpio(inst)) {
        device_set_error(inst, 777);
        return 0;
    }
    rcc_periph_clock_enable(device_rcc[inst]);

	usart_set_baudrate(device_base[inst], device_config_settings[inst][CONF_BAUDRATE]);
	usart_set_databits(device_base[inst], 8);
	usart_set_stopbits(device_base[inst], stopbits);
	usart_set_parity  (device_base[inst], parity);
	usart_set_mode    (device_base[inst], USART_MODE_TX_RX);
	usart_set_flow_control(device_base[inst], USART_FLOWCONTROL_NONE);

    usart_enable(device_base[inst]);

    return 1;
}

static int device_reset(int inst)
{
    device_reset_uart_gpio(inst);

    usart_disable(device_base[inst]);

    rcc_periph_clock_disable(device_rcc[inst]);

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
            device_check_times[inst] = 0;

            if (device_setup(inst)) {
                device_work |= b;
                return 1;
            } else {
                return 0;
            }
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
    if (inst < HW_UART_INSTANCE_NUM) {
        int used = device_used & (1 << inst);

        if (!used) {
            int c;

            device_error[inst] = 0;
            device_event_settings[inst] = 0;
            device_used |= 1 << inst;
            device_work &= ~(1 << inst);
            for (c = 0; c < HW_UART_CONFIG_NUM; c++) {
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
    if (device_is_inused(inst) && which < HW_UART_CONFIG_NUM) {
        device_config_settings[inst][which] = setting;
        return 1;
    }
    return 0;
}

static int device_config_get(int id, int inst, int which, int *setting)
{
    (void) id;
    if (device_is_inused(inst) && which < HW_UART_CONFIG_NUM && setting) {
        *setting = device_config_settings[inst][which];
        return 1;
    }
    return 0;
}

static void device_listen(int id, int inst, int event)
{
    (void) id;
    if (device_is_inused(inst) && event < HW_UART_EVENT_NUM) {
        device_event_settings[inst] |= 1 << event;
    }
}

static void device_ignore(int id, int inst, int event)
{
    (void) id;
    if (device_is_inused(inst) && event < HW_UART_EVENT_NUM) {
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

int hw_setup_usart(void)
{
    device_used = 0;
    device_work = 0;
    return 0;
}

void hw_poll_usart(void)
{
    int i;

    for (i = 0; i < HW_UART_INSTANCE_NUM; i++) {
        if (device_is_work(i)) {
            device_check_recv(i);
            device_check_send(i);
        }
    }
}

const hw_driver_t hw_driver_uart = {
    .request = device_request,
    .release = device_release,
    .get_err = device_get_error,
    .enable  = device_enable,
    .disable = device_disable,
    .config_set = device_config_set,
    .config_get = device_config_get,
    .listen = device_listen,
    .ignore = device_ignore,
    .io.stream = {
        .send = device_send,
        .recv = device_recv,
        .received = device_received,
    }
};

const hw_device_t hw_device_uart= {
    .name = HW_UART_DEVICE_NAME,
    .id   = HW_UART_DEVICE_ID,
    .type = HW_DEVICE_STREAM,
    .inst_num   = HW_UART_INSTANCE_NUM,
    .conf_num   = HW_UART_CONFIG_NUM,
    .event_num  = HW_UART_EVENT_NUM,
    .conf_names = device_config_names,
    .conf_descs = device_config_descs,
    .opt_names  = device_opt_names,
};

