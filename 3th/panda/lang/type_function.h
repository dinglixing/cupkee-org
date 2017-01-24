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


#ifndef __LANG_FUNCTION_INC__
#define __LANG_FUNCTION_INC__

#include "config.h"

#include "val.h"
#include "env.h"
#include "interp.h"

#define MAGIC_FUNCTION  (MAGIC_BASE + 5)

typedef struct function_t {
    uint8_t magic;
    uint8_t age;
    uint8_t reserved[2];
    uint8_t *entry;
    scope_t *super;
} function_t;

typedef val_t (*function_native_t) (env_t *env, int ac, val_t *av);

intptr_t  function_create(env_t *env, uint8_t *code);
int function_destroy(intptr_t func);

static inline
int function_mem_space(function_t *f) {
    (void) f;
    return SIZE_ALIGN(sizeof(function_t));
}

static inline
uint8_t function_varc(function_t *fn) {
    return executable_func_get_var_cnt(fn->entry);
}

static inline
uint8_t function_size(function_t *fn) {
    return executable_func_get_code_size(fn->entry);
}

static inline
uint8_t function_argc(function_t *fn) {
    return executable_func_get_arg_cnt(fn->entry);
}

static inline
uint16_t function_stack_high(function_t *fn) {
    return executable_func_get_stack_high(fn->entry);
}

static inline
int function_is_closure(function_t *fn) {
    return executable_func_is_closure(fn->entry);
}

static inline
uint8_t *function_code(function_t *fn) {
    return fn->entry + FUNC_HEAD_SIZE;
}

#endif /* __LANG_FUNCTION_INC__ */

