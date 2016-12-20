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
    CU_ASSERT(0 == test_cupkee_run_with_reply("Device('uart', 999)\r",                         "undefined\r\n", 1));

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

static void test_config(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d\r",                                      "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Device('uart')\r",                         "<object>\r\n", 1));

    // config get default setting
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0)\r",                                "9600\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1)\r",                                "8\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2)\r",                                "\"1bit\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(3)\r",                                "\"none\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate')\r",                       "9600\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dataBits')\r",                       "8\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopBits')\r",                       "\"1bit\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity')\r",                         "\"none\"\r\n", 1));

    // return undefined, query a configure not defined
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(5)\r",                                "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('other')\r",                          "undefined\r\n", 1));

    // config set valid
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, 115200)\r",                        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1, 9)\r",                             "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2, 1)\r",                             "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(3, 1)\r",                             "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate')\r",                       "115200\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dataBits')\r",                       "9\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopBits')\r",                       "\"2bit\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity')\r",                         "\"odd\"\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate', 38400)\r",                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dataBits', 8)\r",                    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopBits', '0.5bit')\r",             "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity', 'even')\r",                 "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate')\r",                       "38400\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dataBits')\r",                       "8\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopBits')\r",                       "\"0.5bit\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity')\r",                         "\"even\"\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config({"
                                                "baudrate: 9600,"
                                                "dataBits: 9,"
                                                "stopBits: 0,"
                                                "parity: 0"
                                              "})\r",                                         "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate')\r",                       "9600\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dataBits')\r",                       "9\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopBits')\r",                       "\"1bit\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity')\r",                         "\"none\"\r\n", 1));

    // config set invalid
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, true)\r",                          "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, NaN)\r",                           "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1, 'hello')\r",                       "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1, [])\r",                            "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2, 5)\r",                             "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2, 'other')\r",                       "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(4, 'other')\r",                       "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('other', 1)\r",                       "false\r\n", 1));

    return;
}

static void test_enable(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d, e, h;\r",                               "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Device('uart')\r",                         "<object>\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.instance == 0\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.type == 'uart'\r",                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.category == 'stream'\r",                     "true\r\n", 1));

    // Enable & Disable
    hw_dbg_uart_setup_status_set(0, CUPKEE_OK);

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));

    // return true if device already enabled
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));

    // return false if hardware setup fail
    hw_dbg_uart_setup_status_set(0, CUPKEE_EINVAL);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));

    // return true if hardware setup ok
    hw_dbg_uart_setup_status_set(0, CUPKEE_OK);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));

    // Config set is forbidden, when device is enabled.
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate', 9600)\r",                 "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dataBits', 8)\r",                    "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopBits', 0)\r",                    "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity', 0)\r",                      "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config({"
                                                "baudrate: 9600,"
                                                "dataBits: 9,"
                                                "stopBits: 0,"
                                                "parity: 0"
                                              "})\r",                                         "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({"
                                                "baudrate: 9600,"
                                                "dataBits: 9,"
                                                "stopBits: 0,"
                                                "parity: 0"
                                              "})\r",                                         "false\r\n", 1));

    // Disable & Reset configures
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate', 9600)\r",                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dataBits', 8)\r",                    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopBits', 0)\r",                    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity', 0)\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate')\r",                       "9600\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dataBits')\r",                       "8\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopBits')\r",                       "\"1bit\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity')\r",                         "\"none\"\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config({"
                                                "baudrate: 19200,"
                                                "dataBits: 7,"
                                                "stopBits: 1,"
                                                "parity: 1"
                                              "})\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate')\r",                       "19200\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dataBits')\r",                       "7\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopBits')\r",                       "\"2bit\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity')\r",                         "\"odd\"\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({"
                                                "baudrate: 9600,"
                                                "dataBits: 8,"
                                                "stopBits: 0,"
                                                "parity: 0"
                                              "})\r",                                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate')\r",                       "9600\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('dataBits')\r",                       "8\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopBits')\r",                       "\"1bit\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity')\r",                         "\"none\"\r\n", 1));

    // Enable & Callback
    CU_ASSERT(0 == test_cupkee_run_with_reply("e\r",                                          "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("h\r",                                          "undefined\r\n", 1));

    // callback with handle, if config setting was accept
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({"
                                                "baudrate: 9600,"
                                                "dataBits: 8,"
                                                "stopBits: 0,"
                                                "parity: 0"
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

    // callback with error, if give config and device already enabled
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({"
                                                "baudrate: 9600,"
                                                "dataBits: 8,"
                                                "stopBits: 0,"
                                                "parity: 0"
                                              "}, def (err, hnd) {"
                                                 "if (err) e = err"
                                              "})\r",                                         "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("e < 0\r",                                      "true\r\n", 1));

    // callback with error, if config not acceptable
    hw_dbg_uart_setup_status_set(0, CUPKEE_EINVAL);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable(function (err, hnd) {"
                                                 "if (err) e = true"
                                              "})\r",                                         "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("e\r",                                          "true\r\n", 1));

    test_reply_show(1);
    test_reply_show(0);
    return;
}

CU_pSuite test_device_uart(void)
{
    CU_pSuite suite = CU_add_suite("device uart", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "create",    test_create);
        CU_add_test(suite, "config",    test_config);
        CU_add_test(suite, "enable",    test_enable);
#if 0
        CU_add_test(suite, "read  ",    test_read);
        CU_add_test(suite, "write ",    test_write);
        CU_add_test(suite, "event ",    test_event);
#endif
    }

    return suite;
}

