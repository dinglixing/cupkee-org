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

static uint32_t systicks = 0;
static int led, led_data = 0;

void systick_event_post(void)
{
    systicks++;
}

void devices_event_post(int dev, int which, int event)
{
    (void) dev;
    (void) which;
    (void) event;

    return;
}

static void console_input_handle(void *data, int n)
{
    int pos = 0;
    char *s = data;

    while (pos < n) {
        hw_console_sync_putc(s[pos++]);
    }
}

static void console_drain_handle(void)
{
}

void led_on(void)
{
    led_data = 1;
    hw_gpio_write(led, led_data);
}

void led_off(void)
{
    led_data = 0;
    hw_gpio_write(led, led_data);
}

void led_toggle(void)
{
    hw_gpio_write(led, ++led_data);
}

static void nosys_setup(void)
{
    hw_gpio_conf_t conf;

    hw_setup();

    hw_gpio_conf_reset(&conf);
    conf.pin_num = 1;
    conf.pin_seq[0] = 0x2d;
    conf.mod = OPT_GPIO_MOD_OUTPUT_PUSHPULL;
    led = hw_gpio_group_alloc();
    hw_gpio_enable(led, &conf);

    hw_console_set_callback(console_input_handle, console_drain_handle);
}

int main(void)
{
    uint32_t tick = 0;

    nosys_setup();
    while(1) {

        if (tick != systicks) {
            tick = systicks;
            if (tick % 1000 == 0) {
                led_toggle();
            }
        }

        hw_poll();
    }
}

