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

#include "hardware.h"

/*******************************************************************************
 * dbg field
*******************************************************************************/
#define MEMORY_SIZE     (32 * 1024)

static char memory_buf[MEMORY_SIZE];
static int  memory_alloced = 0;

static void hw_dbg_memory_reset(void)
{
    memset(memory_buf, 0, MEMORY_SIZE);
    memory_alloced = 0;
}

void hw_dbg_set_systicks(uint32_t x)
{
    system_ticks_count = x;
}

void hw_dbg_reset(void)
{
    hw_dbg_set_systicks(0);

    hw_dbg_memory_reset();

    hw_dbg_console_reset();

    hw_scripts_erase();
}

/*******************************************************************************
 * dbg field end
*******************************************************************************/
uint32_t system_ticks_count = 0;

void hw_setup(void)
{
    hw_setup_gpio();
}

void hw_poll(void)
{
    static uint32_t system_ticks_count_pre = 0;

    if (system_ticks_count_pre != system_ticks_count) {
        system_ticks_count_pre = system_ticks_count;
        systick_event_post();
    }
}

void hw_halt(void)
{
    printf("\nSystem into halt!\n");
    while(1)
        ;
}

void hw_info_get(hw_info_t * info)
{
    (void) info;

    return;
}

int hw_memory_alloc(void **p, int size, int align)
{
    int start = (intptr_t) memory_buf + memory_alloced;
    int shift = 0;

    if (size == 0) {
        return 0;
    }

    if (start % align) {
        shift = align - (start % align);
    } else {
        shift = 0;
    }
    memory_alloced += shift;


    if (size < 0) {
        size = MEMORY_SIZE - memory_alloced;
    } else
    if (memory_alloced + size > MEMORY_SIZE) {
        return -1;
    }

    if (p) {
        *p = memory_buf + memory_alloced;
        memory_alloced += size;
    }

    return size;
}

int hw_pin_map(int id, int port, int pin)
{
    (void) id;
    (void) port;
    (void) pin;

    return 0;
}

const hw_driver_t *hw_device_request(int type, int instance)
{
    switch (type) {
    case DEVICE_TYPE_PIN:       return hw_request_pin(instance);
    case DEVICE_TYPE_ADC:
    case DEVICE_TYPE_DAC:
    case DEVICE_TYPE_PWM:
    case DEVICE_TYPE_PULSE:
    case DEVICE_TYPE_TIMER:
    case DEVICE_TYPE_COUNTER:
    case DEVICE_TYPE_UART:
    case DEVICE_TYPE_USART:
    case DEVICE_TYPE_SPI:
    default:                    return NULL;
    }
}

int hw_device_instances(int type)
{
    switch (type) {
    case DEVICE_TYPE_PIN:       return HW_INSTANCES_PIN;
    case DEVICE_TYPE_ADC:
    case DEVICE_TYPE_DAC:
    case DEVICE_TYPE_PWM:
    case DEVICE_TYPE_PULSE:
    case DEVICE_TYPE_TIMER:
    case DEVICE_TYPE_COUNTER:
    case DEVICE_TYPE_UART:
    case DEVICE_TYPE_USART:
    case DEVICE_TYPE_SPI:
    default:                    return 0;
    }
}

