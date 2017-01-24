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



#ifndef __LANG_HEAP_INC__
#define __LANG_HEAP_INC__

#include "config.h"

typedef struct heap_t {
    int size;
    int free;
    void *base;
} heap_t;

void heap_init(heap_t *heap, void *base, int size);
void heap_clean(heap_t *heap);

void *heap_alloc(heap_t *heap, int size);

static inline
int heap_is_owned(heap_t *heap, void *p) {
    int dis = p - heap->base;
    return dis >= 0 && dis < heap->size;
}

static inline
void heap_reset(heap_t *heap) {
    heap->free = 0;
}

static inline
void heap_copy(heap_t *dst, heap_t *src) {
    dst->base = src->base;
    dst->size = src->size;
    dst->free = src->free;
}

static inline
int heap_free_size(heap_t *heap) {
    return heap->size - heap->free;
}

static inline
void *heap_free_addr(heap_t *heap) {
    return heap->base + heap->free;
}


#endif /* __LANG_HEAP_INC__ */

