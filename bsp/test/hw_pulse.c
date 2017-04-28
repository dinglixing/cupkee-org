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

typedef struct hw_pulse_t {
    uint8_t inused;
    uint8_t dev_id;
    const hw_config_pulse_t *config;
} hw_pulse_t;

static hw_pulse_t pulse_controls[HW_INSTANCES_PULSE];

static void pulse_release(int instance)
{
    /* Do hardware release here */

    pulse_controls[instance].inused = 0;
}

static void pulse_reset(int instance)
{
    /* Do hardware reset here */

    pulse_controls[instance].dev_id = DEVICE_ID_INVALID;
    pulse_controls[instance].config = NULL;
}

static int pulse_setup(int instance, uint8_t dev_id, const hw_config_t *conf)
{
    hw_pulse_t *control = &pulse_controls[instance];
    const hw_config_pulse_t *config = (const hw_config_pulse_t*)conf;
    int err;

    /* hardware setup here */

    err = 0;

    if (!err) {
        control->dev_id = dev_id;
        control->config = config;
    }
    return err;
}

static int pulse_set(int instance, int off, uint32_t data)
{
    (void) instance;
    (void) off;
    (void) data;
    return 1;
}

static int pulse_size(int instance)
{
    hw_pulse_t *control = &pulse_controls[instance];

    return control->config->chn_num;
}

static const hw_driver_t pulse_driver = {
    .release = pulse_release,
    .reset   = pulse_reset,
    .setup   = pulse_setup,
    .io.map  = {
        .set  = pulse_set,
        .size = pulse_size
    }
};

const hw_driver_t *hw_request_pulse(int instance)
{
    if (instance >= HW_INSTANCES_PULSE || pulse_controls[instance].inused) {
        return NULL;
    }

    pulse_controls[instance].inused = 1;
    pulse_controls[instance].dev_id = DEVICE_ID_INVALID;
    pulse_controls[instance].config = NULL;

    return &pulse_driver;
}

void hw_setup_pulse(void)
{
    int i;

    for (i = 0; i < HW_INSTANCES_PULSE; i++) {
        pulse_controls[i].inused = 0;
    }
}

