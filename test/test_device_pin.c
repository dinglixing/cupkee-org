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

    // create device 'pin'
    CU_ASSERT(0 == test_cupkee_run_with_reply("var a, b\r",                                    "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a = Device('pin')\r",                           "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b = Device('pin', 1)\r",                        "<object>\r\n", 1));

    // return undefined, if instance out of range
    CU_ASSERT(0 == test_cupkee_run_with_reply("Device('pin', 2)\r",                            "undefined\r\n", 1));

    // return undefined, if instance already existed
    CU_ASSERT(0 == test_cupkee_run_with_reply("Device('pin')\r",                               "undefined\r\n", 1));

    // return undefined, if type is unknowned
    CU_ASSERT(0 == test_cupkee_run_with_reply("Device('other')\r",                             "undefined\r\n", 1));

    // destroy and recreate
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.destroy())\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b.destroy())\r",                                "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a = Device('pin', 0)\r",                        "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b = Device('pin', 1)\r",                        "<object>\r\n", 1));

    // properties of device
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.instance\r",                                  "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b.instance\r",                                  "1\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a.type == 'pin'\r",                             "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b.type == 'pin'\r",                             "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a.category == 'map'\r",                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b.category == 'map'\r",                         "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a.isEnabled()\r",                               "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.getError()\r",                                "0\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a.read\r",                                      "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.write\r",                                     "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.enable\r",                                    "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.disable\r",                                   "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.listen\r",                                    "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.ignore\r",                                    "<function>\r\n", 1));

    return;
}

static void test_config(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d\r",                                      "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Device('pin')\r",                          "<object>\r\n", 1));

    // config get default setting
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)\r",                                "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1)\r",                                "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2)\r",                                "\"in\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('pinNum')\r",                         "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('pinStart')\r",                       "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dir')\r",                            "\"in\"\r\n", 1));

    // return undefined, query a configure not defined
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('other')\r",                          "undefined\r\n", 1));

    // config set valid
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, 1)\r",                             "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1, 1)\r",                             "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2, 1)\r",                             "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)\r",                                "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1)\r",                                "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2)\r",                                "\"out\"\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('pinNum', 4)\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('pinStart', 2)\r",                    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dir', 'dual')\r",                    "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)\r",                                "4\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1)\r",                                "2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2)\r",                                "\"dual\"\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config({pinNum: 1,"
                                                "pinStart:1,"
                                                "dir: 'out'"
                                              "})\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)\r",                                "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1)\r",                                "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2)\r",                                "\"out\"\r\n", 1));

    // config set invalid
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, true)\r",                          "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, NaN)\r",                           "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1, 'hello')\r",                       "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1, [])\r",                            "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2, 5)\r",                             "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2, 'other')\r",                       "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(3, 'other')\r",                       "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('other', 1)\r",                       "false\r\n", 1));

    test_reply_show(1);
    test_reply_show(0);

    return;
}

static void test_enable(void)
{
    return;
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d;\r",                                     "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = xxx('map')\r",                             "<object>\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool')\r",                           "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number')\r",                         "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option')\r",                         "\"x\"\r\n", 1));

    // return is enable, without argument
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnable()\r",                               "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnable()\r",                               "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnable()\r",                               "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({\
  bool: true,\
  number: 10,\
  option: 'yy'\
})\r",                                                                                        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnable()\r",                               "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool')\r",                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number')\r",                         "10\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option')\r",                         "\"yy\"\r\n", 1));

}

#if 0
static void test_read(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d;\r",                                     "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = xxx('map')\r",                             "<object>\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({\
  bool: true,\
  number: 10,\
  option: 'yy'\
})\r",                                                                                        "true\r\n", 1));

    hw_dbg_map_set(0, 0, 1);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(0)\r",                                  "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[0]\r",                                       "1\r\n", 1));

    hw_dbg_map_set(0, -1, 3);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read()\r",                                   "3\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(0)\r",                                  "undefined\r\n", 1));
}

static void test_write(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d;\r",                                     "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = xxx('map')\r",                             "<object>\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({\
  bool: true,\
  number: 10,\
  option: 'yy'\
})\r",                                                                                        "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(0, 2)\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[0]\r",                                       "2\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(4)\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read()\r",                                   "4\r\n", 1));
}

static void test_event(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));


    CU_ASSERT(0 == test_cupkee_run_with_reply("var d, v, e\r",                                "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = xxx('map', 1))\r",                         "<object>\r\n", 1));

    // enable & listen
    CU_ASSERT(0 == test_cupkee_run_with_reply("\
d.enable({\
  bool: true,\
  number: 8,\
  option: 'x'\
}, function(err, dev) { \
  if (!err) { \
    dev.listen('data', function(data) { \
      v = data; \
    });\
    dev.listen('error', function(err) { \
      e = err; \
    });\
  }\
})\r",                                                                                        "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("e\r",                                          "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v\r",                                          "undefined\r\n", 1));

    hw_dbg_map_event_triger(1, DEVICE_EVENT_ERR);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("e > 0\r",                                      "true\r\n", 1));

    // support combine read
    hw_dbg_map_set(1, -1, 5);
    hw_dbg_map_event_triger(1, DEVICE_EVENT_DATA);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v == 5\r",                                     "true\r\n", 1));

    // support combine read
    hw_dbg_map_set_size(1, 8);
    hw_dbg_map_set(1, 0, 3);
    hw_dbg_map_event_triger(1, DEVICE_EVENT_DATA);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v\r",                                          "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v.length()\r",                                 "8\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v[0]\r",                                       "3\r\n", 1));

}
#endif

CU_pSuite test_device_pin(void)
{
    CU_pSuite suite = CU_add_suite("device pin", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "create",    test_create);
        CU_add_test(suite, "config",    test_config);
        CU_add_test(suite, "enable",    test_enable);

#if 0
        CU_add_test(suite, "read",      test_map_read);
        CU_add_test(suite, "write",     test_map_write);
        CU_add_test(suite, "event",     test_map_event);
#endif
    }

    return suite;
}

