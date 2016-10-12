

#ifndef __HAL_INC__
#define __HAL_INC__

int hal_init(void);
void hal_loop(void);

int hal_console_out(const char *str, int len);
int hal_console_set_cb(void (*input)(void *, int), void (*drain)(void));

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

