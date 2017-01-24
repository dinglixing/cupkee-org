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

#include "err.h"
#include "type_string.h"
#include "type_buffer.h"

static inline type_buffer_t *buffer_alloc(env_t *env, int size)
{
    type_buffer_t *b = env_heap_alloc(env, SIZE_ALIGN(sizeof(type_buffer_t) + size));

    if (b) {
        b->magic = MAGIC_BUFFER;
        b->age = 0;
        b->len = size;
    }
    return b;
}

type_buffer_t *buffer_create(env_t *env, int size)
{
    return buffer_alloc(env, size);
}

type_buffer_t *buffer_slice(env_t *env, type_buffer_t *b, int start, int size)
{
    type_buffer_t *slice;

    if (start < 0) {
        start = 0;
    }

    if (start + size > b->len) {
        size = b->len - start;
        if (size < 0) {
            size = 0;
        }
    }

    slice = buffer_alloc(env, size);
    if (slice) {
        memcpy(slice->buf, b->buf + start, size);
    } else {
        env_set_error(env, ERR_NotEnoughMemory);
    }
    return slice;
}

int buffer_read_int(type_buffer_t *b, int off, int size, int be, int *v)
{
    if (off >= 0 && size > 0 && off + size <= b->len) {
        int num = 0, i;
        int8_t s;

        if (be) {
            s = b->buf[off];
            num = s;
            for (i = 1; i < size; i++) {
                num <<= 8;
                num |= b->buf[off + i];
            }
        } else {
            s = b->buf[off + size - 1];
            num = s;
            for (i = size - 2; i > -1; i--) {
                num <<= 8;
                num |= b->buf[off + i];
            }
        }
        *v = num;
        return off + size;
    } else {
        return 0;
    }
}

int buffer_write_int(type_buffer_t *b, int off, int size, int be, int num)
{
    if (off >= 0 && size > 0 && off + size <= b->len) {
        int i;

        if (be) {
            b->buf[off + size - 1] = num;
            for (i = size - 2; i > -1; i--) {
                num >>= 8;
                b->buf[off + i] = num;
            }
        } else {
            b->buf[off] = num;
            for (i = 1; i < size; i++) {
                num >>= 8;
                b->buf[off + i] = num;
            }
        }

        return off + size;
    } else {
        return 0;
    }
}

val_t buffer_native_create(env_t *env, int ac, val_t *av)
{
    int size = 8; // define a default size
    void *data = NULL;
    type_buffer_t *buf;

    if (ac > 0) {
        if (val_is_number(av)) {
            size = val_2_integer(av);
        } else {
            data = (void *)val_2_cstring(av);
            if (data) {
                size = strlen(data);
            }
        }
    }

    buf = buffer_alloc(env, size);
    if (buf) {
        if (data) {
            memcpy(buf->buf, data, size);
        }
        return val_mk_buffer(buf);
    } else {
        env_set_error(env, ERR_NotEnoughMemory);
        return VAL_UNDEFINED;
    }
}

static void param_parse(int ac, val_t *av, int *off, int *size, int *be)
{
    *off = 0;
    *size = 1;
    *be = 0;

    if (ac > 0 && val_is_number(av)) {
        *off = val_2_integer(av);
        if (ac > 1 && val_is_number(av + 1)) {
            *size = val_2_integer(av + 1);
            if (*size > 1 && ac > 2 && val_is_true(av + 2)) {
                *be = 1;
            }
        }
    }
}

val_t buffer_native_read_int(env_t *env, int ac, val_t *av)
{
    type_buffer_t *buf = NULL;
    int off = 0, size = 1, be = 0, num;

    (void) env;

    if (ac < 1 || !val_is_buffer(av)) {
        return VAL_UNDEFINED;
    }
    buf = (type_buffer_t *) val_2_intptr(av);

    param_parse(ac - 1, av + 1, &off, &size, &be);
    if (!buffer_read_int(buf, off, size, be, &num)) {
        return VAL_UNDEFINED;
    } else {
        return val_mk_number(num);
    }
}

val_t buffer_native_write_int(env_t *env, int ac, val_t *av)
{
    type_buffer_t *buf = NULL;
    int off = 0, size = 1, be = 0, num;

    (void) env;

    if (ac < 2 || !val_is_buffer(av) || !val_is_number(av + 1)) {
        return val_mk_number(0);
    }

    buf = (type_buffer_t *) val_2_intptr(av);
    num = val_2_integer(av + 1);

    param_parse(ac - 2, av + 2, &off, &size, &be);
    if ((off = buffer_write_int(buf, off, size, be, num)) > 0) {
        return val_mk_number(off);
    } else {
        return val_mk_number(0);
    }
}

val_t buffer_native_to_string(env_t *env, int ac, val_t *av)
{
    type_buffer_t *buf;
    uint8_t *ptr;
    void    *str;
    val_t s;
    int len;

    if (ac < 1 || !val_is_buffer(av)) {
        env_set_error(env, ERR_InvalidInput);
        return VAL_UNDEFINED;
    }

    buf = (type_buffer_t *)val_2_intptr(av);
    for (len = 0; len < buf->len; len++) {
        uint8_t ch = buf->buf[len];
        if (ch < ' ' || ch >= 127) {
            break;
        }
    }

    s = string_create_heap_val(env, len);
    str = (void *)val_2_cstring(&s);
    if (str && len) {
        // defence GC
        buf = (type_buffer_t *)val_2_intptr(av);
        ptr = buf->buf;

        memcpy(str, ptr, len);
    }

    return s;
}

val_t buffer_native_slice(env_t *env, int ac, val_t *av)
{
    type_buffer_t *buf, *slice;
    int start = 0, end = -1;

    if (ac < 1 || !val_is_buffer(av)) {
        env_set_error(env, ERR_InvalidInput);
        return VAL_UNDEFINED;
    }

    buf = (type_buffer_t *)val_2_intptr(av);
    if (ac > 1 && val_is_number(av + 1)) {
        start = val_2_integer(av + 1);
        if (ac > 2 && val_is_number(av + 2)) {
            end = val_2_integer(av + 2);
        }
    }

    if (end < 0) {
        end = buf->len;
    }

    if (start > end) {
        end = start;
    }

    slice = buffer_slice(env, buf, start, end - start);
    if (slice) {
        return val_mk_buffer(slice);
    } else {
        return VAL_UNDEFINED;
    }
}

void buffer_elem_get(val_t *self, int index, val_t *elem)
{
    type_buffer_t *buf;

    buf = (type_buffer_t *)val_2_intptr(self);
    if (index >= 0 && index < buf->len) {
        val_set_number(elem, buf->buf[index]);
    } else {
        val_set_undefined(elem);
    }
}

