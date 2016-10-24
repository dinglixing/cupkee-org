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
#include "usb.h"
#include "storage.h"

/* systick interrupt handle routing  */
uint32_t system_ticks_count = 0;
void sys_tick_handler(void)
{
    system_ticks_count++;
}

static void hal_led_setup(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);

	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO8);

	gpio_set(GPIOA, GPIO8);
}

static void hal_systick_setup(void)
{
    systick_set_frequency(SYSTEM_TICKS_PRE_SEC, 72000000);

    systick_interrupt_enable();
    systick_counter_enable();
}

void hal_info_get(hal_info_t *info)
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

void hal_led_on(void)
{
	gpio_clear(GPIOA, GPIO8);
}

void hal_led_off(void)
{
	gpio_set(GPIOA, GPIO8);
}

void hal_led_toggle(void)
{
	gpio_toggle(GPIOA, GPIO8);
}

void board_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

    hal_led_setup();

    /* setup usb: to support console */
    hal_usb_setup();

    hal_systick_setup();

    hal_storage_setup();
}

extern vector_table_t vector_table;
extern char end;
int hal_memory_alloc(void **p, int size, int align)
{
    static void *heap_free = &end;
    intptr_t mem_pos = (intptr_t)heap_free;
    void *mem;
    int max;

    mem_pos = mem_pos % align;
    if (mem_pos) {
        mem = heap_free + (align - mem_pos);
    } else {
        mem = heap_free;
    }

    max = (char *)(vector_table.initial_sp_value) - (char *)mem;
    if (size > 0) {
        size = size < max ? size : max;
    } else {
        size = max;
    }
    heap_free = mem + size;

    *p = mem;
    return size;
}

void hal_poll(void)
{
    hal_usb_poll();
}

void hal_halt(void)
{
    while (1) {
        int i;
        for (i = 0; i < 0x1000000; i++)
            __asm__("nop");
        hal_led_toggle();
    }
}

