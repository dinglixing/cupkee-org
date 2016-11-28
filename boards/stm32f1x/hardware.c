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

#define MAIN_STACK_SIZE 8192

extern vector_table_t vector_table;
extern char end;
static int memory_alloced = 0;
static int memory_size = 0;

static const hw_device_t *hw_devices[] = {
    &hw_device_pin,
    &hw_device_key,
    &hw_device_adc,
    &hw_device_usart,
    &hw_device_pulse,
    &hw_device_pwm,
    &hw_device_count,
    &hw_device_timer,
};

static const hw_driver_t *hw_drivers[] = {
    &hw_driver_pin,
    &hw_driver_key,
    &hw_driver_adc,
    &hw_driver_usart,
    &hw_driver_pulse,
    &hw_driver_pwm,
    &hw_driver_count,
    &hw_driver_timer,
};

static void hw_memory_init(void)
{
    memory_alloced = 0;
    memory_size = (char *)(vector_table.initial_sp_value) - (&end) - MAIN_STACK_SIZE;
}

static void hw_setup_systick(void)
{
    systick_set_frequency(SYSTEM_TICKS_PRE_SEC, 72000000);

    systick_interrupt_enable();
    systick_counter_enable();
}

/* systick interrupt handle routing  */
uint32_t system_ticks_count = 0;
void sys_tick_handler(void)
{
    system_ticks_count++;
}

int hw_memory_alloc(void **p, int size, int align)
{
    int start = (intptr_t) (&end) + memory_alloced;
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
        size = memory_size - memory_alloced;
    } else
    if (memory_alloced + size > memory_size) {
        return -1;
    }

    if (p) {
        *p = &end + memory_alloced;
        memory_alloced += size;
    }

    return size;
}

void hw_info_get(hw_info_t *info)
{
    if (info) {
        info->sys_freq = 72000000;
        info->sys_ticks_pre_sec = SYSTEM_TICKS_PRE_SEC;
        info->ram_sz = 64 * 1024;
        info->rom_sz = desig_get_flash_size() * 1024;
        info->ram_base = (void *)0x20000000;
        info->rom_base = (void *)0x08000000;
    }
}

void hw_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

    hw_setup_systick();

    hw_setup_usb();         // usb used as console
    hw_setup_gpio();
    hw_setup_adc();
    hw_setup_usart();
    hw_setup_timer();
    hw_setup_storage();

    hw_memory_init();
}

void hw_poll(void)
{
    static uint32_t system_ticks_count_pre = 0;

    hw_poll_usb();
    hw_poll_usart();
    hw_poll_timer();

    if (system_ticks_count_pre != system_ticks_count) {
        system_ticks_count_pre = system_ticks_count;

        systick_event_post();

        hw_poll_gpio();
        hw_poll_adc();
    }
}

void hw_halt(void)
{
    while (1) {
    }
}

const hw_device_t *hw_device_descript(int i)
{
    int max = sizeof(hw_devices) / sizeof(hw_device_t *);

    if (i < 0 || i >= max) {
        return NULL;
    }
    return hw_devices[i];
}

const hw_device_t *hw_device_take(const char *name, int inst, const hw_driver_t **driver)
{
    int i, max = sizeof(hw_devices) / sizeof(hw_device_t *);

    _TRACE("request %s[%d]\n", name, inst);
    for (i = 0; i < max; i++) {
        const hw_device_t *desc = hw_devices[i];
        if (!strcmp(name, desc->name)) {
            if (hw_drivers[i]->request(desc->id, inst)) {
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

