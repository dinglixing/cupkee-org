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


/*******************************************************************************
 * hw field
*******************************************************************************/
#include "hardware.h"

/*******************************************************************************
 * dbg field
*******************************************************************************/
#define CONSOLE_BUF_SIZE    (1 * 1024)

static char console_buf[CONSOLE_BUF_SIZE];
static int  console_end = 0;
static void (*console_input_cb)(void *, int);
static void (*console_drain_cb)(void);

void hw_dbg_console_clr_buf(void)
{
    console_end = 0;
}

void hw_dbg_console_reset(void)
{
    memset(console_buf, 0, CONSOLE_BUF_SIZE);
    console_end = 0;
    console_input_cb = NULL;
    console_drain_cb = NULL;
}

int hw_dbg_console_get_reply(char **ptr)
{
    if (ptr) {
        *ptr = console_buf;
    }
    console_buf[console_end] = 0;

    return console_end;
}

void hw_dbg_console_set_input(const char *data)
{
    char *in = (char *)data;
    if (console_input_cb) {
        console_input_cb(in, strlen(in));
    } else {
        printf("Hi, console input callback not register!!!\n");
    }
}

/*******************************************************************************
 * bsp interface
*******************************************************************************/
int hw_console_set_callback(void (*input)(void *, int), void (*drain)(void))
{
    console_input_cb = input;
    console_drain_cb = drain;
    return 0;
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

int hw_console_puts(const char *s)
{
    int n = 0;

    while (s[n]) {
        hw_console_putc(s[n++]);
    }

    return n;
}

int hw_console_sync_putc(int ch)
{
    return hw_console_putc(ch);
}

int hw_console_sync_puts(const char *s)
{
    int n = 0;

    while (s[n]) {
        hw_console_putc(s[n++]);
    }

    return n;
}

