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

#include "util.h"


/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/
#define CK_BUF_NUM              4
#define CK_BUF_SIZE             32

typedef struct cupkee_buf_t {
    uint16_t cap;
    uint16_t bgn;
    uint16_t len;
    uint8_t  ptr[CK_BUF_SIZE];
} cupkee_buf_t;

static uint32_t     cupkee_buffer_inused = 0;
static cupkee_buf_t cupkee_buffers[CK_BUF_NUM];

static void util_setup_buf(void)
{
    cupkee_buffer_inused = 0;
}

static inline void _cupkee_buf_reset(cupkee_buf_t *b) {
    b->cap = CK_BUF_SIZE;
    b->len = 0;
    b->bgn = 0;
}

void cupkee_buf_reset(void *p)
{
    cupkee_buf_t *b = (cupkee_buf_t *)p;

    _cupkee_buf_reset(b);
}

void *cupkee_buf_alloc(void)
{
    int i;

    for (i = 0; i < CK_BUF_NUM; i++) {
        if (!(cupkee_buffer_inused & (1 << i))) {
            cupkee_buf_t *b = cupkee_buffers + i;

            _cupkee_buf_reset(b);

            cupkee_buffer_inused |= (1 << i);

            return b;
        }
    }
    return NULL;
}

void cupkee_buf_release(void *p)
{
    cupkee_buf_t *b = (cupkee_buf_t *)p;
    int i = b - cupkee_buffers;

    if (i >= 0 && i < CK_BUF_NUM) {
        cupkee_buffer_inused &= ~(1 << i);
    }
}

int cupkee_buf_is_empty(void *p)
{
    cupkee_buf_t *b = (cupkee_buf_t *)p;

    return b->len == 0;
}

int cupkee_buf_is_full(void *p)
{
    cupkee_buf_t *b = (cupkee_buf_t *)p;

    return b->len == b->cap;
}

int cupkee_buf_capacity(void *p)
{
    cupkee_buf_t *b = (cupkee_buf_t *)p;

    return b->cap;
}

int cupkee_buf_space(void *p)
{
    cupkee_buf_t *b = (cupkee_buf_t *)p;

    return b->cap - b->len;
}

int cupkee_buf_length(void *p)
{
    cupkee_buf_t *b = (cupkee_buf_t *)p;

    return b->len;
}

int cupkee_buf_push(void *p, uint8_t d)
{
    cupkee_buf_t *b = (cupkee_buf_t *)p;

    if (b->len < b->cap) {
        int tail = b->bgn + b->len++;
        if (tail >= b->cap) {
            tail -= b->cap;
        }
        b->ptr[tail] = d;
        return 1;
    } else {
        return 0;
    }
}

int cupkee_buf_shift(void *p, uint8_t *d)
{
    cupkee_buf_t *b = (cupkee_buf_t *)p;

    if (b->len) {
        *d = b->ptr[b->bgn++];
        if (b->bgn >= b->cap) {
            b->bgn = 0;
        }
        b->len--;
        return 1;
    }
    return 0;
}

int cupkee_buf_take(void *p, int n, void *buf)
{
    cupkee_buf_t *b = (cupkee_buf_t *)p;

    if (n > b->len) {
        n = b->len;
    }

    if (n) {
        int tail = b->bgn + n;
        int size = n;

        if (tail > b->cap) {
            tail -= b->cap;
            size -= tail;
            memcpy(buf + size, b->ptr, tail);
        } else
        if (tail == b->cap) {
            tail = 0;
        }

        memcpy(buf, b->ptr + b->bgn, size);
        b->bgn = tail;
        b->len -= n;
    }

    return n;
}

int cupkee_buf_give(void *p, int n, void *buf)
{
    cupkee_buf_t *b = (cupkee_buf_t *)p;

    if (n + b->len > b->cap) {
        n = b->cap - b->len;
    }

    if (n) {
        int head = b->bgn + b->len;
        int size = n;

        if (head >= b->cap) {
            head -= b->cap;
        } else
        if (head + n > b->cap) {
            int wrap = head + n - b->cap;

            size -= wrap;
            memcpy(b->ptr, buf + size, wrap);
        }
        memcpy(b->ptr + head, buf, size);

        b->len += n;
    }

    return n;
}


/******************************************************************************/
/* Private                                                                     */
/******************************************************************************/
void cupkee_do_callback(env_t *env, val_t *cb, uint8_t ac, val_t *av)
{
    if (!cb) return;

    if (val_is_native(cb)) {
        function_native_t fn = (function_native_t) val_2_intptr(cb);
        fn(env, ac, av);
    } else
    if (val_is_script(cb)){
        if (ac) {
            int i;
            for (i = ac - 1; i >= 0; i--)
                env_push_call_argument(env, av + i);
        }
        env_push_call_function(env, cb);
        interp_execute_call(env, ac);
    }
}

val_t cupkee_error(env_t *env, int code)
{
    (void) env;

    return val_mk_number(code);
}

void cupkee_do_callback_error(env_t *env, val_t *cb, int code)
{
    val_t err = cupkee_error(env, code);

    cupkee_do_callback(env, cb, 1, &err);
}

int cupkee_id(val_t *in, int max, const char **names)
{
    if (val_is_number(in)) {
        return val_2_integer(in);
    } else {
        const char *str = val_2_cstring(in);
        if (str) {
            int id;
            for (id = 0; id < max && names[id]; id++) {
                if (!strcmp(str, names[id])) {
                    return id;
                }
            }
        }
    }
    return max;
}

int utils_init(void)
{
    util_setup_buf();

    return 0;
}

