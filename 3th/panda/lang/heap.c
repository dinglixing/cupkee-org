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


#include "heap.h"

void heap_init(heap_t *heap, void *base, int size)
{
    if (heap && base && size) {
        heap->size = size;
        heap->free = 0;
        heap->base = base;
        //memset(base, 0, size);
    }
}

void heap_clean(heap_t *heap)
{
    if (heap) {
        heap->free = 0;
        memset(heap->base, 0, heap->size);
    }
}

void *heap_alloc(heap_t *heap, int size) {
    if (heap) {
        int free;

        size = SIZE_ALIGN(size);
        free = heap->free + size;
        //printf("Alloc %d, size: %u, free: %u\n", size, heap->size, heap->free);
        if (free < heap->size) {
            void *p = heap->base + heap->free;
            heap->free = free;
            return p;
        }
    }
    return NULL;
}

