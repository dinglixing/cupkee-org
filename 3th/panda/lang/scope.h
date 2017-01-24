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

#ifndef __LANG_SCOPE_INC__
#define __LANG_SCOPE_INC__

#define MAGIC_SCOPE             (MAGIC_BASE + 1)

#define SCOPE_FL_HEAP           (1)     // variable space alloced in heap

typedef struct scope_t {
    uint8_t magic;
    uint8_t age;
    uint8_t num;                // all variables number
    uint8_t nao;                // nonamed arguments offset
    val_t   *var_buf;
    struct scope_t *super;
} scope_t;

static inline int scope_mem_space(scope_t *scope) {
    return SIZE_ALIGN(sizeof(scope_t)) + SIZE_ALIGN(sizeof(val_t) * scope->num);
}

#endif /* __LANG_SCOPE_INC__ */

