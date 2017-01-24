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



#ifndef __LANG_EXECUTABLE_INC__
#define __LANG_EXECUTABLE_INC__

#include "config.h"

#include "val.h"

#define FUNC_HEAD_SIZE 8

#define EXEC_FL_BE     1
#define EXEC_FL_64     2

typedef struct executable_t {
    uint16_t  string_max;
    uint16_t  string_num;

    uint16_t  number_max;
    uint16_t  number_num;

    uint16_t  func_max;
    uint16_t  func_num;

    double   *number_map;
    intptr_t *string_map;
    uint8_t **func_map;

    uint32_t  main_code_end;
    uint32_t  func_code_end;

    uint8_t  *code;
} executable_t;

typedef struct image_info_t {
    int8_t      error;
    uint8_t     addr_size;
    uint8_t     byte_order;
    uint8_t     version;

    uint32_t    size;
    uint32_t    end;

    uint32_t    num_cnt, num_ent;
    uint32_t    str_cnt, str_ent;
    uint32_t    fn_cnt, fn_ent;

    uint8_t    *base;
} image_info_t;


int executable_init(executable_t *exe, void *memory, int size,
                    int number_max, int string_max, int func_max, int code_max);

static inline
void executable_main_clr(executable_t *exe)
{
    exe->main_code_end = 0;
}

int executable_func_set_head(void *buf, uint8_t vc, uint8_t ac, uint32_t code_size, uint16_t stack_size, int closure);
int executable_func_get_head(void *buf, uint8_t *vc, uint8_t *ac, uint32_t *code_size, uint16_t *stack_size, int *closure);
int executable_main_add(executable_t *exe, void *code, uint16_t size, uint8_t vc, uint8_t ac, uint16_t stack_size, int closure);
int executable_func_add(executable_t *exe, void *code, uint16_t size, uint8_t vc, uint8_t ac, uint16_t stack_size, int closure);

static inline
uint8_t executable_func_get_var_cnt(const uint8_t *entry) {
    return entry[0];
}

static inline
uint8_t executable_func_get_arg_cnt(const uint8_t *entry) {
    return entry[1];
}

static inline
uint16_t executable_func_get_stack_high(const uint8_t *entry) {
    return (entry[2] * 0x100 + entry[3]) & 0x7FFF;
}

static inline
uint32_t executable_func_get_code_size(const uint8_t *entry) {
    return (entry[4] * 0x1000000 + entry[5] * 0x10000 + entry[6] * 0x100 + entry[7]);
}

static inline
const uint8_t *executable_func_get_code(const uint8_t *entry) {
    return entry + FUNC_HEAD_SIZE;
}

static inline
int executable_func_is_closure(const uint8_t *entry) {
    return (entry[2] & 0x80) == 0x80;
}

int executable_number_find_add(executable_t *exe, double n);
int executable_string_find_add(executable_t *exe, intptr_t s);

int image_init(image_info_t *img, void *mem_ptr, int mem_size, int byte_order, int nc, int sc, int fc);
int image_load(image_info_t *img, uint8_t *input, int size);
int image_fill_data(image_info_t *img, unsigned int nc, double *nv, unsigned int sc, intptr_t *sv);
int image_fill_code(image_info_t *img, unsigned int entry, uint8_t vc, uint8_t ac, uint16_t stack_need, int closure, uint8_t *code, unsigned int size);
double *image_number_entry(image_info_t *img);
double image_get_number(image_info_t *img, int index);
const char *image_get_string(image_info_t *img, int index);
const uint8_t *image_get_function(image_info_t *img, int index);

static inline int image_size(image_info_t *img) {
    return img->end;
}

#endif /* __LANG_EXECUTABLE_INC__ */

