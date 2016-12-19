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

static void test_create(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    // create device 'uart'
    CU_ASSERT(0 == test_cupkee_run_with_reply("var a, b\r",                                    "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a = Device('uart')\r",                          "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b = Device('uart', 1)\r",                       "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.instance\r",                                  "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b.instance\r",                                  "1\r\n", 1));

    // return undefined, if instance out of range
    CU_ASSERT(0 == test_cupkee_run_with_reply("Device('uart', 100)\r",                         "undefined\r\n", 1));

    // return undefined, if instance already existed
    CU_ASSERT(0 == test_cupkee_run_with_reply("Device('uart')\r",                              "undefined\r\n", 1));

    // return undefined, if type is unknowned
    CU_ASSERT(0 == test_cupkee_run_with_reply("Device('other')\r",                             "undefined\r\n", 1));

    // destroy and recreate
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.destroy())\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b.destroy())\r",                                "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a = Device('uart', 0)\r",                       "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b = Device('uart', 1)\r",                       "<object>\r\n", 1));

    // properties of device
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.instance\r",                                  "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b.instance\r",                                  "1\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a.type == 'uart'\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b.type == 'uart'\r",                            "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a.category == 'stream'\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b.category == 'stream'\r",                      "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a.isEnabled()\r",                               "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.error\r",                                     "0\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a.read\r",                                      "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.write\r",                                     "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.enable\r",                                    "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.disable\r",                                   "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.listen\r",                                    "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.ignore\r",                                    "<function>\r\n", 1));

    return;
}

CU_pSuite test_device_uart(void)
{
    CU_pSuite suite = CU_add_suite("device uart", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "create",    test_create);
#if 0
        CU_add_test(suite, "config",    test_config);
        CU_add_test(suite, "enable",    test_enable);
        CU_add_test(suite, "read",      test_read);
        CU_add_test(suite, "write",     test_write);
        CU_add_test(suite, "event",     test_event);
#endif
    }

    return suite;
}

