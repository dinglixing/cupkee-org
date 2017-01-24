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

#ifndef __LANG_TYPE_BUFFER_INC__
#define __LANG_TYPE_BUFFER_INC__

#include "config.h"

#include "val.h"
#include "env.h"

#define MAGIC_BUFFER        (MAGIC_BASE + 13)
typedef struct type_buffer_t {
    uint8_t  magic;
    uint8_t  age;
    uint16_t len;
    uint8_t  buf[0];
} type_buffer_t;

static inline
int buffer_mem_space(type_buffer_t *buf) {
    return SIZE_ALIGN(sizeof(type_buffer_t) + buf->len);
}

type_buffer_t *buffer_create(env_t *env, int size);
type_buffer_t *buffer_slice(env_t *env, type_buffer_t *b, int start, int size);
int buffer_read_int(type_buffer_t *b, int off, int size, int be, int *v);
int buffer_write_int(type_buffer_t *b, int off, int size, int be, int num);

static inline
int   _buffer_size(type_buffer_t *b) {return b->len;}
static inline
void *_buffer_addr(type_buffer_t *b) {return b->buf;}

val_t buffer_native_create(env_t *env, int ac, val_t *av);
val_t buffer_native_write_int(env_t *env, int ac, val_t *av);
val_t buffer_native_write_uint(env_t *env, int ac, val_t *av);
val_t buffer_native_read_int(env_t *env, int ac, val_t *av);
val_t buffer_native_read_uint(env_t *env, int ac, val_t *av);
val_t buffer_native_slice(env_t *env, int ac, val_t *av);
val_t buffer_native_to_string(env_t *env, int ac, val_t *av);

void buffer_elem_get(val_t *self, int index, val_t *elem);

static inline
void *_val_buffer_addr(val_t *v) {
    return _buffer_addr((type_buffer_t *) val_2_intptr(v));
}

static inline
int _val_buffer_size(val_t *v) {
    return _buffer_size((type_buffer_t *) val_2_intptr(v));
}

#endif /* __LANG_TYPE_BUFFER_INC__ */

