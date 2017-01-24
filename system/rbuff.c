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

#include <string.h>
#include "rbuff.h"

int rbuff_shift(rbuff_t *rb)
{
    if (rb->cnt <= 0) {
        return -1;
    }

    int pos = rb->head++;
    if (rb->head >= rb->size) {
        rb->head = 0;
    }
    rb->cnt--;

    return pos;
}

int rbuff_unshift(rbuff_t *rb)
{
    if (rb->cnt >= rb->size) {
        return -1;
    }

    if (rb->head == 0) {
        rb->head = rb->size - 1;
    } else {
        rb->head--;
    }
    rb->cnt++;
    return rb->head;
}


int rbuff_push(rbuff_t *rb)
{
    if (rb->cnt >= rb->size) {
        return -1;
    }

    int pos = rb->head + rb->cnt++;
    if (pos >= rb->size) {
        pos %= rb->size;
    }
    return pos;
}

int rbuff_pop(rbuff_t *rb)
{
    if (rb->cnt <= 0) {
        return -1;
    }

    int pos = rb->head + rb->cnt--;
    if (pos >= rb->size) {
        pos %= rb->size;
    }
    return pos;
}

int rbuff_get(rbuff_t *rb, int pos)
{
    if (pos < 0 || pos >= rb->cnt) {
        return -1;
    }
    pos = rb->head + pos;
    if (pos >= rb->size) {
        pos = pos % rb->size;
    }
    return pos;
}

