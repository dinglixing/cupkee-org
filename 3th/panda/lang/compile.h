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


#ifndef __LANG_COMPILE_INC__
#define __LANG_COMPILE_INC__

#include "config.h"

#include "ast.h"
#include "env.h"
#include "interp.h"

typedef struct compile_func_t {
    int16_t owner;
    uint16_t stack_space;
    uint16_t stack_high;

    uint8_t closure;
    uint8_t var_max;
    uint8_t var_num;
    uint8_t arg_num;

    uint16_t code_max;
    uint16_t code_num;
    uint8_t  *code_buf;
    intptr_t *var_map;
} compile_func_t;

typedef struct compile_t {
    int     error;     //

    int16_t bgn_pos;   // for loop continue
    int16_t skip_pos;  // for loop break

    uint16_t func_size;
    uint16_t func_num;
    uint16_t func_cur;
    uint16_t func_offset;

    env_t  *env;
    heap_t  heap;

    compile_func_t *func_buf;
} compile_t;

int compile_init(compile_t *cpl, env_t *env, void *heap_ptr, int heap_size);
int compile_deinit(compile_t *cpl);

/*
int compile_arg_add(compile_t *cpl, intptr_t sym_id);
int compile_var_add(compile_t *cpl, intptr_t sym_id);
int compile_var_get(compile_t *cpl, intptr_t sym_id);
*/

int compile_stmt(compile_t *cpl, stmt_t *stmt);
int compile_one_stmt(compile_t *cpl, stmt_t *stmt);
int compile_multi_stmt(compile_t *cpl, stmt_t *stmt);

int compile_update(compile_t *cpl);

int compile_env_init(env_t *env, void *mem_ptr, int mem_size);
int compile_exe(env_t *env, const char *input, void *mem_ptr, int mem_size);


#endif /* __LANG_COMPILE_INC__ */
