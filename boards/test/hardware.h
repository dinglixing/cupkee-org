#ifndef __HW_MOCK_INC__
#define __HW_MOCK_INC__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/******************************************************************************
 * Debug api
******************************************************************************/
// MISC
void hw_dbg_reset(void);
void hw_dbg_set_systicks(uint32_t x);
// CONSOLE
void hw_dbg_console_reset(void);
void hw_dbg_console_set_input(const char *data);
int  hw_dbg_console_get_reply(char **ptr);
void hw_dbg_console_clr_buf(void);
// GPIO
int hw_dbg_gpio_get_pin(int port, int pin);
int hw_dbg_gpio_set_pin(int port, int pin);
int hw_dbg_gpio_clr_pin(int port, int pin);

/******************************************************************************
 * Hardware interface not in bsp.h
******************************************************************************/
// CONSOLE
void hw_console_setup(void);
int  hw_console_putc(int ch);
// GPIO
int  hw_gpio_setup(void);
void hw_gpio_poll(void);

//  form cupkee system
void device_event_post(int magic, int n, int type);
void timeout_event_post(void);

#endif /* __HW_MOCK_INC__ */
