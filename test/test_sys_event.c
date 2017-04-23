/*
MIT License

This file is part of cupkee project

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

#include <stdio.h>
#include <string.h>

#include "test.h"
#include <cupkee.h>

/* */

void hw_enter_critical(uint32_t *state)
{
    (void) state;
}

void hw_exit_critical(uint32_t state)
{
    (void) state;
}

/* */

static int test_setup(void)
{
    return 0;
}

static int test_clean(void)
{
    return 0;
}

static void test_post_take(void)
{
    int i;
    cupkee_event_t e;

    cupkee_event_setup();

    CU_ASSERT_EQUAL(cupkee_event_post(0, 0, 0), 1);
    CU_ASSERT_EQUAL(cupkee_event_take(&e), 1);
    CU_ASSERT_EQUAL(e.type , 0);
    CU_ASSERT_EQUAL(e.which, 0);
    CU_ASSERT_EQUAL(e.code, 0);

    CU_ASSERT_EQUAL(cupkee_event_post(1, 1, 1), 1);
    CU_ASSERT_EQUAL(cupkee_event_take(&e), 1);
    CU_ASSERT_EQUAL(e.type , 1);
    CU_ASSERT_EQUAL(e.which, 1);
    CU_ASSERT_EQUAL(e.code, 1);

    for (i = 0; i < 256; i++) {
        cupkee_event_post(i, i, i);
        cupkee_event_take(&e);
    }
    CU_ASSERT_EQUAL(e.type , 255);
    CU_ASSERT_EQUAL(e.which, 255);
    CU_ASSERT_EQUAL(e.code, 255);

    cupkee_event_reset();
}

CU_pSuite test_sys_event(void)
{
    CU_pSuite suite = CU_add_suite("system event", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "post & take", test_post_take);
    }

    return suite;
}


