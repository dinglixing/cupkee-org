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

#include <bsp.h>
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

static const hw_device_desc_t *hw_device_descs [] = {
    &hw_device_map_desc,
    &hw_device_serial_desc,
};
static const hw_driver_t *hw_drivers [] = {
    &hw_device_map_driver,
    &hw_device_serial_driver,
};

uint32_t system_ticks_count = 0;

const hw_device_desc_t *hw_device_descript(int i)
{
    int max = sizeof(hw_device_descs) / sizeof(hw_device_desc_t *);

    if (i < 0 || i >= max) {
        return NULL;
    }
    return hw_device_descs[i];
}

const hw_device_desc_t *hw_device_take(const char *name, int inst, const hw_driver_t **driver)
{
    int i, max = sizeof(hw_device_descs) / sizeof(hw_device_desc_t *);

    _TRACE("request %s[%d]\n", name, inst);
    for (i = 0; i < max; i++) {
        const hw_device_desc_t *desc = hw_device_descs[i];
        if (!strcmp(name, desc->name)) {
            if (hw_drivers[i]->request(inst)) {
                if (driver) {
                    *driver = hw_drivers[i];
                }
                return desc;
            }
            break;
        }
    }
    return NULL;
}

void hw_setup(void)
{
    hw_gpio_setup();
    hw_adc_setup();
    hw_usart_setup();

    hw_device_map_setup();
    hw_device_serial_setup();
}

void hw_poll(void)
{
    static uint32_t system_ticks_count_pre = 0;

    if (system_ticks_count_pre != system_ticks_count) {
        system_ticks_count_pre = system_ticks_count;
        systick_event_post();
    }

    hw_device_map_poll();
    hw_device_serial_poll();

    // delete
    hw_gpio_poll();
    hw_adc_poll();
    hw_usart_poll();
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

void hw_led_on(void)
{}

void hw_led_off(void)
{}

void hw_led_toggle(void)
{}


