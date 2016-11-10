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
 * hw field
*******************************************************************************/
#include <bsp.h>
#include "hardware.h"

#if 0
#define _TRACE(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define _TRACE(fmt, ...)    //
#endif

#define USART_INUSED      0x80
#define USART_ENABLE      0x40

#define USART_STATE_MASK  0x30

#define USART_EVENT_MASK  0x0f
#define USART_BUF_SIZE  64

typedef struct hw_usart_t{
    uint8_t flags;

    uint8_t send_bgn;
    uint8_t send_len;
    uint8_t recv_bgn;
    uint8_t recv_len;
    uint8_t send_buf[USART_BUF_SIZE];
    uint8_t recv_buf[USART_BUF_SIZE];
} hw_usart_t;

static hw_usart_t usart_buffs[USART_INSTANCE_MAX];
static uint8_t    usart_flags[USART_INSTANCE_MAX];

static inline int hw_usart_verify(unsigned instance) {
    return (instance < USART_INSTANCE_MAX && (usart_flags[instance] & USART_INUSED));
}

static int hw_usart_config_set(int instance, hw_usart_conf_t *cfg)
{
    /* instance setup code here */
    /* ...                      */

    return 0;
}

static int hw_usart_config_clr(int instance)
{
    /* instance reset code here */
    /* ...                      */

    return 0;
}

int hw_usart_setup(void)
{
    int i;

    /* usart setup code here    */
    /* ...                      */

    for (i = 0; i < USART_INSTANCE_MAX; i++) {
        usart_flags[i] = 0;
        usart_buffs[i].recv_bgn = 0;
        usart_buffs[i].recv_len = 0;
        usart_buffs[i].send_bgn = 0;
        usart_buffs[i].send_len = 0;
    }

    return 0;
}

void hw_usart_poll(void)
{
}

int hw_usart_alloc(void)
{
    int i;

    for (i = 0; i < USART_INSTANCE_MAX; i++) {
        if (!(usart_flags[i] & USART_INUSED)) {
            usart_flags[i] = USART_INUSED;
            usart_buffs[i].recv_bgn = 0;
            usart_buffs[i].recv_len = 0;
            usart_buffs[i].send_bgn = 0;
            usart_buffs[i].send_len = 0;

            return i;
        }
    }

    return -1;
}

int hw_usart_release(int instance)
{
    if (hw_usart_verify(instance)) {
        /* instance release code here */
        /* ...                      */

        usart_flags[instance] = 0;
        return 0;
    }
    return -1;
}

void hw_usart_conf_reset(hw_usart_conf_t *conf)
{
    conf->baudrate = 9600;
    conf->databits = 8;
    conf->stopbits = 1;
    conf->parity   = OPT_USART_PARITY_NONE;
}

int hw_usart_enable(int instance, hw_usart_conf_t *cfg)
{
    if (!hw_usart_verify(instance)) {
        return -1;
    }

    if (!(usart_flags[instance] & USART_ENABLE)) {
        if (0 == hw_usart_config_set(instance, cfg)) {
            usart_flags[instance] |= USART_ENABLE;
            return 0;
        }
    }

    return -1;
}

int hw_usart_disable(int instance)
{
    if (!hw_usart_verify(instance)) {
        return -1;
    }

    if (usart_flags[instance] & USART_ENABLE) {
        if (0 == hw_usart_config_clr(instance)) {
            usart_flags[instance] &= ~USART_ENABLE;
            return 0;
        }
    }
    return -1;
}

int hw_usart_read(int instance, int size, uint8_t *buf)
{
    return -1;
}

int hw_usart_write(int instance, int size, uint8_t *buf)
{
    return -1;
}

int hw_usart_event_enable(int instance, int event)
{
    if (!hw_usart_verify(instance) || event >= USART_EVENT_MAX) {
        return -1;
    }

    usart_flags[instance] |= (1 << event);


    return 0;
}

int hw_usart_event_disable(int instance, int event)
{
    if (!hw_usart_verify(instance) || event >= USART_EVENT_MAX) {
        return -1;
    }

    usart_flags[instance] &= ~(1 << event);

    return 0;
}

