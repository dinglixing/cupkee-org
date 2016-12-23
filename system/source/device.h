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

#ifndef __DEVICE_INC__
#define __DEVICE_INC__

#include <cupkee.h>

#include "device_util.h"

#define DEVICE_FL_ENABLE            1

#define DEVICE_PIN_CONF_NUM         0
#define DEVICE_PIN_CONF_START       1
#define DEVICE_PIN_CONF_DIR         2
#define DEVICE_PIN_CONF_MAX         3

#define DEVICE_UART_CONF_BAUDRATE   0
#define DEVICE_UART_CONF_DATABITS   1
#define DEVICE_UART_CONF_STOPBITS   2
#define DEVICE_UART_CONF_PARITY     3
#define DEVICE_UART_CONF_MAX        4

#define DEVICE_ADC_CONF_CHANNELS    0
#define DEVICE_ADC_CONF_INTERVAL    1
#define DEVICE_ADC_CONF_MAX         2

#define DEVICE_PWM_CONF_CHANNELS    0
#define DEVICE_PWM_CONF_POLARITY    1
#define DEVICE_PWM_CONF_PERIOD      2
#define DEVICE_PWM_CONF_MAX         3

void device_init(void);
void device_poll(void);
void device_sync(uint32_t systicks);
void device_event_proc(env_t *env, int event);

extern const char *device_opt_dir[];
extern const char *device_opt_polarity[];
extern const char *device_opt_parity[];
extern const char *device_opt_stopbits[];

extern const char *device_pin_conf_names[];
extern const char *device_adc_conf_names[];
extern const char *device_dac_conf_names[];
extern const char *device_pwm_pulse_timer_counter_conf_names[];
extern const char *device_uart_conf_names[];

typedef struct cupkee_device_desc_t {
    const char *name;
    uint8_t type;
    uint8_t category;
    uint8_t conf_num;
    uint8_t event_mask;
    const char **conf_names;
    void (*def)(hw_config_t *conf);
    int (*set)(env_t *env, hw_config_t *conf, int which, val_t *val);
    int (*get)(env_t *env, hw_config_t *conf, int which, val_t *val);
} cupkee_device_desc_t;

typedef struct cupkee_device_t {
    uint16_t magic;
    int16_t  error;
    uint8_t  inst;
    uint8_t  flags;
    uint8_t  event;
    uint8_t  reserved;
    val_t   *event_handle[DEVICE_EVENT_MAX];
    hw_config_t                  conf;
    const hw_driver_t           *driver;
    const cupkee_device_desc_t  *desc;
    struct cupkee_device_t      *next;
} cupkee_device_t;

#include "device_pin.h"
#include "device_adc.h"
#include "device_pwm.h"
#include "device_pulse.h"
#include "device_timer.h"
#include "device_counter.h"
#include "device_uart.h"

#endif /* __DEVICE_INC__ */

