

#ifndef __BSP_INC__
#define __BSP_INC__

#include <stdint.h>

#define SYSTEM_TICKS_PRE_SEC    1000
#define SYSTEM_STACK_SIZE       (8 * 1024)

/* PWM configurables */

/* CAPTURE configurables */

/* ADC configurables */

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

/*
 * GPIO
 */
#define GPIO_PORT_MAX           8       // 
#define GPIO_PIN_MAX            16      // 
#define GPIO_GROUP_MAX          4       // board depened
#define GPIO_GROUP_SIZE         8       // board depened

#define GPIO_EVENT_MAGIC        (0x20)  //
enum GPIO_EVENT_TYPE {
    GPIO_EVENT_CHANGE = 0,
    GPIO_EVENT_MAX,
};

#define CFG_GPIO_SEL            0       // Selected pins
#define CFG_GPIO_DIR            1       // direction:
#define CFG_GPIO_MOD            2
#define CFG_GPIO_SPEED          3

#define OPT_GPIO_DIR_OUT        0       // output
#define OPT_GPIO_DIR_IN         1       // input
#define OPT_GPIO_DIR_DUAL       2       // in & out put

#define OPT_GPIO_MOD_PUSHPULL   0       // output only
#define OPT_GPIO_MOD_OPENDRAIN  1       // output or dual
#define OPT_GPIO_MOD_ANALOG     2       // analog in/out
#define OPT_GPIO_MOD_FLOATING   3       // input
#define OPT_GPIO_MOD_PULLDOWN   4       // input
#define OPT_GPIO_MOD_PULLUP     5       // input

#define MAX_GPIO_SPEED          50      // M
#define MIN_GPIO_SPEED          2       // M


typedef struct hw_gpio_conf_t{
    uint8_t speed;
    uint8_t dir;
    uint8_t mod;
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


#endif /* __BSP_INC__ */

