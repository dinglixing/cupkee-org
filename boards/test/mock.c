
#include <stdio.h>
#include <string.h>

#include <bsp.h>
#include "hardware.h"

/*
* bsp mock api
*/
void board_setup(void)
{
    hw_reset();
}

int hal_memory_alloc(void **p, int size, int align)
{
    (void) p;
    (void) size;
    (void) align;

    return 0;
}

void hal_poll(void)
{
}

void hal_halt(void)
{
    printf("\nSystem into halt!\n");
    while(1)
        ;
}

void hal_info_get(hal_info_t * info)
{
    (void) info;

    return;
}

int hal_console_set_cb(void (*input)(void *, int), void (*drain)(void))
{
    hw_console_set_cb(input, drain);
    return 0;
}

int hal_console_write_byte(char c)
{
    return hw_console_putc(c);
}

int hal_console_puts(const char *s)
{
    int n = 0;

    while (s[n]) {
        hw_console_putc(s[n++]);
    }

    return n;
}

int hal_console_write_sync_byte(char c)
{
    return hw_console_putc(c);
}

int hal_console_sync_puts(const char *s)
{
    int n = 0;

    while (s[n]) {
        hw_console_putc(s[n++]);
    }

    return n;
}

void hal_led_on(void)
{}

void hal_led_off(void)
{}

void hal_led_toggle(void)
{}

int hal_scripts_erase(void)
{
    return hw_scripts_erase();
}

int hal_scripts_remove(int id)
{
    return hw_scripts_remove(id);
}

int hal_scripts_save(const char *s)
{
    return hw_scripts_save(s);
}

const char *hal_scripts_load(const char *prev)
{
    return hw_scripts_load(prev);
}

