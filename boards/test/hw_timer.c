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

typedef struct hw_timer_t {
    uint8_t inused;
    uint8_t dev_id;
    const hw_config_timer_t *config;
} hw_timer_t;

static hw_timer_t timer_controls[HW_INSTANCES_TIMER];

static void timer_release(int instance)
{
    /* Do hardware release here */

    timer_controls[instance].inused = 0;
}

static void timer_reset(int instance)
{
    /* Do hardware reset here */

    timer_controls[instance].dev_id = DEVICE_ID_INVALID;
    timer_controls[instance].config = NULL;
}

static int timer_setup(int instance, uint8_t dev_id, const hw_config_t *conf)
{
    hw_timer_t *control = &timer_controls[instance];
    const hw_config_timer_t *config = (const hw_config_timer_t*)conf;
    int err;

    /* hardware setup here */

    err = 0;

    if (!err) {
        control->dev_id = dev_id;
        control->config = config;
    }
    return err;
}

static int timer_set(int instance, int off, uint32_t data)
{
    (void) instance;
    (void) off;
    (void) data;
    return 1;
}

static int timer_size(int instance)
{
    hw_timer_t *control = &timer_controls[instance];

    return control->config->chn_num;
}

static const hw_driver_t timer_driver = {
    .release = timer_release,
    .reset   = timer_reset,
    .setup   = timer_setup,
    .io.map  = {
        .set  = timer_set,
        .size = timer_size
    }
};

const hw_driver_t *hw_request_timer(int instance)
{
    if (instance >= HW_INSTANCES_TIMER || timer_controls[instance].inused) {
        return NULL;
    }

    timer_controls[instance].inused = 1;
    timer_controls[instance].dev_id = DEVICE_ID_INVALID;
    timer_controls[instance].config = NULL;

    printf("request timer %d ok\n", instance);
    return &timer_driver;
}

void hw_setup_timer(void)
{
    int i;

    for (i = 0; i < HW_INSTANCES_TIMER; i++) {
        timer_controls[i].inused = 0;
    }
}

