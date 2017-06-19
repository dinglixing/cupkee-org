/*
MIT License

This file is part of cupkee project

Copyright (c) 201y Lixing Ding <ding.lixing@gmail.com>

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
#include <cupkee.h>

static int test_setup(void)
{
    TU_pre_init();
    return 0;
}

static int test_clean(void)
{
    TU_pre_deinit();
    return 0;
}

static void test_alloc(void)
{
    int i;

    cupkee_memory_desc_t descs[4] = {
        {32, 4}, {64, 4}, {128, 4}, {256, 4}
    };

    cupkee_memory_init(4, descs);

    // alloc from pool

    for (i = 0; i < 256; i++) {
        void *p = cupkee_malloc(32);

        if (!p) CU_ASSERT_FATAL(0);

        cupkee_free(p);
    }
    for (i = 0; i < 256; i++) {
        void *p = cupkee_malloc(64);

        if (!p) CU_ASSERT_FATAL(0);

        cupkee_free(p);
    }
    for (i = 0; i < 256; i++) {
        void *p = cupkee_malloc(128);

        if (!p) CU_ASSERT_FATAL(0);

        cupkee_free(p);
    }
    for (i = 0; i < 256; i++) {
        void *p = cupkee_malloc(256);

        if (!p) CU_ASSERT_FATAL(0);

        cupkee_free(p);
    }
}

static void test_ref(void)
{
    void *o, *p;
    cupkee_memory_desc_t desc = {32, 4};

    cupkee_memory_init(1, &desc);

    CU_ASSERT((o = cupkee_malloc(31)) != NULL);

    // ran out of memory
    while (cupkee_malloc(3))
        ;

    // not memory should be alloced
    CU_ASSERT(cupkee_malloc(31) == NULL);

    // release 1 block
    cupkee_free(o);
    CU_ASSERT((p = cupkee_malloc(31)) != NULL);

    // create refence
    CU_ASSERT(cupkee_mem_ref(p) == p);
    CU_ASSERT(cupkee_malloc(31) == NULL);

    cupkee_free(p);
    CU_ASSERT(cupkee_malloc(31) == NULL);
    cupkee_free(p);
    CU_ASSERT((p = cupkee_malloc(31)) != NULL);
}

CU_pSuite test_sys_memory(void)
{
    CU_pSuite suite = CU_add_suite("system memory", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "alloc", test_alloc);
        CU_add_test(suite, "ref",   test_ref);
    }

    return suite;
}

