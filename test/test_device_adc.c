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
    CU_ASSERT(0 == test_cupkee_run_with_reply("var a\r",                                       "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a = Device('adc')\r",                           "<object>\r\n", 1));

    // return undefined, if instance out of range
    CU_ASSERT(0 == test_cupkee_run_with_reply("Device('adc', 999)\r",                          "undefined\r\n", 1));

    // return undefined, if instance already existed
    CU_ASSERT(0 == test_cupkee_run_with_reply("Device('adc')\r",                               "undefined\r\n", 1));

    // return undefined, if type is unknowned
    CU_ASSERT(0 == test_cupkee_run_with_reply("Device('other')\r",                             "undefined\r\n", 1));

    // destroy and recreate
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.destroy())\r",                                "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a = Device('adc', 0)\r",                        "<object>\r\n", 1));

    // properties of device
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.instance\r",                                  "0\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a.type == 'adc'\r",                             "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("a.category == 'map'\r",                         "true\r\n", 1));

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

static void test_config(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d\r",                                      "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Device('adc')\r",                          "<object>\r\n", 1));

    // config get default setting
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)\r",                                "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1)\r",                                "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('channel')\r",                        "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('interval')\r",                       "0\r\n", 1));

    // return undefined, query a configure not defined
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2)\r",                                "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('other')\r",                          "undefined\r\n", 1));


    // config set valid
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, 1)\r",                             "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1, 1)\r",                             "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)\r",                                "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)[0]\r",                             "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0).length()\r",                       "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1)\r",                                "1\r\n", 1));


    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('channel',  4)\r",                    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('interval', 100)\r",                  "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)\r",                                "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)[0]\r",                             "4\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0).length()\r",                       "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1)\r",                                "100\r\n", 1));


    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('channel', [3, 1, 4, 5])\r",          "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)\r",                                "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0).length()\r",                       "4\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)[0]\r",                             "3\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)[1]\r",                             "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)[2]\r",                             "4\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)[3]\r",                             "5\r\n", 1));


    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config({"
                                                "channel: [0, 1],"
                                                "interval: 10"
                                              "})\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)\r",                                "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0).length()\r",                       "2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)[0]\r",                             "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)[1]\r",                             "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1)\r",                                "10\r\n", 1));

    // config set invalid
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, true)\r",                          "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, NaN)\r",                           "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, [])\r",                            "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, [1, 2, 3, 4, 5])\r",               "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1, 'hello')\r",                       "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1, [])\r",                            "false\r\n", 1));

    return;
}

static void test_enable(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d, e, h;\r",                               "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Device('adc')\r",                          "<object>\r\n", 1));

    // Enable & Disable
    hw_dbg_adc_setup_status_set(0, CUPKEE_OK);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));

    hw_dbg_adc_setup_status_set(0, CUPKEE_EINVAL);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "false\r\n", 1));
    hw_dbg_adc_setup_status_set(0, CUPKEE_OK);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));

    // Config set is forbidden, when device is enabled.
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('channel', 4)\r",                     "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('interval', 2)\r",                    "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config({"
                                                "channel: 1,"
                                                "interval: 2"
                                              "})\r",                                         "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({"
                                                "channel: 2,"
                                                "interval: 3"
                                              "})\r",                                         "false\r\n", 1));

    // Disable & Reset configures
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('channel', 4)\r",                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('interval', 2)\r",                    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config({"
                                                "channel: 1,"
                                                "interval: 2"
                                              "})\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({"
                                                "channel: 2,"
                                                "interval: 3"
                                              "})\r",                                         "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)[0]\r",                             "2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1)\r",                                "3\r\n", 1));

    // Enable & Callback
    CU_ASSERT(0 == test_cupkee_run_with_reply("e\r",                                          "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h\r",                                          "undefined\r\n", 1));

    // callback with handle, if config setting was accept
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({"
                                                "channel: 2,"
                                                "interval: 3"
                                              "}, def (err, hnd) {"
                                                 "if (err) e = err else h = hnd"
                                              "})\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("e\r",                                          "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h == d\r",                                     "true\r\n", 1));

    // callback with handle, if device already enabled
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable(def (err, hnd) {"
                                                 "if (err) e = err else h = true"
                                              "})\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("e\r",                                          "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h\r",                                          "true\r\n", 1));

    // callback with error, if device already enabled
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({"
                                                "channel: 2,"
                                                "interval: 3"
                                              "}, def (err, hnd) {"
                                                 "if (err) e = err"
                                              "})\r",                                         "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("e < 0\r",                                      "true\r\n", 1));

    // callback with error, if config not acceptable
    hw_dbg_pin_setup_status_set(0, CUPKEE_EINVAL);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({"
                                                 "channel: true,"
                                                 "other: 5"
                                              "}, def (err, hnd) {"
                                                 "if (err) e = err"
                                              "})\r",                                         "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("e < 0\r",                                      "true\r\n", 1));

    return;
}

static void test_read(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d;\r",                                     "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Device('adc')\r",                          "<object>\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(0)\r",                                  "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read()\r",                                   "undefined\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({channel: 0})\r",                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));

    // get undefined, when not converted
    //
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(0)\r",                                  "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(1)\r",                                  "undefined\r\n", 1));

    hw_dbg_adc_update(0, 0, 1);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read()\r",                                   "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(0)\r",                                  "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[0]\r",                                       "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[1]\r",                                       "undefined\r\n", 1));
}

static void test_write(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d;\r",                                     "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Device('adc')\r",                          "<object>\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(15)\r",                                "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(0, 1)\r",                              "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({pinNum: 4})\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));

    // write is forbidden
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(0)\r",                                 "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(0, 1)\r",                              "false\r\n", 1));

    return;
}

static void test_event(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d, e, v, i;\r",                            "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Device('adc')\r",                          "<object>\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read()\r",                                   "undefined\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.listen('data', function (data, idx) {"
                                                "v = data;"
                                                "i = idx;"
                                              "})\r",                                         "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({channel: [4, 1]})\r",                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));

    hw_dbg_adc_update(0, 0, 100);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v == 100\r",                                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("i == 0\r",                                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[0]\r",                                       "100\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[1]\r",                                       "undefined\r\n", 1));

    hw_dbg_adc_update(0, 1, 999);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v == 999\r",                                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("i == 1\r",                                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[0]\r",                                       "100\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d[1]\r",                                       "999\r\n", 1));
    return;
}

CU_pSuite test_device_adc(void)
{
    CU_pSuite suite = CU_add_suite("device adc", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "create",    test_create);
        CU_add_test(suite, "config",    test_config);
        CU_add_test(suite, "enable",    test_enable);
        CU_add_test(suite, "read  ",    test_read);
        CU_add_test(suite, "write ",    test_write);
        CU_add_test(suite, "event ",    test_event);
    }

    return suite;
}

