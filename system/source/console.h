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


#ifndef __CONSOLE_INC__
#define __CONSOLE_INC__

#define CON_IN              0
#define CON_OUT             1
#define CON_BUFF_MAX        2
#define CON_BUFF_SIZE       512
#define CON_PREVENT_DEFAULT 1

typedef enum console_ctrl_t{
    CON_CTRL_IDLE = 0,
    CON_CTRL_CHAR = 1,
    CON_CTRL_BACKSPACE,
    CON_CTRL_DELETE,
    CON_CTRL_TABLE,
    CON_CTRL_ENTER,
    CON_CTRL_ESCAPE,
    CON_CTRL_UP,
    CON_CTRL_DOWN,
    CON_CTRL_RIGHT,
    CON_CTRL_LEFT,
    CON_CTRL_F1,
    CON_CTRL_F2,
    CON_CTRL_F3,
    CON_CTRL_F4
} console_ctrl_t;


int console_init(void);
int console_handle_register(int (*handle)(int));

int console_put(char c);
int console_get(void);

int console_puts(const char *s);
int console_gets(char *buf, int max);

int  console_input_curr_tok(char *buf, int size);
int  console_input_peek(int pos);
void console_input_clear(void);
void console_input_string(char *s);

#endif /* __CONSOLE_INC__ */

