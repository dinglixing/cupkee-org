/*
MIT License

This file is part of cupkee project.

Copyright (c) 2017 Lixing Ding <ding.lixing@gmail.com>

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

#include "cupkee.h"

#define MEM_BLK_SMALL       0
#define MEM_BLK_NORMAL      1
#define MEM_BLK_HUGE        2
#define MEM_BLK_N           3

#define MEM_BLK_SZ_SMALL    32
#define MEM_BLK_SZ_NORMAL   64

#define BLK_SIZE_ALIGN(s)   (((s) + 31) & ~31)

#define member_offset(T, m) (intptr_t)(&(((T *)0)->m))
#define container_of(p, T, m) ((T*)((intptr_t)(p) - member_offset(T, m)))

#define BLK_REF_INC(b)      ((b)->ref += 2)
#define BLK_REF_DEC(b)      do {if ((b)->ref > 1) (b)->ref -= 2;} while (0)

typedef struct memory_pool_t {
    struct memory_pool_t *next;
    size_t size;
    size_t free;
    void  *base;
} memory_pool_t;

typedef struct memory_block_t {
    uint16_t tag;
    uint16_t ref;
    struct memory_block_t *next;
} memory_block_t;

static memory_pool_t *pool_head = NULL;
static memory_block_t *block_head[MEM_BLK_N];

static void memory_pool_add(size_t bytes, void *ptr)
{
    memory_pool_t *pool = ptr;

    if (bytes < sizeof(memory_pool_t)) {
        return;
    }

    pool->base = ptr + sizeof(memory_pool_t);
    pool->size = bytes - sizeof(memory_pool_t);
    pool->free = 0;

    pool->next = pool_head;
    pool_head  = pool;
}

static void *memory_pool_alloc(size_t tag)
{
    memory_pool_t *p = pool_head;
    size_t size = MEM_BLK_SZ_SMALL * (tag + 1) + sizeof(memory_block_t);

    while (p) {
        if (p->size - p->free > size) {
            memory_block_t *b = (memory_block_t *)(p->base + p->free);
            p->free += size;
            b->tag = tag;
            b->ref = 2;
            return &b->next;
        }
        p = p->next;
    }
    return NULL;
}

void cupkee_memory_setup(size_t bytes, void *ptr)
{
    block_head[MEM_BLK_SMALL]  = NULL;
    block_head[MEM_BLK_NORMAL] = NULL;
    block_head[MEM_BLK_HUGE]   = NULL;

    memory_pool_add(bytes, ptr);
}

void *cupkee_alloc(size_t n)
{
    size_t tag = (BLK_SIZE_ALIGN(n) >> 5) - 1;
    memory_block_t *b, *p;

    if (tag == MEM_BLK_SMALL && block_head[MEM_BLK_SMALL]) {
        b = block_head[MEM_BLK_SMALL];
        block_head[MEM_BLK_SMALL] = b->next;
        BLK_REF_INC(b);
        return &b->next;
    }
    if (tag <= MEM_BLK_NORMAL && block_head[MEM_BLK_NORMAL]) {
        b = block_head[MEM_BLK_NORMAL];
        block_head[MEM_BLK_NORMAL] = b->next;
        BLK_REF_INC(b);
        return &b->next;
    }

    p = NULL;
    b = block_head[MEM_BLK_HUGE];
    while (b) {
        if (b->tag >= tag) {
            if (p) {
                p->next = b->next;
            } else {
                block_head[MEM_BLK_HUGE] = b->next;
            }
            BLK_REF_INC(b);
            return &(b->next);
        }
        p = b;
        b = b->next;
    }

    return memory_pool_alloc(tag);
}

void cupkee_free(void *p)
{
    memory_block_t *b = container_of(p, memory_block_t, next);

    BLK_REF_DEC(b);

    if (b->ref == 0) {
        if (b->tag > MEM_BLK_NORMAL) {
            b->next = block_head[MEM_BLK_HUGE];
            block_head[MEM_BLK_HUGE] = b;
        } else {
            b->next = block_head[b->tag];
            block_head[b->tag] = b;
        }
    }
}

void *cupkee_mem_ref(void *p)
{
    memory_block_t *b = container_of(p, memory_block_t, next);

    BLK_REF_INC(b);

    return p;
}

