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
#include "timer.h"

static const hw_device_t *desc;
static const hw_driver_t *driver;

int pulse_enable(void)
{
    desc = hw_device_take("pulse", 2, &driver);

    if (desc) {
        driver->listen(desc->id, 2, DEVICE_EVENT_DRAIN);
        return driver->enable(desc->id, 2);
    } else {
        hw_console_sync_puts("pulse device not find\r\n");
        return 0;
    }
}

int pulse_trigger(void)
{
    if (desc) {
        return driver->io.map.set(desc->id, 2, 0, 500000);
    }
    return 0;
}

int pwm_enable(void)
{
    desc = hw_device_take("pwm", 2, &driver);

    if (desc) {
        driver->listen(desc->id, 2, DEVICE_EVENT_DRAIN);
        return driver->enable(desc->id, 2);
    } else {
        hw_console_sync_puts("pwm device not find\r\n");
        return 0;
    }
}

int pwm_set(int val)
{
    if (desc) {
        return driver->io.map.set(desc->id, 2, 0, val);
    }
    return 0;
}

