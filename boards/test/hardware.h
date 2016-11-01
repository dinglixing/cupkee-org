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
#include "system.h"
#include "hw_console.h"
#include "hw_gpio.h"

#endif /* __HW_MOCK_INC__ */
