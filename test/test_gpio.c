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
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'pin')\r",   "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'mode')\r",  "\"in-float\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'speed')\r", "2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'other')\r", "undefined\r\n", 1));

    // pins config
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'pin', pin('A', 0))\r",  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'pin')\r",               "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'pin', [pin('A', 0), pin('A', 1)])\r",  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'pin').length()\r",      "2\r\n", 1));
    test_reply_show(1);
    test_reply_show(0);

    // mode config
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'out-pushpull')\r",    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode')\r",                    "\"out-pushpull\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'in-float')\r",        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode')\r",                    "\"in-float\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'dual')\r",            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode')\r",                    "\"dual\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'other')\r",           "undefined\r\n", 1));

    // speed config
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 10)\r",   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 20)\r",   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed')\r",       "20\r\n", 1));

    // config set is forbid if enabled
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                            "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, true)\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'pin', pin('A', 0))\r",        "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'out-pushpull')\r",    "false\r\n", 1));
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

    // configs
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'pin', [pin('A', 0), pin('B', 15)])\r",        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'out-opendrain')\r",                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 25)\r",                               "true\r\n", 1));

    // enable gpio
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 1)\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                            "true\r\n", 1));

    // write
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 1)\r",                                          "true\r\n", 1));
    CU_ASSERT(1 == hw_dbg_gpio_get_pin(0, 0));
    CU_ASSERT(0 == hw_dbg_gpio_get_pin(1, 15));

    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 2)\r",                                          "true\r\n", 1));
    CU_ASSERT(0 == hw_dbg_gpio_get_pin(0, 0));
    CU_ASSERT(1 == hw_dbg_gpio_get_pin(1, 15));

    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 3)\r",                                          "true\r\n", 1));
    CU_ASSERT(1 == hw_dbg_gpio_get_pin(0, 0));
    CU_ASSERT(1 == hw_dbg_gpio_get_pin(1, 15));

    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 0)\r",                                          "true\r\n", 1));
    CU_ASSERT(0 == hw_dbg_gpio_get_pin(0, 0));
    CU_ASSERT(0 == hw_dbg_gpio_get_pin(1, 15));

    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, true)\r",                                       "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, '1')\r",                                        "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, {})\r",                                         "false\r\n", 1));

    // read
    // read always false, if dir is 'out'
    CU_ASSERT(0 == test_cupkee_run_with_reply("read(h)\r",                                              "false\r\n", 1));

    test_reply_show(1);
    test_reply_show(0);
}

static void test_read(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var h, d\r",                                             "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("(h = device('GPIO')) >= 0\r",                            "true\r\n", 1));

    // configs
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'pin', [pin('A', 0), pin('B', 15)])\r",        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'in-pullupdown')\r",                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 25)\r",                               "true\r\n", 1));

    // enable gpio
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 1)\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                            "true\r\n", 1));

    // read
    hw_dbg_gpio_set_pin(0, 0);
    hw_dbg_gpio_set_pin(1, 15);
    CU_ASSERT(0 == test_cupkee_run_with_reply("read(h)\r",                                              "3\r\n", 1));

    hw_dbg_gpio_clr_pin(0, 0);
    CU_ASSERT(0 == test_cupkee_run_with_reply("read(h)\r",                                              "2\r\n", 1));

    hw_dbg_gpio_clr_pin(1, 15);
    CU_ASSERT(0 == test_cupkee_run_with_reply("read(h)\r",                                              "0\r\n", 1));

    // write
    // write always false, if dir is 'in'
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 1)\r",                                          "false\r\n", 1));
}

static void test_read_write(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var h, d\r",                                             "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("(h = device('GPIO')) >= 0\r",                            "true\r\n", 1));

    // configs
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'pin', [pin('A', 0), pin('B', 15)])\r",        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'dual')\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 25)\r",                               "true\r\n", 1));

    // enable gpio
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 1)\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                            "true\r\n", 1));

    // write & read
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 1)\r",                                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("read(h)\r",                                              "1\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 3)\r",                                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("read(h)\r",                                              "3\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 0)\r",                                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("read(h)\r",                                              "0\r\n", 1));

    test_reply_show(1);
    test_reply_show(0);
}

static void test_event(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));


    CU_ASSERT(0 == test_cupkee_run_with_reply("var h, d\r",                                             "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("(h = device('GPIO')) >= 0\r",                            "true\r\n", 1));

    // configs
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'pin', [pin('A', 0), pin('B', 15)])\r",        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'dual')\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'speed', 25)\r",                               "true\r\n", 1));

    // enable gpio
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 1)\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                            "true\r\n", 1));

    hw_dbg_gpio_clr_pin(0, 0);
    hw_dbg_gpio_clr_pin(1, 15);
    CU_ASSERT(0 == test_cupkee_run_with_reply("read(h)\r",                                              "0\r\n", 1));

    // listen "change"
    CU_ASSERT(0 == test_cupkee_run_with_reply("def cb(){d = read(h)}\r",                                "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("listen(h, 'change', def() {d = read(h)})\r",             "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d\r",                                                    "undefined\r\n", 1));

    hw_dbg_gpio_set_pin(0, 0);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d\r",                                                    "1\r\n", 1));

    hw_dbg_gpio_set_pin(1, 15);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d\r",                                                    "3\r\n", 1));

    hw_dbg_gpio_clr_pin(0, 0);
    hw_dbg_gpio_clr_pin(1, 15);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d\r",                                                    "0\r\n", 1));

    // ignore "change"
    CU_ASSERT(0 == test_cupkee_run_with_reply("ignore(h, 0)\r",                                         "true\r\n", 1));

    hw_dbg_gpio_set_pin(1, 15);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d\r",                                                    "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("read(h)\r",                                              "2\r\n", 1));
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
        if (0) {
        }
    }

    return suite;
}

