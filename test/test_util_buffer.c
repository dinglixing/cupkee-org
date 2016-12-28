/*
MIT License

This file is part of cupkee project

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

#include <stdio.h>
#include <string.h>

#include "test.h"

static int test_setup()
{
    return 0;
}

static int test_clean()
{
    return 0;
}

static void test_alloc_release(void)
{
    void *b;
    int i = 0;

    test_cupkee_reset();

    CU_ASSERT(NULL != (b = cupkee_buf_alloc()));

    while (i++ < 100) {
        cupkee_buf_release(cupkee_buf_alloc());
    }
    CU_ASSERT(NULL != (b = cupkee_buf_alloc()));
    cupkee_buf_release(b);
}

static void test_misc(void)
{
    void *b;
    int cap;

    test_cupkee_reset();

    CU_ASSERT(NULL != (b = cupkee_buf_alloc()));

    CU_ASSERT(cupkee_buf_is_empty(b));
    CU_ASSERT(!cupkee_buf_is_full(b));

    CU_ASSERT(0 < (cap = cupkee_buf_capacity(b)));
    CU_ASSERT(cap == cupkee_buf_space(b));
    CU_ASSERT(0 == cupkee_buf_length(b));
}

static void test_push_shift(void)
{
    void *b;
    int i, cap;
    uint8_t d, m;

    test_cupkee_reset();

    CU_ASSERT(NULL != (b = cupkee_buf_alloc()));
    CU_ASSERT(0 < (cap = cupkee_buf_capacity(b)));

    CU_ASSERT(1 == cupkee_buf_push(b, 'a'));
    CU_ASSERT(1 == cupkee_buf_length(b));
    CU_ASSERT(1 + cupkee_buf_space(b) == cap);

    CU_ASSERT(1 == cupkee_buf_shift(b, &d));
    CU_ASSERT(0 == cupkee_buf_length(b));
    CU_ASSERT(0 + cupkee_buf_space(b) == cap);

    CU_ASSERT(d == 'a');

    d = 0;
    for (i = 0; i < cap; i++) {
        cupkee_buf_push(b, d++);
    }
    CU_ASSERT(cupkee_buf_is_full(b));
    CU_ASSERT(cap == cupkee_buf_length(b));
    CU_ASSERT(0 == cupkee_buf_push(b, d));

    for (i = 0; i < cap; i++) {
        cupkee_buf_shift(b, &d);
    }
    CU_ASSERT(cupkee_buf_is_empty(b));
    CU_ASSERT(0 == cupkee_buf_length(b));
    CU_ASSERT(0 == cupkee_buf_shift(b, &d));

    d = 0; m = 0;
    while (1) {
        for (i = 0; i < 7; i++) {
            cupkee_buf_push(b, d++);
        }

        if (cupkee_buf_is_full(b)) {
            break;
        }

        for (i = 0; i < 6; i++) {
            uint8_t v;
            cupkee_buf_shift(b, &v);
            if (v != m++) {
                CU_ASSERT(0);
            }
        }
    }

}

static void test_give_take(void)
{
    void *b;
    int i, cap;
    uint8_t td[16], gd[7];
    uint8_t d, m;

    test_cupkee_reset();

    CU_ASSERT(NULL != (b = cupkee_buf_alloc()));
    CU_ASSERT(0 < (cap = cupkee_buf_capacity(b)));

    CU_ASSERT(0 == cupkee_buf_take(b, 1, &d));

    for (i = 0, d = 0; i < cap; i++) {
        if ((1 != cupkee_buf_give(b, 1, &d)) ||
            (1 != cupkee_buf_take(b, 1, &m)) ||
            (m != d++)) {
            CU_ASSERT(0);
        }
    }

    for (i = 0, d = 0; i < cap; i++, d++) {
        if (1 != cupkee_buf_give(b, 1, &d)) {
            CU_ASSERT(0);
        }
    }
    CU_ASSERT(0 == cupkee_buf_give(b, 1, &d));

    for (i = 0, d = 0; i < cap; i++) {
        if ((1 != cupkee_buf_take(b, 1, &m)) || (m != d++)) {
            CU_ASSERT(0);
        }
    }
    CU_ASSERT(0 == cupkee_buf_take(b, 1, &d));

    for (i = 0, d = 0; i < 16; i++) {
        gd[i] = d++;
    }
    CU_ASSERT(16 == cupkee_buf_give(b, 16, gd));

    m = 0;
    while (!cupkee_buf_is_empty(b)) {
        for (i = 0; i < 6; i++) {
            gd[i] = d++;
        }
        if (6 != cupkee_buf_give(b, 6, gd)) {
            CU_ASSERT(0); // fail!
        }

        if (7 != cupkee_buf_take(b, 7, td)) {
            CU_ASSERT(0); // fail!
        }
        for (i = 0; i < 7; i++) {
            if (td[i] != m++) {
                CU_ASSERT(0); // fail!
            }
        }
    }
}

CU_pSuite test_util_buffer()
{
    CU_pSuite suite = CU_add_suite("util buffer", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "alloc&release", test_alloc_release);
        CU_add_test(suite, "misc         ", test_misc);
        CU_add_test(suite, "push&shift   ", test_push_shift);
        CU_add_test(suite, "give&take    ", test_give_take);
    }

    return suite;
}

