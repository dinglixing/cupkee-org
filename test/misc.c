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
    return 0;
}

static int test_clean()
{
    return 0;
}

static void test_initial(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start("var a = 0"));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "0\r\n", 1));
}

static void test_enter(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    // empty input should feedback new prompt
    CU_ASSERT(0 == test_cupkee_run_with_reply("\r", NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("   \t\r", NULL, 1));
}

static void test_systicks(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));


    CU_ASSERT(0 == test_cupkee_run_with_reply("\r", NULL, 1));

    hw_systicks_set(0);
    CU_ASSERT(0 == test_cupkee_run_with_reply("systicks()\r", "0\r\n", 1));

    hw_systicks_set(1234);
    CU_ASSERT(0 == test_cupkee_run_with_reply("systicks()\r", "1234\r\n", 1));
}

static void test_scripts(void)
{
    test_cupkee_reset();

    hw_scripts_save("var a = 0");

    CU_ASSERT(0 == test_cupkee_start(NULL));

    // variable a should defined
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "0\r\n", 1));

    // scripts should be delete
    CU_ASSERT(0 == test_cupkee_run_with_reply("scripts('delete')\r", "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("scripts()\r", "0\r\n", 1));

    hw_scripts_save("a\r\n");
    hw_scripts_save("b\r\n");
    CU_ASSERT(0 == test_cupkee_run_with_reply("scripts(0)\r", "[0000] a\r\n2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("scripts(1)\r", "[0001] b\r\n2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("scripts()\r", "[0000] a\r\n[0001] b\r\n2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("scripts(1, 'delete')\r", "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("scripts(1)\r", "1\r\n", 1));

    hw_scripts_save("c\r\n");
    hw_scripts_save("d\r\n");
    CU_ASSERT(0 == test_cupkee_run_with_reply("scripts()\r", "[0000] a\r\n[0001] c\r\n[0002] d\r\n3\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("scripts(0, 'delete')\r", "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("scripts()\r", "[0000] c\r\n[0001] d\r\n2\r\n", 1));
}

static void test_timeout(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    //test_reply_show(1);
    CU_ASSERT(0 == test_cupkee_run_with_reply("var a = 0\r", "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("def f() return a++;\r", NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("f();\r", "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "1\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var t = setTimeout(f, 10)\r", "undefined\r\n", 1));
    hw_systicks_set(9);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 10));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "1\r\n", 1));

    // trigger timeout
    hw_systicks_set(10);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "2\r\n", 1));

    // clear timeout
    CU_ASSERT(0 == test_cupkee_run_with_reply("var t = setTimeout(f, 10)\r", "undefined\r\n", 1));
    hw_systicks_set(19);
    CU_ASSERT(0 == test_cupkee_run_with_reply("clearTimeout(t)\r", "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("clearTimeout(t)\r", "0\r\n", 1));
    hw_systicks_set(20);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "2\r\n", 1));

    // register self
    CU_ASSERT(0 == test_cupkee_run_with_reply("def fn() if (a < 5) { a++, setTimeout(fn, 10)};\r", NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("fn()\r", "undefined\r\n", 1));
    hw_systicks_set(29);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 10));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "3\r\n", 1));
    hw_systicks_set(30);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "4\r\n", 1));
    hw_systicks_set(40);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "5\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("clearTimeout()\r", "1\r\n", 1));

    // interval
    CU_ASSERT(0 == test_cupkee_run_with_reply("t = setInterval(f, 10)\r", NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "5\r\n", 1));
    // trigger interval event
    hw_systicks_set(50);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "6\r\n", 1));
    hw_systicks_set(60);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "7\r\n", 1));
    hw_systicks_set(70);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "8\r\n", 1));

    // clear interval
    CU_ASSERT(0 == test_cupkee_run_with_reply("clearInterval(t)\r", "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("clearInterval(t)\r", "0\r\n", 1));
    hw_systicks_set(80);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a\r", "8\r\n", 1));
}

CU_pSuite test_misc_entry()
{
    CU_pSuite suite = CU_add_suite("misc", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "initial",   test_initial);
        CU_add_test(suite, "empty",     test_enter);
        CU_add_test(suite, "systicks",  test_systicks);
        CU_add_test(suite, "scripts",   test_scripts);
        CU_add_test(suite, "timeout",   test_timeout);
    }

    return suite;
}

