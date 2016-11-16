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

void systick_event_post(void);
void devices_event_post(int dev, int which, int event);
void led_on(void);
void led_off(void);
void led_toggle(void);

typedef struct event_info_t {
    int dev;
    int which;
    int event;
} event_info_t;

static uint32_t systicks = 0;
static int led, led_data = 0;
static hw_gpio_conf_t led_conf;
static event_info_t event_queue[4];
static int event_len = 0;

static char command_buf[80];
static int command_len = 0;
static int command_done = 1;

static int usart;
static int adc;

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

void led_on(void)
{
    led_data = 1;
    hw_gpio_write(led, -1, led_data);
}

void led_off(void)
{
    led_data = 0;
    hw_gpio_write(led, -1, led_data);
}

void led_toggle(void)
{
    hw_gpio_write(led, 0, (++led_data) & 1);
}

static void nosys_setup(void)
{

    hw_setup();

    hw_gpio_conf_reset(&led_conf);
    led_conf.pin_num = 1;
    led_conf.pin_seq[0] = 0x2d;
    led_conf.mod = OPT_GPIO_MOD_OUTPUT_PUSHPULL;
    led = hw_gpio_group_alloc();
    hw_gpio_enable(led, &led_conf);

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

        if (e->dev == ADC_DEVICE_ID) {
            uint32_t v;
            hw_adc_read(adc, 0, &v);
            snprintf(buf, 64, "adc: [%lx]\r\n", v);
            hw_console_sync_puts(buf);
        }
    }
    event_len = 0;
}

static void command_adc_start(void)
{
    hw_adc_conf_t adc_conf;

    adc = hw_adc_alloc();
    adc_conf.chn_num = 1;
    adc_conf.chn_seq[0] = 8;
    adc_conf.interval = 50;
    hw_adc_enable(adc, &adc_conf);
    hw_adc_event_enable(adc, ADC_EVENT_READY);
    hw_adc_event_enable(adc, ADC_EVENT_DATA);
}

static void command_usart_start(void)
{
    hw_usart_conf_t usart_conf;

    usart = hw_usart_alloc();
    usart_conf.baudrate = 9600;
    usart_conf.databits = 8;
    usart_conf.stopbits = 1;
    usart_conf.parity = 0;
    hw_usart_enable(usart, &usart_conf);
    hw_usart_event_enable(usart, USART_EVENT_DATA);
    hw_usart_event_enable(usart, USART_EVENT_DRAIN);
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
        led_toggle();
    } else
    if (!strcmp("adcs\r", command_buf)) {
        command_adc_start();
    } else
    if (!strcmp("adcv\r", command_buf)) {
        uint32_t v;
        char buf[64];

        hw_adc_read(adc, 0, &v);
        snprintf(buf, 64, "ADC0: %u[%x]\r\n", v, v);
        hw_console_sync_puts(buf);
    } else
    if (!strcmp("usart\r", command_buf)) {
        command_usart_start();
    } else
    if (!strcmp("send\r", command_buf)) {
        hw_usart_send(usart, 5, (uint8_t *)"hello");
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

        command_proc();
        event_proc();

        hw_poll();
    }
}

