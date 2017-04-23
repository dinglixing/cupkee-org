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

#ifdef SIZE_ALIGN
#undef SIZE_ALIGN
#endif

#define SIZE_ALIGN(s)   (((s) + 15) & ~15)

#ifdef ADDR_ALIGN
#undef ADDR_ALIGN
#endif
#define ADDR_ALIGN(a)   (void *)((((intptr_t)(a)) + 15) & ~15)

#define member_offset(T, m) (intptr_t)(&(((T *)0)->m))
#define container_of(p, T, m) ((T*)((intptr_t)(p) - member_offset(T, m)))

#define BLK_REF_INC(b)      ((b)->ref += 2)
#define BLK_REF_DEC(b)      do {if ((b)->ref > 1) (b)->ref -= 2;} while (0)

typedef struct mem_block_t {
    struct mem_block_t*next;
} mem_block_t;

typedef struct memory_pool_t {
    struct memory_pool_t *next;
    uint16_t block_size;
    uint16_t block_num;

    uint32_t pool_size;
    void    *pool_base;

    uint16_t *block_ref;

    mem_block_t *block_head;
} memory_pool_t;


static memory_pool_t *pool_head = NULL;

void cupkee_memory_setup(void)
{
    pool_head = NULL;
}

void cupkee_memory_pool_setup(size_t block_size, size_t pool_size, void *ptr)
{
    memory_pool_t *pool = (memory_pool_t *)ptr;
    uint32_t i;

    block_size = SIZE_ALIGN(block_size);

    pool->pool_base = ADDR_ALIGN(pool + 1);
    pool->pool_size = (pool_size - ((pool->pool_base) - ptr)) & ~15; // 16Byte align

    pool->block_size = block_size;
    pool->block_num  = pool->pool_size / (block_size + sizeof(uint16_t));

    // update pool_size
    pool->pool_size = block_size * pool->block_num;
    pool->block_ref = (uint16_t *)(pool->pool_base + pool->pool_size);

    pool->block_head = NULL;

    for (i = 0; i < pool->block_num; i++) {
        mem_block_t *block = (mem_block_t *)(pool->pool_base + block_size * i);

        block->next = pool->block_head;
        pool->block_head = block;
        pool->block_ref[i] = 0;
    }

    pool->next = pool_head;
    pool_head = pool;
}

void *cupkee_alloc(size_t n)
{
    memory_pool_t *pool = pool_head;
    while (pool) {
        if (pool->block_size >= n && pool->block_head) {
            mem_block_t *block = pool->block_head;
            uint32_t id = ((intptr_t)block - (intptr_t)pool->pool_base) / pool->block_size;

            pool->block_head = block->next;
            pool->block_ref[id] = 2;

            return block;
        }
        pool = pool->next;
    }

    return NULL;
}

void cupkee_free(void *p)
{
    memory_pool_t *pool = pool_head;

    while (pool) {
        intptr_t dis = (intptr_t) p - (intptr_t) pool->pool_base;
        if (dis >= 0 && dis < pool->pool_size) {
            int id = dis / pool->block_size;

            if (pool->block_ref[id] > 1) {
                pool->block_ref[id] -= 2;
            }
            if (pool->block_ref[id] == 0) {
                mem_block_t *block = p;

                block->next = pool->block_head;
                pool->block_head = block;
            }
            return;
        }
        pool = pool->next;
    }
}

void *cupkee_mem_ref(void *p)
{
    memory_pool_t *pool = pool_head;

    while (pool) {
        intptr_t dis = (intptr_t) p - (intptr_t) pool->pool_base;
        if (dis >= 0 && dis < pool->pool_size) {
            int id = dis / pool->block_size;

            pool->block_ref[id] += 2;
            return p;
        }
        pool = pool->next;
    }
    return NULL;
}

