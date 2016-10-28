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

static void test_device(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    // create device 'gpio'
    CU_ASSERT(0 == test_cupkee_run_with_reply("device('GPIO') >= 0\r",    "true\r\n", 1));

    // create device with callback
    CU_ASSERT(0 == test_cupkee_run_with_reply("var e, h, h2\r", "undefined\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("device('other', def fn(err, hnd) {if (err) e = err else h = hnd})\r",    "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h\r",            "undefined\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("e\r",            "-20001\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("e = 0\r",        "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("(h2 = device('GPIO', fn)) >= 0\r", "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h == h2\r",      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("e\r",            "0\r\n", 1));

}

static void test_config(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var e, h, c\r", "undefined\r\n", 1));


    CU_ASSERT(0 == test_cupkee_run_with_reply("(h = device('GPIO')) >= 0\r","true\r\n", 1));

    // get device default configs
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'select')\r",  "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'dir')\r",     "\"out\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'mode')\r",    "\"push-pull\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'speed')\r",   "2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'other')\r",   "undefined\r\n", 1));

    // select pins config
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'select', pin('A', 0))\r",  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'select')\r",               "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'select', [pin('A', 0), pin('A', 1)])\r",  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'select').length()\r",      "2\r\n", 1));

    // dir config
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir', 'in')\r",   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir')\r",         "\"in\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir', 'out')\r",  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir')\r",         "\"out\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir', 'dual')\r", "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir')\r",         "\"dual\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir', 'other')\r","undefined\r\n", 1));

    // mode config
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'push-pull')\r",   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode')\r",         "\"push-pull\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'floating')\r", "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode')\r",         "\"floating\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'other')\r","undefined\r\n", 1));

    // speed config
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 10)\r",   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 20)\r",   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed')\r",       "20\r\n", 1));

    // config set is forbid if enabled
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                            "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, true)\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'select', pin('A', 0))\r",     "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir', 'in')\r",               "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'push-pull')\r",       "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 10)\r",               "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                            "true\r\n", 1));

    // disable & enable gpio group
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 0)\r",                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                            "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 1)\r",                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                            "true\r\n", 1));

}

static void test_write(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var h, d\r",                                             "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("(h = device('GPIO')) >= 0\r",                            "true\r\n", 1));

    //printf("\n\n\n start\n");
    // get device default configs
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'select', [pin('A', 0), pin('B', 15)])\r",     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir',  'out')\r",                             "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'open-drain')\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 25)\r",                               "true\r\n", 1));

    // enable gpio
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 1)\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                            "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 2)\r",                                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, true)\r",                                       "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 'a')\r",                                        "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, {})\r",                                         "false\r\n", 1));
    CU_ASSERT(0 == hw_dbg_gpio_get_pin(0, 0));
    CU_ASSERT(1 == hw_dbg_gpio_get_pin(1, 15));

    test_reply_show(1);
    test_reply_show(0);
}

static void test_read(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var h, d\r",                                             "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("(h = device('GPIO')) >= 0\r",                            "true\r\n", 1));

    // get device default configs
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'select', [pin('A', 0), pin('B', 15)])\r",     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir',  'in')\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'pull-down')\r",                       "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 25)\r",                               "true\r\n", 1));

    // enable gpio
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 1)\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                            "true\r\n", 1));

    hw_dbg_gpio_set_pin(0, 0);
    hw_dbg_gpio_get_pin(1, 15);
    CU_ASSERT(0 == test_cupkee_run_with_reply("read(h)\r",                                              "3\r\n", 1));
    test_reply_show(1);
    test_reply_show(0);
}

static void test_read_write(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var h, d\r",                                               "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("(h = device('GPIO')) >= 0\r",                            "true\r\n", 1));

    // get device default configs
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'select', [pin('A', 0), pin('B', 15)])\r",     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir',  'out')\r",                             "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'open-drain')\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 25)\r",                               "true\r\n", 1));
}

static void test_event(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));
}

static void test_unref(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));
}

CU_pSuite test_gpio_entry(void)
{
    CU_pSuite suite = CU_add_suite("gpio", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "device",    test_device);
        CU_add_test(suite, "config",    test_config);
        CU_add_test(suite, "write",     test_write);
        CU_add_test(suite, "read",      test_read);
        CU_add_test(suite, "read-write",test_read_write);
        CU_add_test(suite, "event",     test_event);
        CU_add_test(suite, "unref",     test_unref);
    }

    return suite;
}

