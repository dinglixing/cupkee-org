/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016-2017 Lixing Ding <ding.lixing@gmail.com>

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

#include <cupkee.h>

const char *device_opt_dir[] = {
    "in", "out", "dual"
};

const char *device_opt_polarity[] = {
    "positive", "negative", "edge"
};

const char *device_opt_parity[] = {
    "none", "odd", "even",
};

const char *device_opt_stopbits[] = {
    "1bit", "2bit", "0.5bit", "1.5bit"
};

const char *device_pin_conf_names[] = {
    "pinNum", "pinStart", "dir"
};

const char *device_adc_conf_names[] = {
    "channel", "interval"
};

const char *device_dac_conf_names[] = {
    "channel", "interval"
};

const char *device_pwm_pulse_timer_counter_conf_names[] = {
    "channel", "polarity", "period"
};

const char *device_uart_conf_names[] = {
    "baudrate", "dataBits", "stopBits", "parity"
};

static const char *device_event_names[] = {
    "error", "data", "drain", "ready"
};

static const cupkee_device_desc_t device_pin = {
    .name = "pin",
    .type = DEVICE_TYPE_PIN,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 3,
    .conf_names = device_pin_conf_names,
};

static const cupkee_device_desc_t device_adc = {
    .name = "adc",
    .type = DEVICE_TYPE_ADC,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 2,
    .conf_names = device_adc_conf_names,
};

static const cupkee_device_desc_t device_pwm = {
    .name = "pwm",
    .type = DEVICE_TYPE_PWM,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 3,
    .conf_names = device_pwm_pulse_timer_counter_conf_names,
};

static const cupkee_device_desc_t device_pulse = {
    .name = "pulse",
    .type = DEVICE_TYPE_PULSE,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 2,
    .conf_names = device_pwm_pulse_timer_counter_conf_names,
};

static const cupkee_device_desc_t device_timer = {
    .name = "timer",
    .type = DEVICE_TYPE_TIMER,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 2,
    .conf_names = device_pwm_pulse_timer_counter_conf_names,
};

static const cupkee_device_desc_t device_counter = {
    .name = "counter",
    .type = DEVICE_TYPE_COUNTER,
    .category = DEVICE_CATEGORY_MAP,
    .conf_num = 3,
    .conf_names = device_pwm_pulse_timer_counter_conf_names,
};

static const cupkee_device_desc_t device_uart = {
    .name = "uart",
    .type = DEVICE_TYPE_UART,
    .category = DEVICE_CATEGORY_STREAM,
    .conf_num = 4,
    .conf_names = device_uart_conf_names,
};

static const cupkee_device_desc_t device_usb_cdc = {
    .name = "usb-cdc",
    .type = DEVICE_TYPE_USB_CDC,
    .category = DEVICE_CATEGORY_STREAM,
    .conf_num = 0,
};

const cupkee_device_desc_t *device_entrys[] = {
    &device_pin,
    &device_adc,
    &device_pwm,
    &device_pulse,
    &device_timer,
    &device_counter,
    &device_uart,
    &device_usb_cdc,
    NULL
};

