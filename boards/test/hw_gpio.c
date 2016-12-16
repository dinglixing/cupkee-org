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

typedef struct hw_pin_t {
    uint8_t inused;
    uint8_t dev_id;
    hw_config_pin_t *config;
} hw_pin_t;

static hw_pin_t pin_controls[HW_INSTANCES_PIN];

static void pin_release(int instance)
{
    /* Do hardware release here */

    pin_controls[instance].inused = 0;
}

static void pin_reset(int instance)
{
    /* Do hardware reset here */

    pin_controls[instance].dev_id = DEVICE_ID_INVALID;
    pin_controls[instance].config = NULL;
}

static int pin_setup(int instance, uint8_t dev_id, hw_config_t *conf)
{
    return -CUPKEE_EIMPLEMENT;
}

static int pin_poll(int instance)
{
    return -CUPKEE_EIMPLEMENT;
}

static int pin_get(int instance, int off, uint32_t *data)
{
    return -CUPKEE_EIMPLEMENT;
}

static int pin_set(int instance, int off, uint32_t data)
{
    return -CUPKEE_EIMPLEMENT;
}

static int pin_size(int instance)
{
    return -CUPKEE_EIMPLEMENT;
}

static const hw_driver_t pin_driver = {
    .release = pin_release,
    .reset   = pin_reset,
    .setup   = pin_setup,
    .poll    = pin_poll,
    .io.map  = {
        .get = pin_get,
        .set = pin_set,
        .size = pin_size
    }
};

const hw_driver_t *hw_request_pin(int instance)
{
    if (instance >= HW_INSTANCES_PIN || pin_controls[instance].inused) {
        return NULL;
    }

    pin_controls[instance].inused = 1;
    pin_controls[instance].dev_id = DEVICE_ID_INVALID;
    pin_controls[instance].config = NULL;

    return &pin_driver;
}

void hw_setup_gpio(void)
{
    int i;

    for (i = 0; i < HW_INSTANCES_PIN; i++) {
        pin_controls[i].inused = 0;
    }
}

