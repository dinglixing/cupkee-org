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

#ifndef __HW_MOCK_INC__
#define __HW_MOCK_INC__

/******************************************************************************
 * system interface
******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <cupkee_errno.h>
#include <cupkee_event.h>
#include <cupkee_utils.h>
#include <cupkee_bsp.h>

/******************************************************************************
 * Hardware interface
******************************************************************************/
#include "hw_console.h"

#include "hw_gpio.h"
#include "hw_uart.h"

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

// pin device
void hw_dbg_pin_setup_status_set(int instance, int status);
void hw_dbg_pin_data_set(int instance, int offset, uint32_t value);
uint32_t hw_dbg_pin_data_get(int instance, int offset);
void hw_dbg_pin_trigger_error(int instance, int code);
void hw_dbg_pin_trigger_data(int instance, uint32_t data);

// uart device
void hw_dbg_uart_setup_status_set(int instance, int status);

#if 0
#define _TRACE(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define _TRACE(fmt, ...)    //
#endif

#endif /* __HW_MOCK_INC__ */

