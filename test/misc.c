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

#include <stdio.h>
#include <string.h>

#include "test.h"

static int test_setup()
{
    return test_cupkee_init();
}

static int test_clean()
{
    return test_cupkee_deinit();
}

static void test_systicks(void)
{
    // show logo
    CU_ASSERT(0 == test_cupkee_run_with_reply("\r", NULL, 1));

    hw_mock_systicks_set(0);
    CU_ASSERT(0 == test_cupkee_run_with_reply("systicks()\r", "0", 1));

    hw_mock_systicks_set(1234);
    CU_ASSERT(0 == test_cupkee_run_with_reply("systicks()\r", "1234", 1));
}

CU_pSuite test_misc_entry()
{
    CU_pSuite suite = CU_add_suite("misc", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "systicks", test_systicks);
    }

    return suite;
}

