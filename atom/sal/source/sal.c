#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal.h"
#include "sal.h"


static int console_in_bgn;
static int console_in_end;
static int console_out_bgn;
static int console_out_end;

static char console_out_buf[128];
static char console_in_buf[128];

static int console_ready = 0;

static void console_input_handle(void *data, int len)
{
    char *s = (char *)data;
    int i = 0;


    while (i++ < len && console_in_end < 128) {
        char c = *s++;

        console_in_buf[console_in_end++] = c;
        if (c == '\r') {
            console_in_buf[console_in_end++] = '\n';
            hal_console_out("\n", 1);
        }
    }

    console_ready = 1;
}

static void console_drain_handle(void)
{
    int len = console_out_end - console_out_bgn;

    if (len) {
        int n = hal_console_out(console_out_buf + console_out_bgn, len);
        if (n > 0) {
            if (n == len) {
                console_out_bgn = 0;
                console_out_end = 0;
            } else {
                console_out_bgn += n;
            }
        }
    }
}

static int sal_console_init(void)
{
    return hal_console_set_cb(console_input_handle, console_drain_handle);
}

void sal_console_output(const char *s)
{
    int len = console_out_end - console_out_bgn;

    if (len == 0) {
        while (*s && hal_console_out(s, 1)) {
            s++;
        }
    }

    while (*s && console_out_end < 128) {
        console_out_buf[console_out_end++] = *s++;
    }
}

const char *sal_console_getline(void)
{
    int len = console_in_end - console_in_bgn;

    if (len > 0 && console_in_buf[console_in_end - 1] == '\n') {
        const char *line = console_in_buf + console_in_bgn;
        console_in_buf[console_in_end] = 0;
        console_in_bgn = 0;
        console_in_end = 0;

        hal_led_toggle(HAL_LED_1);
        return line;
    }
    return NULL;
}

int sal_init(void)
{
    //os_init();

    if (sal_console_init()) {
        return -1;
    }

    return 0;
}

void sal_loop(void)
{
}

int sal_console_ready(void) {
    return console_ready;
}

