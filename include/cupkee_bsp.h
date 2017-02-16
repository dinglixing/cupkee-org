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

#ifndef __CUPKEE_BSP_INC__
#define __CUPKEE_BSP_INC__

#include <stdint.h>

/****************************************************************/
/* hardware defines                                             */
/****************************************************************/
#define SYSTEM_TICKS_PRE_SEC                1000
#define SYSTEM_STACK_SIZE                   (8 * 1024)

#define HW_PIN_MAX                          16
#define HW_CHN_MAX_ADC                      4
#define HW_CHN_MAX_PWM                      4
#define HW_CHN_MAX_PULSE                    4
#define HW_CHN_MAX_TIMER                    2
#define HW_CHN_MAX_COUNTER                  4

#define HW_BUF_SIZE                         32

/****************************************************************/
/* system define                                                */
/****************************************************************/
#define DEVICE_ID_INVALID                   0xff

#define DEVICE_EVENT_ERR                    0
#define DEVICE_EVENT_DATA                   1
#define DEVICE_EVENT_DRAIN                  2
#define DEVICE_EVENT_READY                  3
#define DEVICE_EVENT_MAX                    4

#define DEVICE_CATEGORY_MAP                 0
#define DEVICE_CATEGORY_STREAM              1
#define DEVICE_CATEGORY_BLOCK               2
#define DEVICE_CATEGORY_MAX                 3

enum DEVICE_TYPE {
    DEVICE_TYPE_PIN = 0,
    DEVICE_TYPE_ADC,
    DEVICE_TYPE_DAC,
    DEVICE_TYPE_PWM,
    DEVICE_TYPE_PULSE,
    DEVICE_TYPE_TIMER,
    DEVICE_TYPE_COUNTER,
    DEVICE_TYPE_UART,
    DEVICE_TYPE_USART,
    DEVICE_TYPE_SPI,
    DEVICE_TYPE_I2C,
    DEVICE_TYPE_USB_CDC,
    DEVICE_TYPE_DUMMY
};

#define DEVICE_OPT_DIR_IN                   0   // default
#define DEVICE_OPT_DIR_OUT                  1
#define DEVICE_OPT_DIR_DUAL                 2
#define DEVICE_OPT_DIR_MAX                  3

#define DEVICE_OPT_POLARITY_POSITIVE        0   // default
#define DEVICE_OPT_POLARITY_NEGATIVE        1
#define DEVICE_OPT_POLARITY_EDGE            2
#define DEVICE_OPT_POLARITY_MAX             3

#define DEVICE_OPT_PARITY_NONE              0   // default
#define DEVICE_OPT_PARITY_ODD               1
#define DEVICE_OPT_PARITY_EVEN              2
#define DEVICE_OPT_PARITY_MAX               3

#define DEVICE_OPT_STOPBITS_1               0   // default
#define DEVICE_OPT_STOPBITS_2               1
#define DEVICE_OPT_STOPBITS_0_5             2
#define DEVICE_OPT_STOPBITS_1_5             3
#define DEVICE_OPT_STOPBITS_MAX             4

typedef struct hw_info_t {
    int ram_sz;
    int rom_sz;
    void *ram_base;
    void *rom_base;
    unsigned sys_freq;
    unsigned sys_ticks_pre_sec;
} hw_info_t;

typedef struct hw_config_pin_t {
    uint8_t num;
    uint8_t start;
    uint8_t dir;         // DEVICE_OPT_DIR
} hw_config_pin_t;

typedef struct hw_config_adc_t {
    uint16_t interval;
    uint8_t chn_num;
    uint8_t chn_seq[HW_CHN_MAX_ADC];
} hw_config_adc_t;

typedef struct hw_config_pwm_t {
    uint16_t period;     // ms
    uint8_t  polarity;   // DEVICE_OPT_POLARITY
    uint8_t chn_num;
    uint8_t chn_seq[HW_CHN_MAX_PWM];
} hw_config_pwm_t;

typedef struct hw_config_pulse_t {
    uint8_t  polarity;   // DEVICE_OPT_POLARITY
    uint8_t chn_num;
    uint8_t chn_seq[HW_CHN_MAX_PULSE];
} hw_config_pulse_t;

typedef struct hw_config_timer_t {
    uint8_t  polarity;   // DEVICE_OPT_POLARITY
    uint8_t chn_num;
    uint8_t chn_seq[HW_CHN_MAX_TIMER];
} hw_config_timer_t;

typedef struct hw_config_counter_t {
    uint16_t period;     // us
    uint8_t  polarity;   // DEVICE_OPT_POLARITY
    uint8_t chn_num;
    uint8_t chn_seq[HW_CHN_MAX_COUNTER];
} hw_config_counter_t;

typedef struct hw_config_uart_t {
    uint32_t baudrate;
    uint8_t  data_bits;
    uint8_t  stop_bits;  // DEVICE_OPT_STOPBITS
    uint8_t  parity;     // DEVICE_OPT_PARITY
} hw_config_uart_t;

typedef struct hw_config_t {
    union {
        hw_config_pin_t     pin;
        hw_config_adc_t     adc;
        hw_config_pwm_t     pwm;
        hw_config_pulse_t   pulse;
        hw_config_timer_t   timer;
        hw_config_counter_t counter;
        hw_config_uart_t    uart;
    } data;
} hw_config_t;

typedef struct hw_driver_t {
    void (*release) (int inst);
    void (*reset) (int inst);
    int  (*setup) (int inst, uint8_t devid, const hw_config_t *conf);
    void (*sync)  (int inst, uint32_t systicks);
    void (*poll)  (int inst);

    int  (*get) (int inst, int offset, uint32_t*data);
    int  (*set) (int inst, int offset, uint32_t data);
    int  (*size)(int inst);
    union {
        struct {
            int (*recv) (int inst, int max, void *buf);
            int (*send) (int inst, int n,   void *data);
            int (*recv_sync) (int inst, int n, uint8_t *buf);
            int (*send_sync) (int inst, int n, const uint8_t *data);
            int (*received) (int inst);
        } stream;
        struct {
            int (*read)  (int inst, int offset, int size, void *buf);
            int (*write) (int inst, int offset, int size, void *buf);
        } block;
    } io;
} hw_driver_t;

/****************************************************************/
/* system utils                                                 */
/****************************************************************/

/****************************************************************/
/* hardware implements                                          */
/****************************************************************/
extern uint32_t system_ticks_count;

void hw_setup(void);
void _hw_reset(void);

void hw_poll(void);
void hw_halt(void);

void hw_info_get(hw_info_t *);

int  hw_memory_alloc(void **p, int size, int align);

/* DEBUG LED */
int  hw_led_map(int port, int pin);
void hw_led_set(void);
void hw_led_clear(void);
void hw_led_toggle(void);

/* GPIO */
int hw_pin_map(int id, int port, int pin);

/* DEVICE */
const hw_driver_t *hw_device_request(int type, int inst);
int   hw_device_instances(int type);

#endif /* __CUPKEE_BSP_INC__ */

