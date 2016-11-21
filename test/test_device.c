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

    // create device 'map'
    CU_ASSERT(0 == test_cupkee_run_with_reply("var a, b, c\r",                                      "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a = xxx('other')\r",                                 "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("a = xxx('map')\r",                                   "<object>\r\n", 1));

    // create device indicate instance
    CU_ASSERT(0 == test_cupkee_run_with_reply("b = xxx('map', 0)\r",                                "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b = xxx('map', 2)\r",                                "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b = xxx('map', 1)\r",                                "<object>\r\n", 1));

    // destroy and recreate
    CU_ASSERT(0 == test_cupkee_run_with_reply("a.destroy())\r",                                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("b.destroy())\r",                                     "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("xxx('map', 0)\r",                                    "<object>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("xxx('map', 1)\r",                                    "<object>\r\n", 1));
}

static void test_config(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d, c\r",                                    "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = xxx('map')\r",                              "<object>\r\n", 1));

    /***********************************
     * set
     * return value:
     *   true: ok
     *   false: setting not be accepted
     *   undefined: attribute not exist
     * get
     * return value:
     *   number | string: current setting
     *   undefined: attribute not exist
    ***********************************/

    /* always return undefined, when get/set */
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('other')\r",                          "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('other', true)\r",                    "undefined\r\n", 1));

    /* boolean attribute*/
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool', true)\r",                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool', false)\r",                    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool', 0)\r",                        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool', 1)\r",                        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool')\r",                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool', NaN)\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool')\r",                           "false\r\n", 1));

    /* number attribure */
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number', 0)\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number')\r",                         "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number', -1)\r",                     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number')\r",                         "-1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number', 1)\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number')\r",                         "1\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number', NaN)\r",                    "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number', false)\r",                  "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number')\r",                         "1\r\n", 1));

    /* number attribure */
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option', 0)\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option')\r",                         "\"x\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option', 'yy')\r",                   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option')\r",                         "\"yy\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option', 2)\r",                      "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option')\r",                         "\"zzz\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option', NaN)\r",                    "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option', false)\r",                  "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option')\r",                         "\"zzz\"\r\n", 1));

    /* combine */
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config({\
  bool: true,\
  number: 10,\
  option: 'yy'\
})\r",                                                                                        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool')\r",                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number')\r",                         "10\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option')\r",                         "\"yy\"\r\n", 1));
}

static void test_enable(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d;\r",                                     "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = xxx('map')\r",                             "<object>\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool')\r",                           "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number')\r",                         "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option')\r",                         "\"x\"\r\n", 1));

    // return is enable, without argument
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable(1)\r",                                "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable(0)\r",                                "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({\
  bool: true,\
  number: 10,\
  option: 'yy'\
})\r",                                                                                        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('bool')\r",                           "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('number')\r",                         "10\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('option')\r",                         "\"yy\"\r\n", 1));

}

static void test_map_read(void)
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
    test_reply_show(1);
    test_reply_show(0);
}

static void test_map_write(void)
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

static void test_map_event(void)
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

static void test_stream_read(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d, v, s = '12345'\r",                      "undefined\r\n", 1));

    // create device
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = xxx('stream'))\r",                         "<object>\r\n", 1));

    // get default configure
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate'))\r",                      "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopbits'))\r",                      "\"1\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity'))\r",                        "\"none\"\r\n", 1));

    // enable
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable({\
    baudrate: 115200\
})\r",                                                                                        "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.read(5)\r",                                  "true\r\n", 1));

    hw_dbg_stream_set_input("12345");
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("\
d.read(5, function(err, data) {\
  if (!err) v = data;\
})\r",                                                                                        "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v\r",                                          "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v.length()\r",                                 "5\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v[0] == 49\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v[1] == 50\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v[2] == 51\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v[3] == 52\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v[4] == 53\r",                                 "true\r\n", 1));
    test_reply_show(1);
    test_reply_show(0);
}

static void test_stream_write(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));

    CU_ASSERT(0 == test_cupkee_run_with_reply("var d, v, buf, info\r",                       "undefined\r\n", 1));

    // create device
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = xxx('stream'))\r",                         "<object>\r\n", 1));

    // set & get configure
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(0, 9600))\r",                         "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(1, 5)) // option not defined\r",      "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(2, 1))\r",                            "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config(3, 0)) //item not defined\r",         "undefined\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('baudrate'))\r",                      "9600\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('stopbits'))\r",                      "\"1\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.config('parity'))\r",                        "\"odd\"\r\n", 1));

    // enable
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable(true)\r",                             "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.enable()\r",                                 "true\r\n", 1));

    // write & cause send buffer full
    CU_ASSERT(0 == test_cupkee_run_with_reply("buf = Buffer(100)\r",                          "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("buf.length()\r",                               "100\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write(buf) > 0\r",                           "true\r\n", 1));

    // can't send any, when send buffer is fulled
    CU_ASSERT(0 == test_cupkee_run_with_reply("\
d.write('hello', function(err, n, data) {\
    if (!err) info = [n, data]\
})\r",                                                                                        "0\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("info[0] == 0\r",                               "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("info[1] == 'hello'\r",                         "true\r\n", 1));

    hw_dbg_stream_set_send(5);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("\
d.write(buf, function(err, n, data) {\
    if (!err) info = [n, data]\
})\r",                                                                                        "5\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("info[0]\r",                                    "5\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("info[1]\r",                                    "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("info[1].length()\r",                           "100\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("info[1] == buf\r",                             "true\r\n", 1));

    hw_dbg_stream_set_send(50);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("\
d.write(buf, 16, function(err, n, data) {\
    if (!err) info = [n, data]\
})\r",                                                                                        "16\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("info[0]\r",                                    "16\r\n", 1));
}

static void test_stream_event(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));


    CU_ASSERT(0 == test_cupkee_run_with_reply("var d, v, e, s\r",                             "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("d = xxx('stream')\r",                          "<object>\r\n", 1));

    // enable & listen
    CU_ASSERT(0 == test_cupkee_run_with_reply("\
d.enable({\
  baudrate: 19200 \
}, function(err, dev) { \
  if (!err) { \
    dev.listen('error', function(err) { \
      e = err; \
    });\
    dev.listen('data', function(data) { \
      v = data; \
    });\
    dev.listen('drain', function() { \
      s = true; \
    });\
  }\
})\r",                                                                                        "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("e\r",                                          "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v\r",                                          "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("s\r",                                          "undefined\r\n", 1));

    hw_dbg_stream_set_error(0, 8);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("e == 8\r",                                     "true\r\n", 1));

    // trigger data event
    hw_dbg_stream_set_input("12345");
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v\r",                                          "<buffer>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v.length()\r",                                 "5\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v[0] == 49\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v[1] == 50\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v[2] == 51\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v[3] == 52\r",                                 "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("v[4] == 53\r",                                 "true\r\n", 1));

    // trigger drain event
    CU_ASSERT(0 == test_cupkee_run_with_reply("d.write('hello')\r",                           "5\r\n", 1));
    hw_dbg_stream_set_send(50);
    CU_ASSERT(0 == test_cupkee_run_without_reply(NULL, 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("s\r",                                          "true\r\n", 1));

    test_reply_show(1);
    test_reply_show(0);
}

CU_pSuite test_devices(void)
{
    CU_pSuite suite = CU_add_suite("device", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "create",    test_create);
        CU_add_test(suite, "config",    test_config);
        CU_add_test(suite, "enable",    test_enable);

        CU_add_test(suite, "map read",      test_map_read);
        CU_add_test(suite, "map write",     test_map_write);
        CU_add_test(suite, "map event",     test_map_event);

        CU_add_test(suite, "stream read",   test_stream_read);
        CU_add_test(suite, "stream write",  test_stream_write);
        CU_add_test(suite, "stream event",  test_stream_event);
#if 0
#endif
    }

    return suite;
}

