/*
MIT License

This file is part of cupkee project.

Copyright (c) 2017 Lixing Ding <ding.lixing@gmail.com>

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

#include <cupkee.h>

#define COMMAND_ARG_MAX      8

static short command_num;
static short command_buf_size;
static char *command_buf;
static cupkee_command_entry_t *command_entrys;
static char *command_args[COMMAND_ARG_MAX];

static int command_parse(char *input)
{
    int i;
    char *word, *p;
    const char *sep = " \t\r\n";

    for (word = strtok_r(input, sep, &p), i = 0;
         word && i < COMMAND_ARG_MAX;
         word = strtok_r(NULL, sep, &p)) {

        command_args[i++] = word;
    }

    return i;
}


static void command_do(int ac, char **av)
{
    int i;

    if (command_num < 1 || !command_entrys) {
        return;
    }

    for (i = 0; i < command_num; i++) {
        if (!strcmp(av[0], command_entrys[i].name)) {
            if (command_entrys[i].handle) {
                command_entrys[i].handle(ac, av);
            }
            break;
        }
    }
}

static int command_complete(void)
{
    void *ctx;

    // Use command_buf as auto_complete buffer
    if (!command_buf) {
        return CON_EXECUTE_DEF;
    }
    ctx = cupkee_auto_complete_init(command_buf, command_buf_size);
    if (ctx) {
        int i;

        for (i = 0; command_entrys && i < command_num; i++) {
            cupkee_auto_complete_update(ctx, command_entrys[i].name);
        }
        cupkee_auto_complete_finish(ctx);
    }
    return CON_EXECUTE_DEF;
}

int cupkee_command_init(int n, cupkee_command_entry_t *entrys, int buf_size, char *buf)
{
    command_buf = buf;
    command_buf_size = buf_size;
    command_num = n;
    command_entrys = entrys;

    return 0;
}

int cupkee_command_handle(int type, int ch)
{
    (void) ch;

    if (type == CON_CTRL_ENTER) {
        int len = console_input_load(command_buf_size, command_buf);
        int argc;

        if (len < 1) {
            return CON_EXECUTE_DEF;
        }
        command_buf[len] = 0;
        cupkee_history_push(len, command_buf);

        argc = command_parse(command_buf);
        if (argc) {
            command_do(argc, command_args);
        }
    } else
    if (type == CON_CTRL_TABLE) {
        return command_complete();
    } else
    if (type == CON_CTRL_UP) {
        return cupkee_history_load(-1);
    } else
    if (type == CON_CTRL_DOWN) {
        return cupkee_history_load(1);
    }

    return CON_EXECUTE_DEF;
}

