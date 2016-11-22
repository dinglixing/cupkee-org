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

#ifndef __HARDWARE_INC__
#define __HARDWARE_INC__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>

#include <bsp.h>
#include "system.h"

#define PIN_DEVICE_NAME         "pin"
#define PIN_DEVICE_ID           0
#define PIN_EVENT_NUM           2
#define PIN_CONFIG_NUM          1
#define PIN_INSTANCE_NUM        1

#define KEY_DEVICE_NAME         "key"
#define KEY_DEVICE_ID           1
#define KEY_EVENT_NUM           2
#define KEY_CONFIG_NUM          0
#define KEY_INSTANCE_NUM        1

#define ADC_DEVICE_NAME         "adc"
#define ADC_DEVICE_ID           2
#define ADC_EVENT_NUM           2
#define ADC_CONFIG_NUM          2
#define ADC_INSTANCE_NUM        1

#define USART_DEVICE_NAME       "usart"
#define USART_DEVICE_ID         3
#define USART_EVENT_NUM         3
#define USART_CONFIG_NUM        3
#define USART_INSTANCE_NUM      1

#include "hw_usb.h"
#include "hw_misc.h"
#include "hw_gpio.h"
#include "hw_adc.h"
#include "hw_usart.h"

#if 0
#define _TRACE(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
#define _TRACE(fmt, ...)    //
#endif

#endif /* __HARDWARE_INC__ */

