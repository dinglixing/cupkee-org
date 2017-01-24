/*
MIT License

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

#include <readline/readline.h>
#include <readline/history.h>

#include "example.h"

#define HEAP_SIZE     (1024 * 400)
#define STACK_SIZE    (1024)
#define EXE_MEM_SPACE (1024 * 100)
#define SYM_MEM_SPACE (1024 * 4)
#define MEM_SIZE      (STACK_SIZE * sizeof(val_t) + HEAP_SIZE + EXE_MEM_SPACE + SYM_MEM_SPACE)
#define INPUT_MAX     (1024)

#define MODE_LINE     0
#define MODE_MULTI    1

static uint8_t memory[MEM_SIZE];
static int  input_cacahed = 0;
static int  input_mode = MODE_LINE;
static char input_buf[INPUT_MAX];
static int  done = 0;

static const char *logo = "\
  ___                 _       \n\
 | _ \\ __ _  _ _   __| | __ _ \n\
 |  _// _` || ' \\ / _` |/ _` |\n\
 |_|  \\__,_||_||_|\\__,_|\\__,_|\n\
 www.cupkee.cn\n";

static void input_read(char *buf, int max)
{
    char *input;

    if (input_mode == MODE_MULTI) {
        input = readline(". ");
    } else {
        input = readline("> ");
    }

    if (!input) {
        done = 1;
        return;
    }

    if (strlen(input)) {
        strncpy(buf, input, max);
        add_history(input);
    } else {
        buf[0] = 0;
    }

    free(input);
}

static char *input_more(void)
{
    input_mode = MODE_MULTI;
    return NULL;
}

static void print_error(int error)
{
    switch (error) {
    case ERR_NotEnoughMemory:   output("Error: Not enought memory\n");      break;
    case ERR_NotImplemented:    output("Error: Not implemented\n");         break;
    case ERR_StackOverflow:     output("Error: Stack overflow\n");          break;
    case ERR_ResourceOutLimit:  output("Error: Resource out of limited\n"); break;

    case ERR_InvalidToken:      output("Error: Invalid Token\n");           break;
    case ERR_InvalidSyntax:     output("Error: Invalid syntax\n");          break;
    case ERR_InvalidLeftValue:  output("Error: Invalid left value\n");      break;
    case ERR_InvalidSementic:   output("Error: Invalid Sementic\n");        break;

    case ERR_InvalidByteCode:   output("Error: Invalid Byte code\n");       break;
    case ERR_InvalidInput:      output("Error: Invalid input\n");           break;
    case ERR_InvalidCallor:     output("Error: Invalid callor\n");          break;
    case ERR_NotDefinedId:      output("Error: Not defined id\n");          break;

    case ERR_SysError:          output("Error: System error\n");            break;

    default: output("Error: unknown error\n");
    }
}

static void print_value(val_t *v)
{
    if (val_is_number(v)) {
        char buf[32];
        if (*v & 0xffff) {
            snprintf(buf, 32, "%f\n", val_2_double(v));
        } else {
            snprintf(buf, 32, "%d\n", (int)val_2_double(v));
        }
        output(buf);
    } else
    if (val_is_boolean(v)) {
        output(val_2_intptr(v) ? "true\n" : "false\n");
    } else
    if (val_is_string(v)) {
        output("\"");
        output(val_2_cstring(v));
        output("\"\n");
    } else
    if (val_is_undefined(v)) {
        output("undefined\n");
    } else
    if (val_is_nan(v)) {
        output("NaN\n");
    } else
    if (val_is_function(v)) {
        output("function\n");
    } else {
        output("object\n");
    }
}

static env_t env;

static void line_proc(void)
{
    int    err;
    val_t *res;

    input_read(input_buf, INPUT_MAX);

    if (done) {
        return;
    }

    err = interp_execute_interactive(&env, input_buf, input_more, &res);
    if (err < 0) {
        if (input_mode == MODE_LINE) {
            print_error(-err);
        } else {
            input_cacahed = strlen(input_buf);
        }
        env_set_error(&env, 0);
    } else {
        input_mode = MODE_LINE;
        if (err > 0) {
            print_value(res);
        }
    }
}

static void mult_proc(void)
{
    int    err, len;
    val_t *res;

    input_read(input_buf + input_cacahed, INPUT_MAX - input_cacahed);
    if (done) {
        return;
    }

    len = strlen(input_buf + input_cacahed);
    if (len > 0) {
        input_cacahed += len;
        return;
    }

    err = interp_execute_string(&env, input_buf, &res);
    if (err < 0) {
        print_error(-err);
        env_set_error(&env, 0);
    } else
    if (err > 0) {
        print_value(res);
    }
    input_mode = MODE_LINE;
}

static int interactive(void *mem_ptr, int mem_size, int heap_size, int stack_size)
{
    if(0 != interp_env_init_interactive(&env, mem_ptr, mem_size, NULL, heap_size, NULL, stack_size)) {
        output("env_init fail\n");
        return -1;
    }

    native_init(&env);

    printf(logo, 0, 1, 0);

    while (!done) {
        if (input_mode == MODE_MULTI) {
            mult_proc();
        } else {
            line_proc();
        }
    }

    return 0;
}

int main(int ac, char **av)
{
    return interactive(memory, MEM_SIZE, HEAP_SIZE, STACK_SIZE);
}

