

#ifndef __BSP_INC__
#define __BSP_INC__

#include <stdint.h>

#define SYSTEM_TICKS_PRE_SEC    1000
#define SYSTEM_STACK_SIZE       (8 * 1024)

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
#define OPT_GPIO_MOD_READABLE(mod)      ((mod) == OPT_GPIO_MOD_DUAL || (mod) < OPT_GPIO_MOD_OUTPUT_PUSHPULL)
#define OPT_GPIO_MOD_WRITEABLE(mod)     ((mod) >= OPT_GPIO_MOD_OUTPUT_PUSHPULL)
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
    uint8_t pins[GPIO_GROUP_SIZE];
} hw_gpio_conf_t;

int hw_gpio_group_alloc(void);
int hw_gpio_group_release(int grp);
void hw_gpio_conf_reset(hw_gpio_conf_t *conf);

int hw_gpio_pin_is_valid(uint8_t pin);
int hw_gpio_enable (int grp, hw_gpio_conf_t *conf);
int hw_gpio_disable(int grp);
int hw_gpio_event_enable(int grp, int event);
int hw_gpio_event_disable(int grp, int event);

int hw_gpio_read (int grp, uint32_t *data);
int hw_gpio_write(int grp, uint32_t data);

/* ADC */
#define ADC_DEVICE_ID                  (1)  //
enum ADC_EVENT_TYPE {
    ADC_EVENT_DATA = 0,
    ADC_EVENT_MAX,
};

#define CFG_ADC_CHANNEL                 0       // channel pins
#define CFG_ADC_INTERVAL                1       // sample repeat nMS or 0(no repeat)
#define CFG_ADC_MAX                     2

/* board dependent start */
#define ADC_CHANNEL_MAX                 18
#define ADC_MAX                         1
/* board dependent end */

typedef struct hw_adc_conf_t{
    uint32_t channels;
    uint32_t interval;
} hw_adc_conf_t;

int hw_adc_alloc(void);
int hw_adc_release(int adc);
void hw_adc_conf_reset(hw_adc_conf_t *conf);

int hw_adc_enable (int adc, hw_adc_conf_t *conf);
int hw_adc_disable(int adc);
int hw_adc_event_enable(int adc, int event);
int hw_adc_event_disable(int adc, int event);

int hw_adc_read (int adc, int channel, uint32_t *data);

#endif /* __BSP_INC__ */

