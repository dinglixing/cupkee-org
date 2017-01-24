/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016-2017 Lixing Ding <ding.lixing@gmail.com>

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


#ifndef __RBUFF_INC__
#define __RBUFF_INC__

typedef struct rbuff_t {
    int size;
    int head;
    int  cnt;
} rbuff_t;

static inline void rbuff_init(rbuff_t *rb, int size) {
    rb->size = size;
    rb->head = 0;
    rb->cnt = 0;
}

static inline void rbuff_reset(rbuff_t *rb) {
    rb->head = 0;
    rb->cnt = 0;
}

static inline int _rbuff_get(rbuff_t *rb, int pos)
{
    pos = rb->head + pos;
    if (pos >= rb->size) {
        pos = pos % rb->size;
    }
    return pos;
}

static inline int rbuff_remove(rbuff_t *rb, int n) {
    if (rb->cnt >= n) {
        rb->cnt -= n;
        return n;
    } else {
        return 0;
    }
}

static inline int rbuff_remove_left(rbuff_t *rb, int n) {
    int head;

    if (n < 0 || rb->cnt < n) {
        return 0;
    }

    head = rb->head + n;
    if (head >= rb->size) {
        head = head - rb->size;
    }
    rb->head = head;
    rb->cnt -= n;

    return n;
}

static inline int rbuff_append(rbuff_t *rb, int n) {
    if (rb->cnt + n <= rb->size) {
        rb->cnt += n;
        return n;
    } else {
        return 0;
    }
}

static inline int rbuff_end(rbuff_t *rb) {
    return rb->cnt;
}

static inline int rbuff_is_empty(rbuff_t *rb) {
    return !rb->cnt;
}

static inline int rbuff_is_full(rbuff_t *rb) {
    return rb->cnt == rb->size;
}

static inline int rbuff_has_space(rbuff_t *rb, int space) {
    return rb->size > (rb->cnt + space);
}

int rbuff_shift(rbuff_t *rb);
int rbuff_unshift(rbuff_t *rb);
int rbuff_push(rbuff_t *rb);
int rbuff_pop(rbuff_t *rb);
int rbuff_get(rbuff_t *rb, int pos);

#endif /* __RBUFF_INC__ */

