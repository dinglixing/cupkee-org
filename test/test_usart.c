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
    CU_ASSERT(0 == test_cupkee_run_with_reply("h = device('USART')\r",                              "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h = device('other')\r",                              "undefined\r\n", 1));
}

static void test_config(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var e, h, c\r",                                      "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("(h = device('USART'))\r",                              "<object>\r\n", 1));

    // get device default configs
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('baudrate')\r",                             "9600\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('databits')\r",                             "8\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('stopbits')\r",                             "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity')\r",                               "\"none\"\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(0)\r",                                      "9600\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(1)\r",                                      "8\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(2)\r",                                      "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(3)\r",                                      "\"none\"\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(4)\r",                                      "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('other')\r",                                "undefined\r\n", 1));

    test_reply_show(1);
    test_reply_show(0);

    // config baudrate
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('baudrate', 115200)\r",                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(0)\r",                                      "115200\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(0, 38400)\r",                               "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('baudrate')\r",                             "38400\r\n", 1));

    // config databits
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('databits', 5)\r",                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('databits')\r",                             "5\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(1, 8)\r",                                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(1)\r",                                      "8\r\n", 1));

    // config stopbits
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('stopbits', 2)\r",                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('stopbits')\r",                             "2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(2, 1)\r",                                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(2)\r",                                      "1\r\n", 1));

    // config parity
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity', 0)\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(3)\r",                                      "\"none\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity', 1)\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(3)\r",                                      "\"odd\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity', 2)\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(3)\r",                                      "\"even\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity', 3)\r",                            "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity')\r",                               "\"even\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(3, 'none')\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity')\r",                               "\"none\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(3, 'odd')\r",                               "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity')\r",                               "\"odd\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(3, 'even')\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity')\r",                               "\"even\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config(3, 'other')\r",                             "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity')\r",                               "\"even\"\r\n", 1));

    // config set is forbid if enabled
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                        "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, true)\r",                                  "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('baudrate', 115200)\r",                     "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('databits', 5)\r",                          "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('stopbits', 2)\r",                          "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity', 0)\r",                            "false\r\n", 1));

    // disable & enable
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 0)\r",                                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                        "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('baudrate', 115200)\r",                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('databits', 5)\r",                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('stopbits', 2)\r",                          "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.config('parity', 0)\r",                            "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, 1)\r",                                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                        "true\r\n", 1));
}

static void test_write(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var h, d, l, state;\r",                              "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("(h = device('USART'))\r",                            "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Buffer('hello world')\r",                        "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("l = d.length()\r",                                   "11\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, {\
  baudrate: 115200,\
  databits: 8,\
  stopbits: 1,\
  parity: 0})\r",                                                                                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                        "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 'hello')\r",                                "5\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, d)\r",                                      "11\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, 'hello', 2)\r",                             "2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("write(h, d, 10)\r",                                  "10\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("\
write(h, 'hello', function(err, n, data) {\
    if (!err) state = [n, data]\
})\r",                                                                                              "4\r\n", 1));
    // hw send buffer is full (32)
    CU_ASSERT(0 == test_cupkee_run_with_reply("state[0] == 4\r",                                    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("state[1] == 'hello'\r",                              "true\r\n", 1));

    // send 5 bytes
    hw_dbg_usart_enable_out(5);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("\
write(h, d, function(err, n, data) {\
    if (!err) state = n\
})\r",                                                                                              "5\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("state == 5\r",                                       "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = d.slice(state)\r",                               "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.length())\r",                                      "6\r\n", 1));

    // send 10 bytes
    hw_dbg_usart_enable_out(10);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("h.write(d, 10)\r",                                   "6\r\n", 1));
    return;
}

static void test_read(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var h, d, s = '12345'\r",                                "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h = device('USART'))\r",                                 "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h, {\
    baudrate: 115200,\
    databits: 8,\
    stopbits: 1,\
    parity: 0})\r",                                                                                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("enable(h)\r",                                            "true\r\n", 1));

    // without callback
    CU_ASSERT(0 == test_cupkee_run_with_reply("read(h, 5)\r",                                           "true\r\n", 1));

    // with callback
    hw_dbg_usart_set_input("12345");
    CU_ASSERT(0 == test_cupkee_run_with_reply("\
read(h, 5, function(err, data) {\
  if (!err) d = data;\
})\r",                                                                                                  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d\r",                                                    "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.length()\r",                                           "5\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[0] == 49\r",                                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[1] == 50\r",                                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[2] == 51\r",                                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[3] == 52\r",                                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[4] == 53\r",                                           "true\r\n", 1));

    return;
}

static void test_event(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));


    CU_ASSERT(0 == test_cupkee_run_with_reply("var h, d, c = 0\r",                                      "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h = device('USART'))\r",                                 "<object>\r\n", 1));

    // enable
    CU_ASSERT(0 == test_cupkee_run_with_reply("\
h.enable({\
  baudrate: 115200,\
  databits: 8,\
  stopbits: 1,\
  parity: 0 \
}, function(err, dev) {\
  if (!err) {\
    dev.listen('data', def(data) {d = data}); \
    dev.listen('drain', def(dev) c++); \
  } \
})\r",                                                                                                  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h.enable()\r",                                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d\r",                                                    "undefined\r\n", 1));

    hw_dbg_usart_set_input("12345");
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d\r",                                                    "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.length()\r",                                           "5\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[0] == 49\r",                                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[1] == 50\r",                                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[2] == 51\r",                                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[3] == 52\r",                                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[4] == 53\r",                                           "true\r\n", 1));

    test_reply_show(1);
    test_reply_show(0);

    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Buffer(40)\r",                                       "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.length()\r",                                           "40\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("h.write(d)\r",                                           "32\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c\r",                                                    "0\r\n", 1));

    hw_dbg_usart_enable_out(32);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c\r",                                                    "1\r\n", 1));
}

static void test_unref(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    // not support
}

CU_pSuite test_device_usart(void)
{
    CU_pSuite suite = CU_add_suite("usart", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "device",    test_device);
        CU_add_test(suite, "config",    test_config);
        CU_add_test(suite, "write",     test_write);
        CU_add_test(suite, "read",      test_read);
        CU_add_test(suite, "event",     test_event);
        CU_add_test(suite, "unref",     test_unref);
#if 0
#endif
    }

    return suite;
}

