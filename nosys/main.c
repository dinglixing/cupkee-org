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
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <bsp.h>
#include "key.h"
#include "timer.h"

void systick_event_post(void);
void devices_event_post(int dev, int which, int event);

typedef struct event_info_t {
    int dev;
    int which;
    int event;
} event_info_t;

static uint32_t systicks = 0;
static event_info_t event_queue[4];
static int event_len = 0;

static char command_buf[80];
static int command_len = 0;
static int command_done = 1;

void systick_event_post(void)
{
    systicks++;
}

void devices_event_post(int dev, int which, int event)
{
    (void) dev;
    (void) which;
    (void) event;

    if (event_len < 4) {
        event_info_t *e = event_queue + event_len++;

        e->dev = dev;
        e->which = which;
        e->event = event;
    }

    return;
}

static void console_input_handle(void *data, int n)
{
    int pos = 0;
    char *s = data;

    while (pos < n) {
        char ch = s[pos++];
        if (command_done && command_len < 80) {
            command_buf[command_len++] = ch;
        }

        hw_console_sync_putc(ch);

        if (ch == '\r') {
            hw_console_sync_putc('\n');
            command_buf[command_len] = 0;
            command_done = 0;
            command_len = 0;
        }
    }
}

static void console_drain_handle(void)
{
}

static void nosys_setup(void)
{

    hw_setup();

    hw_console_set_callback(console_input_handle, console_drain_handle);
}

static void event_proc(void)
{
    int i;
    for (i = 0; i < event_len; i++) {
        char buf[64];
        event_info_t *e = event_queue + i;

        snprintf(buf, 64, "from: [%d:%d] event %d\r\n", e->dev, e->which, e->event);
        hw_console_sync_puts(buf);
    }
    event_len = 0;
}

static void command_proc(void)
{
    if (command_done) {
        return;
    }

    if (strcmp("hello\r", command_buf) == 0) {
        hw_console_sync_puts("hi\r\n");
    } else
    if (!strcmp("led\r", command_buf)) {
        hw_led_toggle();
    } else
    if (!strcmp("pulse\r", command_buf)) {
        if (pulse_enable()) {
            hw_console_sync_puts("ok\r\n");
        } else {
            hw_console_sync_puts("fail\r\n");
        }
    } else
    if (!strcmp("trigger\r", command_buf)) {
        if (pulse_trigger()) {
            hw_console_sync_puts("ok\r\n");
        } else {
            hw_console_sync_puts("fail\r\n");
        }
    } else
    if (!strcmp("pwm\r", command_buf)) {
        if (pwm_enable()) {
            hw_console_sync_puts("ok\r\n");
        } else {
            hw_console_sync_puts("fail\r\n");
        }
    } else
    if (!strcmp("pwm10\r", command_buf)) {
        if (pwm_set(10)) {
            hw_console_sync_puts("ok\r\n");
        } else {
            hw_console_sync_puts("fail\r\n");
        }
    } else
    if (!strcmp("pwm50\r", command_buf)) {
        if (pwm_set(50)) {
            hw_console_sync_puts("ok\r\n");
        } else {
            hw_console_sync_puts("fail\r\n");
        }
    } else
    if (!strcmp("pwm80\r", command_buf)) {
        if (pwm_set(80)) {
            hw_console_sync_puts("ok\r\n");
        } else {
            hw_console_sync_puts("fail\r\n");
        }
    } else
    if (!strcmp("pwm100\r", command_buf)) {
        if (pwm_set(100)) {
            hw_console_sync_puts("ok\r\n");
        } else {
            hw_console_sync_puts("fail\r\n");
        }
    } else
    if (!strcmp("usart_send\r", command_buf)) {
    }

    command_done = 1;
}

int main(void)
{
    uint32_t tick = 0;

    nosys_setup();

    while(1) {
        if (tick != systicks) {
            tick = systicks;
            if (tick % 1000 == 0) {
            }
        }

        event_proc();

        command_proc();

        hw_poll();
    }
}

