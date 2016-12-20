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

#define HW_FL_USED      1

typedef struct hw_uart_t {
    uint8_t flags;
    uint8_t dev_id;
    const hw_config_uart_t *config;
} hw_uart_t;

static hw_uart_t uart_controls[HW_INSTANCES_UART];

/****************************************************************************
 * Debug start                                                             */
static int dbg_setup_status[HW_INSTANCES_UART];

void hw_dbg_uart_setup_status_set(int instance, int status)
{
    dbg_setup_status[instance] = status;
}


/* Debug end                                                               *
 ***************************************************************************/
static inline hw_uart_t *uart_get(int instance) {
    return &uart_controls[instance];
}

static void uart_release(int instance)
{
    hw_uart_t *control = uart_get(instance);

    /* Do hardware release here */

    control->flags = 0;
}

static void uart_reset(int instance)
{
    hw_uart_t *control = uart_get(instance);

    /* Do hardware reset here */

    control->dev_id = DEVICE_ID_INVALID;
    control->config = NULL;
}

static int uart_setup(int instance, uint8_t dev_id, const hw_config_t *config)
{
    hw_uart_t *control = uart_get(instance);
    int err;

    /* Do hardware setup here */
    err = -dbg_setup_status[instance];

    if (!err) {
        control->dev_id = dev_id;
        control->config = (const hw_config_uart_t *)config;
    }
    return err;
}

static void uart_poll(int instance)
{
    hw_uart_t *control = uart_get(instance);

    if (0) {
        device_data_post(control->dev_id);
    }
}

static int uart_recv(int instance, int size, void *buf)
{
    return 0;
}

static int uart_send(int instance, int len, void *data)
{
    return 0;
}

static int uart_received(int instance)
{
    return 0;
}

static const hw_driver_t uart_driver = {
    .release = uart_release,
    .reset   = uart_reset,
    .setup   = uart_setup,
    .poll    = uart_poll,
    .io.stream = {
        .recv = uart_recv,
        .send = uart_send,
        .received = uart_received,
    }
};

const hw_driver_t *hw_request_uart(int instance)
{
    if (instance >= HW_INSTANCES_UART || uart_controls[instance].flags) {
        return NULL;
    }

    uart_controls[instance].flags  = HW_FL_USED;
    uart_controls[instance].dev_id = DEVICE_ID_INVALID;
    uart_controls[instance].config = NULL;

    return &uart_driver;
}

void hw_setup_uart(void)
{
    int i;

    for (i = 0; i < HW_INSTANCES_UART; i++) {
        uart_controls[i].flags = 0;
    }
}

