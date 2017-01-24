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


#ifndef __LANG_ARRAY_INC__
#define __LANG_ARRAY_INC__

#include "config.h"
#include "val.h"
#include "env.h"

#define MAGIC_ARRAY         (MAGIC_BASE + 11)

typedef struct array_t {
    uint8_t magic;
    uint8_t age;
    uint16_t elem_size;
    uint16_t elem_bgn;
    uint16_t elem_end;
    val_t *elems;
} array_t;

static inline int array_mem_space(array_t *a) {
    return SIZE_ALIGN(sizeof(array_t) + sizeof(val_t) * a->elem_size);
}

static inline int array_is_true(val_t *v) {
    array_t *a = (array_t *)val_2_intptr(v);
    return a->elem_end - a->elem_bgn > 0 ? 1 : 0;
}

static inline val_t *array_values(array_t *a) {
    return a->elems + a->elem_bgn;
}

static inline int array_len(array_t *a) {
    return a->elem_end - a->elem_bgn;
}

static inline
val_t *_array_element(val_t *array, int i) {
    array_t *a = (array_t *)val_2_intptr(array);
    return (a->elem_bgn + i < a->elem_end) ? (a->elems + i) : NULL;
}

static inline
val_t *_array_elem(array_t *a, int i) {
    return (a->elem_bgn + i < a->elem_end) ? (a->elems + i) : NULL;
}

array_t *_array_create(env_t *env, int len);
intptr_t array_create(env_t *env, int ac, val_t *av);

val_t array_length(env_t *env, int ac, val_t *av);
val_t array_push(env_t *env, int ac, val_t *av);
val_t array_pop(env_t *env, int ac, val_t *av);
val_t array_shift(env_t *env, int ac, val_t *av);
val_t array_unshift(env_t *env, int ac, val_t *av);
val_t array_foreach(env_t *env, int ac, val_t *av);

void array_elem_val(val_t *self, int i, val_t *elem);
val_t *array_elem_ref(val_t *self, int i);

#endif /* __LANG_ARRAY_INC__ */

