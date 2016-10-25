
#include <bsp.h>
#include "hardware.h"

#define CONSOLE_BUF_SIZE    (1 * 1024)

static char console_buf[CONSOLE_BUF_SIZE];
static int  console_end = 0;
static void (*console_input_cb)(void *, int);
static void (*console_drain_cb)(void);

void hw_console_buf_clear(void)
{
    console_end = 0;
}

void hw_console_reset(void)
{
    memset(console_buf, 0, CONSOLE_BUF_SIZE);
    console_end = 0;
    console_input_cb = NULL;
    console_drain_cb = NULL;
}

int hw_console_reply(char **ptr)
{
    if (ptr) {
        *ptr = console_buf;
    }
    console_buf[console_end] = 0;

    return console_end;
}

void hw_console_give(const char *data)
{
    char *in = (char *)data;
    if (console_input_cb) {
        console_input_cb(in, strlen(in));
    } else {
        printf("Hi, console input callback not register!!!\n");
    }
}

void hw_console_set_cb(void (*input)(void *, int), void (*drain)(void))
{
    console_input_cb = input;
    console_drain_cb = drain;
}

int hw_console_putc(int ch)
{
    if (console_end < CONSOLE_BUF_SIZE) {
        console_buf[console_end++] = ch;
        return 1;
    } else {
        return 0;
    }
}

