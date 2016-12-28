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

typedef struct hw_counter_t {
    uint8_t inused;
    uint8_t dev_id;
    const hw_config_counter_t *config;
} hw_counter_t;

static hw_counter_t counter_controls[HW_INSTANCES_COUNTER];

static void counter_release(int instance)
{
    /* Do hardware release here */

    counter_controls[instance].inused = 0;
}

static void counter_reset(int instance)
{
    /* Do hardware reset here */

    counter_controls[instance].dev_id = DEVICE_ID_INVALID;
    counter_controls[instance].config = NULL;
}

static int counter_setup(int instance, uint8_t dev_id, const hw_config_t *conf)
{
    hw_counter_t *control = &counter_controls[instance];
    const hw_config_counter_t *config = (const hw_config_counter_t*)conf;
    int err;

    /* hardware setup here */

    err = 0;

    if (!err) {
        control->dev_id = dev_id;
        control->config = config;
    }
    return err;
}

static int counter_set(int instance, int off, uint32_t data)
{
    (void) instance;
    (void) off;
    (void) data;
    return 1;
}

static int counter_size(int instance)
{
    hw_counter_t *control = &counter_controls[instance];

    return control->config->chn_num;
}

static const hw_driver_t counter_driver = {
    .release = counter_release,
    .reset   = counter_reset,
    .setup   = counter_setup,
    .io.map  = {
        .set  = counter_set,
        .size = counter_size
    }
};

const hw_driver_t *hw_request_counter(int instance)
{
    if (instance >= HW_INSTANCES_COUNTER || counter_controls[instance].inused) {
        return NULL;
    }

    counter_controls[instance].inused = 1;
    counter_controls[instance].dev_id = DEVICE_ID_INVALID;
    counter_controls[instance].config = NULL;

    return &counter_driver;
}

void hw_setup_counter(void)
{
    int i;

    for (i = 0; i < HW_INSTANCES_COUNTER; i++) {
        counter_controls[i].inused = 0;
    }
}

