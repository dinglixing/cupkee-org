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
}

static void test_read(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d, err, data;\r",                          "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Device('uart')\r",                         "<object>\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.instance == 0\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.type == 'uart'\r",                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.category == 'stream'\r",                     "true\r\n", 1));

    // Always false, when device is not enabled
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read()\r",                                   "false\r\n", 1));

    // Enable device
    hw_dbg_uart_setup_status_set(0, CUPKEE_OK);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));

    // Data received clean
    CU_ASSERT(0 == test_cupkee_run_with_reply("function cb(e, d) {"
                                                "if (e) {"
                                                  "err = e;"
                                                "} else {"
                                                  "data = d;"
                                                "}"
                                              "}\r",                                          "<function>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read()\r",                                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(999)\r",                                "true\r\n", 1));

    // read device no received
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.received\r",                                 "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(cb)\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data\r",                                       "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data.length()\r",                              "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(1, cb)\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data\r",                                       "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data.length()\r",                              "0\r\n", 1));

    // read device had received
    hw_dbg_uart_data_give(0, "0123456789");
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.received\r",                                 "10\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(2, cb)\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.received\r",                                 "8\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data\r",                                       "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data.length()\r",                              "2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data[0]\r",                                    "48\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data[1]\r",                                    "49\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(1)\r",                                  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.received\r",                                 "7\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(5, cb)\r",                              "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data\r",                                       "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data.length()\r",                              "5\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data.toString() == '34567'\r",                 "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(cb)\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.received\r",                                 "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data.length()\r",                              "2\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data[0]\r",                                    "56\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data[1]\r",                                    "57\r\n", 1));

    hw_dbg_uart_data_give(0, "0123456789");
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.received\r",                                 "10\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read()\r",                                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.received\r",                                 "0\r\n", 1));

    // error
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(cb)\r",                                 "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("err < 0\r",                                    "true\r\n", 1));
}

static void test_write(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d, buf, err, data, send;\r",               "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Device('uart')\r",                         "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("buf = Buffer('0123456789')\r",                 "<buffer>\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.instance == 0\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.type == 'uart'\r",                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.category == 'stream'\r",                     "true\r\n", 1));

    // Always false, when device is not enabled
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write()\r",                                  "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("function cb(e, d, n) {"
                                                "if (e) {"
                                                  "err = e;"
                                                "} else {"
                                                  "data = d;"
                                                  "send = n;"
                                                "}"
                                              "}\r",                                          "<function>\r\n", 1));

    // Enable device
    hw_dbg_uart_setup_status_set(0, CUPKEE_OK);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));

    // Write
    hw_dbg_uart_send_state(0, 1);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(buf)\r",                               "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(10 == hw_dbg_uart_data_take(0, 10));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(buf, 20)\r",                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(10 == hw_dbg_uart_data_take(0, 20));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(buf, 7)\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(7 == hw_dbg_uart_data_take(0, 20));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(buf, 3, 20)\r",                        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(7 == hw_dbg_uart_data_take(0, 20));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(buf, cb)\r",                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data == buf\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("send == 10\r",                                 "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(buf, 7, cb)\r",                        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data == buf\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("send == 7\r",                                  "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(buf, 7, 20, cb)\r",                    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data == buf\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("send == 3\r",                                  "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(buf, 10, 20, cb)\r",                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data == buf\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("send == 0\r",                                  "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(20 == hw_dbg_uart_data_take(0, 20));

    // false write
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(cb)\r",                                "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("err < 0\r",                                    "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("err = 0\r",                                    "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(buf, -1, cb)\r",                       "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("err < 0\r",                                    "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("err = 0\r",                                    "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.disable()\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(cb)\r",                                "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("err < 0\r",                                    "true\r\n", 1));

    return;
    test_reply_show(1);
    test_reply_show(0);
}

static void test_event(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d, buf, err, data, send;\r",               "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = Device('uart')\r",                         "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("buf = Buffer('0123456789')\r",                 "<buffer>\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.instance == 0\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.type == 'uart'\r",                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.category == 'stream'\r",                     "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "false\r\n", 1));

    // Enable device
    hw_dbg_uart_setup_status_set(0, CUPKEE_OK);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.isEnabled()\r",                              "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.listen(0, def(e) err = e)\r",                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.listen(1, def(d) data = d)\r",               "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.listen(2, def(v) send = v)\r",               "true\r\n", 1));

    hw_dbg_set_systicks(0);
    hw_dbg_uart_data_give(0, "0123456789");
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.received\r",                                 "10\r\n", 1));
    hw_dbg_set_systicks(20);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data\r",                                       "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("data.toString() == '0123456789'\r",            "true\r\n", 1));

    return;
    test_reply_show(1);
    test_reply_show(0);
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read()\r",                                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(999)\r",                                "true\r\n", 1));


    hw_dbg_set_systicks(0);
    hw_dbg_set_systicks(20);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
}


CU_pSuite test_device_uart(void)
{
    CU_pSuite suite = CU_add_suite("device uart", test_setup, test_clean);

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

