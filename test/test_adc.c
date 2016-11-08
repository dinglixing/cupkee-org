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
    CU_ASSERT(0 == test_cupkee_run_with_reply("var h\r",                                            "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h = device('ADC')\r",                                "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h = device('other')\r",                              "undefined\r\n", 1));
}

static void test_config(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var e, h, c\r",                                      "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("(h = device('ADC'))\r",                              "<object>\r\n", 1));

    // get device default configs
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('channel')\r",                              "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('interval')\r",                             "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('other')\r",                                "undefined\r\n", 1));

    test_reply_show(1);
    test_reply_show(0);

    // channel set
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'channel', 1)\r",                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('channel')\r",                              "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('channel', 16)\r",                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('channel')\r",                              "16\r\n", 1));

    // mode config
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'interval', 1)\r",                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('interval')\r",                             "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('interval', 10)\r",                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('interval')\r",                             "10\r\n", 1));

    // config set is forbid if enabled
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                        "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, true)\r",                                  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('channel', 0)\r",                           "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('channel')\r",                              "16\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('interval', 0)\r",                          "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('interval')\r",                             "10\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                        "true\r\n", 1));

    // disable & enable adc
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 0)\r",                                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                        "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 1)\r",                                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                        "true\r\n", 1));
}

static void test_write(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var h;\r",                                           "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("(h = device('ADC'))\r",                              "<object>\r\n", 1));

    test_reply_show(1);
    test_reply_show(0);

    // channel set
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'channel', 1)\r",                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('channel')\r",                              "1\r\n", 1));

    // interval set
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'interval', 1)\r",                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('interval')\r",                             "1\r\n", 1));

    // enable device
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, true)\r",                                  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                        "true\r\n", 1));

    // write, always false
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 1)\r",                                      "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 2)\r",                                      "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 3)\r",                                      "false\r\n", 1));

    test_reply_show(1);
    test_reply_show(0);
}

/*
static void test_read(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var h, d\r",                                             "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h = device('GPIO'))\r",                                  "<object>\r\n", 1));

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
    CU_ASSERT(0 == test_cupkee_run_with_reply("h = device('GPIO'))\r",                                  "<object>\r\n", 1));

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
    CU_ASSERT(0 == test_cupkee_run_with_reply("h = device('GPIO'))\r",                                  "<object>\r\n", 1));

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

static void test_together(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

//
    hw_dbg_gpio_clr_pin(0, 0);
    hw_dbg_gpio_clr_pin(0, 1);
    CU_ASSERT(0 == test_cupkee_run_with_reply("var v, io;\r",                                            "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("\
device('GPIO').enable({\
  pin: [pin('a', 0), pin('a', 1)],\
  mode: 'dual'\
}, function(err, dev) {\
  if (!err) {\
    io = dev;\
  }\
});\r",                                                                                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("io\r",                                                   "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("io.listen('change', def() {v = read(io)})\r",            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("io.config('pin');\r",                                    "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("io.config('mode');\r",                                   "\"dual\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("io.config('speed');\r",                                  "2\r\n", 1));
    test_reply_show(0);

    hw_dbg_gpio_set_pin(0, 0);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v\r",                                                    "1\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("io.write(3)\r",                                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("io.read()\r",                                            "3\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v\r",                                                    "3\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("io.ignore('change')\r",                                  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("io.write(0)\r",                                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v\r",                                                    "3\r\n", 1));
    CU_ASSERT(0 == hw_dbg_gpio_get_pin(0, 0));
    CU_ASSERT(0 == hw_dbg_gpio_get_pin(0, 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("\
device('GPIO').enable({\
  pin: [pin('b', 0), pin('b', 1)],\
  mode: 'dual'\
}, function(err, dev){\
  if (!err) {\
    io = dev\
    dev.listen('change', function(){v = read(io)});\
  }\
})\r",                                                                                                  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v = 0\r",                                                "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("io.write(1)\r",                                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v\r",                                                    "1\r\n", 1));

    return;

    //
}
*/

CU_pSuite test_adc_entry(void)
{
    CU_pSuite suite = CU_add_suite("adc", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "device",    test_device);
        CU_add_test(suite, "config",    test_config);
        CU_add_test(suite, "write",     test_write);
#if 0
        CU_add_test(suite, "read",      test_read);
        CU_add_test(suite, "read-write",test_read_write);
        CU_add_test(suite, "event",     test_event);
        CU_add_test(suite, "unref",     test_unref);
        CU_add_test(suite, "together",  test_together);
#endif
    }

    return suite;
}

