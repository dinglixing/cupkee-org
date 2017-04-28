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

typedef struct hw_pwm_t {
    uint8_t inused;
    uint8_t dev_id;
    const hw_config_pwm_t *config;
} hw_pwm_t;

static hw_pwm_t pwm_controls[HW_INSTANCES_PWM];

static void pwm_release(int instance)
{
    /* Do hardware release here */

    pwm_controls[instance].inused = 0;
}

static void pwm_reset(int instance)
{
    /* Do hardware reset here */

    pwm_controls[instance].dev_id = DEVICE_ID_INVALID;
    pwm_controls[instance].config = NULL;
}

static int pwm_setup(int instance, uint8_t dev_id, const hw_config_t *conf)
{
    hw_pwm_t *control = &pwm_controls[instance];
    const hw_config_pwm_t *config = (const hw_config_pwm_t*)conf;
    int err;

    /* hardware setup here */

    err = 0;

    if (!err) {
        control->dev_id = dev_id;
        control->config = config;
    }
    return err;
}

static int pwm_set(int instance, int off, uint32_t data)
{
    (void) instance;
    (void) off;
    (void) data;
    return 1;
}

static int pwm_size(int instance)
{
    hw_pwm_t *control = &pwm_controls[instance];

    return control->config->chn_num;
}

static const hw_driver_t pwm_driver = {
    .release = pwm_release,
    .reset   = pwm_reset,
    .setup   = pwm_setup,
    .io.map  = {
        .set  = pwm_set,
        .size = pwm_size
    }
};

const hw_driver_t *hw_request_pwm(int instance)
{
    if (instance >= HW_INSTANCES_PWM || pwm_controls[instance].inused) {
        return NULL;
    }

    pwm_controls[instance].inused = 1;
    pwm_controls[instance].dev_id = DEVICE_ID_INVALID;
    pwm_controls[instance].config = NULL;

    return &pwm_driver;
}

void hw_setup_pwm(void)
{
    int i;

    for (i = 0; i < HW_INSTANCES_PWM; i++) {
        pwm_controls[i].inused = 0;
    }
}

