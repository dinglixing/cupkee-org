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

#include "test.h"

typedef struct mock_mblock_t {
    struct mock_mblock_t *next;
    size_t size;
    char mem[0];
} mock_mblock_t;

static mock_mblock_t *mem_chain = NULL;

void hw_enter_critical(uint32_t *state)
{
    (void) state;
}

void hw_exit_critical(uint32_t state)
{
    (void) state;
}

void *hw_malloc(size_t size, size_t align)
{
    mock_mblock_t *mb = malloc(sizeof(mock_mblock_t) + size);

    (void) align;

    if (mb) {
        mb->size = size;
        mb->next = mem_chain;
        mem_chain = mb;

        memset(mb->mem, 0x55, size);

        return mb->mem;
    }
    return NULL;
}

void hw_mock_memory_reset(void)
{
}

