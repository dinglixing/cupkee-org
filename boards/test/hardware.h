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
// ADC
uint16_t hw_dbg_adc_get_channel(int channel);
void hw_dbg_adc_set_channel(int channel, uint16_t v);
void hw_dbg_adc_set_ready(void);
void hw_dbg_adc_clr_ready(void);
int  hw_dbg_adc_test_ready(void);
void hw_dbg_adc_set_eoc(void);
void hw_dbg_adc_clr_eoc(void);
int  hw_dbg_adc_test_eoc(void);
// ADC
void hw_dbg_usart_sbuf_set_space(int instance, int space);
void hw_dbg_usart_sbuf_set_full(int instance);

/******************************************************************************
 * Hardware interface not in bsp.h
******************************************************************************/
#include "system.h"
#include "hw_console.h"
#include "hw_gpio.h"
#include "hw_adc.h"
#include "hw_usart.h"

#endif /* __HW_MOCK_INC__ */
