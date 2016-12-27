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

#include "device.h"

int device_timer_get(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_timer_t *timer = (hw_config_timer_t *) conf;

    switch (which) {
    case DEVICE_TIMER_CONF_CHANNELS: device_get_sequence(env, val, timer->chn_num, timer->chn_seq);   break;
    case DEVICE_TIMER_CONF_POLARITY: device_get_option(val, timer->polarity, DEVICE_OPT_POLARITY_MAX, device_opt_polarity); break;
    default:                       return -CUPKEE_EINVAL;
    }
    return CUPKEE_OK;
}

int device_timer_set(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_timer_t *timer = (hw_config_timer_t *) conf;

    (void) env;
    switch (which) {
    case DEVICE_TIMER_CONF_CHANNELS: return device_set_sequence(val, HW_CHN_MAX_TIMER, &timer->chn_num, timer->chn_seq);
    case DEVICE_TIMER_CONF_POLARITY: return device_set_option(val, &timer->polarity, DEVICE_OPT_POLARITY_MAX, device_opt_polarity); break;
    default:                       return -CUPKEE_EINVAL;
    }
}

