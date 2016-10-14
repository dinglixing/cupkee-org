

#ifndef __HAL_INC__
#define __HAL_INC__

#include <stdint.h>

#define SYSTEM_TICKS_PRE_SEC     1000

extern uint32_t system_ticks_count;

int hal_init(void);
void hal_loop(void);

int hal_console_set_cb(void (*input)(void *, int), void (*drain)(void));
int hal_console_write_byte(char c);
int hal_console_puts(const char *s);
int hal_console_write_sync_byte(char c);
int hal_console_sync_puts(const char *s);

typedef enum HAL_LED{
    HAL_LED_0 = 1,
    HAL_LED_1 = 2,
    HAL_LED_ALL = 3,
} HAL_LED;

void hal_halt(void);

void hal_led_on(HAL_LED sel);
void hal_led_off(HAL_LED sel);
void hal_led_toggle(HAL_LED sel);

#endif /* __HAL_INC__ */

