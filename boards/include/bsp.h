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


#ifndef __BSP_INC__
#define __BSP_INC__

#include <stdint.h>

#define SYSTEM_TICKS_PRE_SEC    1000
#define SYSTEM_STACK_SIZE       (8 * 1024)
#define HW_INVALID_VAL          0xffffffff

/* PWM configurables */

/* CAPTURE configurables */

/* DAC configurables */

/* UART configurables */

/* SPI configurables */

/* I2S configurables */

/* I2C configurables */

/* CAN configurables */

/* DIO configurables */

typedef struct hw_info_t {
    int ram_sz;
    int rom_sz;
    void *ram_base;
    void *rom_base;
    unsigned sys_freq;
    unsigned sys_ticks_pre_sec;
} hw_info_t;

extern uint32_t system_ticks_count;

void hw_setup(void);
void _hw_reset(void);

void hw_poll(void);
void hw_halt(void);

void hw_info_get(hw_info_t *);

int hw_memory_alloc(void **p, int size, int align);

/* console */
int hw_console_set_callback(void (*input)(void *, int), void (*drain)(void));
int hw_console_putc(int ch);
int hw_console_puts(const char *s);
int hw_console_sync_putc(int ch);
int hw_console_sync_puts(const char *s);

/* script storage */
int hw_scripts_erase(void);
int hw_scripts_remove(int id);
int hw_scripts_save(const char *s);
const char *hw_scripts_load(const char *prev);

void hw_led_on(void);
void hw_led_off(void);
void hw_led_toggle(void);

/* GPIO */
#define GPIO_DEVICE_ID         (0)  //
enum GPIO_EVENT_TYPE {
    GPIO_EVENT_CHANGE = 0,
    GPIO_EVENT_MAX,
};

#define CFG_GPIO_PIN                    0       // Selected pins
#define CFG_GPIO_MOD                    1       // direction:
#define CFG_GPIO_SPEED                  2
#define CFG_GPIO_MAX                    3

#define OPT_GPIO_MOD_INPUT_FLOAT        0       // input
#define OPT_GPIO_MOD_INPUT_PULL_UPDOWN  1       // input
#define OPT_GPIO_MOD_OUTPUT_PUSHPULL    2       // output only
#define OPT_GPIO_MOD_OUTPUT_OPENDRAIN   3       // output or dual
#define OPT_GPIO_MOD_DUAL               4       // input
#define OPT_GPIO_MOD_MAX                5

#define GPIO_READABLE(mod)      ((mod) == OPT_GPIO_MOD_DUAL || (mod) < OPT_GPIO_MOD_OUTPUT_PUSHPULL)
#define GPIO_WRITEABLE(mod)     ((mod) >= OPT_GPIO_MOD_OUTPUT_PUSHPULL)

/* board dependent start */
#define GPIO_PORT_MAX                   7
#define GPIO_PIN_MAX                    16
#define GPIO_GROUP_MAX                  4
#define GPIO_GROUP_SIZE                 8
#define OPT_GPIO_SPEED_MAX              50      // M
#define OPT_GPIO_SPEED_MIN              2       // M
/* board dependent end */

typedef struct hw_gpio_conf_t{
    uint8_t mod;
    uint8_t speed;
    uint8_t pin_num;
    uint8_t pin_seq[GPIO_GROUP_SIZE];
} hw_gpio_conf_t;

int hw_gpio_group_alloc(void);
int hw_gpio_group_release(int grp);
void hw_gpio_conf_reset(hw_gpio_conf_t *conf);

int hw_gpio_pin_is_valid(uint8_t pin);
int hw_gpio_enable (int grp, hw_gpio_conf_t *conf);
int hw_gpio_disable(int grp);
int hw_gpio_event_enable(int grp, int event);
int hw_gpio_event_disable(int grp, int event);

int hw_gpio_read (int instance, int off, uint32_t *data);
int hw_gpio_write(int instance, int off, uint32_t data);

/* ADC */
#define ADC_DEVICE_ID                  (1)  //
enum ADC_EVENT_TYPE {
    ADC_EVENT_READY = 0,
    ADC_EVENT_ERROR,
    ADC_EVENT_DATA,
    ADC_EVENT_MAX,
};

#define CFG_ADC_CHANNEL                 0       // channel pins
#define CFG_ADC_INTERVAL                1       // sample repeat nMS or 0(no repeat)
#define CFG_ADC_MAX                     2

/* board dependent start */
#define ADC_CHANNEL_MAX                 8
#define ADC_MAX                         1
/* board dependent end */

typedef struct hw_adc_conf_t{
    uint32_t interval;
    int8_t   chn_num;
    uint8_t  chn_seq[ADC_CHANNEL_MAX];
} hw_adc_conf_t;

int hw_adc_alloc(void);
int hw_adc_release(int adc);
void hw_adc_conf_reset(hw_adc_conf_t *conf);

int hw_adc_enable (int adc, hw_adc_conf_t *conf);
int hw_adc_disable(int adc);
int hw_adc_event_enable(int adc, int event);
int hw_adc_event_disable(int adc, int event);

int hw_adc_read (int adc, int off, uint32_t *data);

/* USART */
#define DEVICE_USART_ID                 (2)  //
enum USART_EVENT_TYPE {
    USART_EVENT_DATA = 0,
    USART_EVENT_DRAIN,
    USART_EVENT_ERROR,
    USART_EVENT_MAX,
};

#define CFG_USART_BAUDRATE                0
#define CFG_USART_DATABITS                1
#define CFG_USART_STOPBITS                2
#define CFG_USART_PARITY                  3
#define CFG_USART_MAX                     4

#define OPT_USART_PARITY_NONE             0
#define OPT_USART_PARITY_ODD              1
#define OPT_USART_PARITY_EVEN             2
#define OPT_USART_PARITY_MAX              3

/* board dependent start */
#define USART_INSTANCE_MAX                1
/* board dependent end */

typedef struct hw_usart_conf_t{
    uint32_t baudrate;
    uint8_t  databits;
    uint8_t  stopbits;
    uint8_t  parity;
} hw_usart_conf_t;

int hw_usart_alloc(void);
int hw_usart_release(int adc);
void hw_usart_conf_reset(hw_usart_conf_t *conf);

int hw_usart_enable (int instance, hw_usart_conf_t *conf);
int hw_usart_disable(int instance);
int hw_usart_event_enable(int instance, int event);
int hw_usart_event_disable(int instance, int event);

int hw_usart_send(int instance, int size, uint8_t *data);
int hw_usart_recv_len(int instance);
void hw_usart_recv_load(int instance, int size, uint8_t *buf);

#endif /* __BSP_INC__ */

