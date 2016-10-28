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

#include <bsp.h>
#include "hw_usb.h"
#include "hw_storage.h"
#include "hw_gpio.h"

#define MAIN_STACK_SIZE 8192

/* systick interrupt handle routing  */
uint32_t system_ticks_count = 0;
void sys_tick_handler(void)
{
    system_ticks_count++;
}

extern vector_table_t vector_table;
extern char end;
static int memory_alloced = 0;
static int memory_size = 0;

static void hw_memory_init(void)
{
    memory_alloced = 0;
    memory_size = (char *)(vector_table.initial_sp_value) - (&end) - MAIN_STACK_SIZE;
}

static void hw_led_setup(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);

	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO8);

	gpio_set(GPIOA, GPIO8);
}

static void hw_systick_setup(void)
{
    systick_set_frequency(SYSTEM_TICKS_PRE_SEC, 72000000);

    systick_interrupt_enable();
    systick_counter_enable();
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

void hw_led_on(void)
{
	gpio_clear(GPIOA, GPIO8);
}

void hw_led_off(void)
{
	gpio_set(GPIOA, GPIO8);
}

void hw_led_toggle(void)
{
	gpio_toggle(GPIOA, GPIO8);
}

void hw_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

    hw_memory_init();

    hw_led_setup();

    hw_gpio_setup();

    /* setup usb: to support console */
    hw_usb_setup();

    hw_systick_setup();

    hw_storage_setup();
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

void hw_poll(void)
{
    hw_usb_poll();
    hw_gpio_poll();
}

void hw_halt(void)
{
    while (1) {
    }
}

