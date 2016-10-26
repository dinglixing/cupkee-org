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

    // create device 'pio'
    CU_ASSERT(0 == test_cupkee_run_with_reply("var h = device('gpio')\r",    "undefined\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("var c\r",                    "undefined\r\n", 1));

    // get device configs
    test_reply_show(1);
    test_reply_show(0);
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'enable')\r",  "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'select')\r",  "<array>\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'dir')\r",     "\"out\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'mode')\r",    "\"push-pull\"\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("c = config(h, 'other')\r",   "undefined\r\n", 1));

    /*
    // device configure
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'select', PORTB[0])\r", "true\r\n", 1)); // one pin
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'select', [PORTB[0], PORTC[0], PORTB[1], PORTD[15]])\r",
                                              "true\r\n", 1)); // multi pins

    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir',  'out')\r",         "true\r\n", 1)); // set as output
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'push-pull')\r",   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'open-drain')\r",  "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'floating')\r",    "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'pull-up')\r",     "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'pull-down')\r",   "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'other')\r",       "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir', 'in')\r",   "true\r\n", 1)); // set as input
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'push-pull')\r",   "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'open-drain')\r",  "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'floating')\r",    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'pull-up')\r",     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'pull-down')\r",   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'other')\r",       "false\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir', 'io')\r",   "true\r\n", 1));            // dual in output/input
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'open-drain')\r",  "true\r\n", 1));    // only working mode
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'push-pull')\r",   "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'floating')\r",    "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'pull-up')\r",     "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'pull-down')\r",   "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'other')\r",       "false\r\n", 1));

    // enable device
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'enable', true)\r",        "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir', 'in')\r",           "false\r\n", 1));   // nothing can be set while had enable, except 'enable'
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'floating')\r",    "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'pull-up')\r",     "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'pull-down')\r",   "false\r\n", 1));


    // disable device
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'enable', false)\r",       "true\r\n", 1));

    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'dir', 'in')\r",   "true\r\n", 1)); // set as input
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'push-pull')\r",   "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'open-drain')\r",  "false\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'floating')\r",    "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'pull-up')\r",     "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'pull-down')\r",   "true\r\n", 1));
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'mode', 'other')\r",       "false\r\n", 1));

    // re enable device
    CU_ASSERT(0 == test_cupkee_run_with_reply("config(h, 'enable', true)\r",        "true\r\n", 1));
    */
}

static void test_read_write(void)
{
    test_cupkee_reset();

    CU_ASSERT(0 == test_cupkee_start(NULL));
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

CU_pSuite test_pin_entry(void)
{
    CU_pSuite suite = CU_add_suite("misc", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "device",    test_device);
        CU_add_test(suite, "read-write",test_read_write);
        CU_add_test(suite, "event",     test_event);
        CU_add_test(suite, "unref",     test_unref);
    }

    return suite;
}

