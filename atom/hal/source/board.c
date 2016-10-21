#include <stdlib.h>
#include <string.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>

#include "hal.h"
#include "usb.h"

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


static int  _storage_sec_size;
static int  _storage_usr_size;
static char *_storage_usr_base;
void *hal_storage_base_usr(void)
{
    return _storage_usr_base;
}

int hal_storage_size_usr(void)
{
    return _storage_usr_size;
}

int hal_storage_valid_usr(const void *addr)
{
    int dis = (intptr_t)addr - (intptr_t)_storage_usr_base;

    return dis >= 0 && dis < _storage_usr_size && (dis % 4) == 0;
}

int hal_storage_erase_usr(void)
{
    int off;

    flash_unlock();
    for (off = 0; off < _storage_usr_size; off += _storage_sec_size) {
        flash_erase_page((uint32_t)(_storage_usr_base + off));
        if (flash_get_status_flags() != FLASH_SR_EOP) {
            return -1;
        }
    }
    flash_lock();

    return 0;
}

int hal_storage_clear_usr(const void *addr, int size)
{
    if (!hal_storage_valid_usr(addr)) {
        return -1;
    }

    if (size % 4) {
        size += 4 - size % 4;
    }

    uint32_t v = 0;
    int off;

    flash_unlock();
    for (off = 0; off < size; off += 4) {
        flash_program_word((uint32_t)addr + off, v);
    }
    flash_lock();

    for (off = 0; off < size; off += 4) {
        if (*((uint32_t*)(addr + off)) != v) {
            return -1;
        }
    }

    return 0;
}

int hal_storage_write_usr(const void *data, int size)
{
    static uint8_t *last = 0;
    uint8_t *bgn = last ? last : (uint8_t *)_storage_usr_base;
    uint8_t *end = (uint8_t *)_storage_usr_base + _storage_usr_size;
    int head, tail, off;

    if (!hal_storage_valid_usr(bgn)) {
        return -1;
    }

    if (size < 1) {
        return 0;
    }

    while(bgn < end && *((uint32_t *)bgn) != 0xffffffff) {
        bgn += 4;
    }
    if (size > end - bgn) {
        return -1;
    }

    tail = size % 4;
    head = size - tail;

    flash_unlock();
    for (off = 0; off < head; off += 4) {
        flash_program_word((uint32_t)(bgn + off), *((uint32_t *)(data + off)));
    }

    if (tail) {
        uint32_t tail_val = 0;

        memcpy(&tail_val, data + off, tail);
        flash_program_word((uint32_t)(bgn + off), tail_val);
        off += 4;
    }
    flash_lock();

    if (!memcmp(bgn, data, size)) {
        last = bgn + off;
        return 0;
    } else {
        return -1;
    }
}

static void hal_storage_setup(void)
{
    int rom = desig_get_flash_size();

    if (rom >= 256) {
        _storage_sec_size = 2 * 1024;
        _storage_usr_size = 8 * 1024;
    } else {
        _storage_sec_size = 1 * 1024;
        _storage_usr_size = 8 * 1024;
    }
    rom *= 1024;

    _storage_usr_base = (char *) (0x08000000 + (rom - _storage_usr_size));
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

void hal_loop(void)
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

