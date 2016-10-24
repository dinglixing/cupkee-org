
#include <stdio.h>
#include <string.h>

#include <bsp.h>
#include "mock.h"


#define MEMORY_SIZE         (64 * 1024)


uint32_t system_ticks_count = 0;

#define CONSOLE_BUF_SIZE    (1024 * 8)

static char console_buf[CONSOLE_BUF_SIZE];
static int  console_end = 0;
static void (*console_input_cb)(void *, int);
static void (*console_drain_cb)(void);

void hw_mock_reset(void)
{
    system_ticks_count = 0;
    console_end = 0;
}

void hw_mock_systicks_set(uint32_t x)
{
    system_ticks_count = x;
}

void hw_mock_console_buf_reset(void)
{
    console_end = 0;
}

int hw_mock_console_reply(char **ptr)
{
    if (ptr) {
        *ptr = console_buf;
    }

    console_buf[console_end] = 0;
    return console_end;
}

void hw_mock_console_give(const char *data)
{
    char *in = (char *)data;
    console_input_cb(in, strlen(in));
}

/*
* bsp mock api
*/
void board_setup(void)
{
    hw_mock_reset();
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
    console_input_cb = input;
    console_drain_cb = drain;

    return 0;
}

int hal_console_write_byte(char c)
{
    console_buf[console_end++] = c;
    (void) c;
    return 1;
}

int hal_console_puts(const char *s)
{
    int n = 0;

    while (s[n]) {
        console_buf[console_end++] = s[n++];
    }

    return n;
}

int hal_console_write_sync_byte(char c)
{
    return hal_console_write_byte(c);
}

int hal_console_sync_puts(const char *s)
{
    return hal_console_puts(s);
}

void hal_led_on(void)
{}

void hal_led_off(void)
{}

void hal_led_toggle(void)
{}



int hal_scripts_erase(void)
{
    return -1;
}
int hal_scripts_remove(int id)
{
    return -1;
}
int hal_scripts_save(const char *s)
{
    return -1;
}

const char *hal_scripts_load(const char *prev)
{
    return NULL;
}

