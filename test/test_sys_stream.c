/*
MIT License

This file is part of cupkee project

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

static uint8_t implement_load_trigger_cnt;
static uint8_t implement_data;

static int test_setup(void)
{
    cupkee_memory_desc_t descs[2] = {
        {64, 4}, {256, 4}
    };

    cupkee_memory_init(2, descs);

    return 0;
}

static int test_clean(void)
{
    return 0;
}

static void read_implement_idle(cupkee_stream_t *s, size_t n)
{
    (void) s;
    (void) n;
}

static void read_implement_immediately(cupkee_stream_t *s, size_t n)
{
    (void) n;

    while (1 == cupkee_stream_push(s, 1, &implement_data))
        ;
}

static void read_implement_trigger(cupkee_stream_t *s, size_t n)
{
    (void) s;
    (void) n;
    implement_load_trigger_cnt++;
}

static int read_implement_load(cupkee_stream_t *s)
{
    int count = 0;
    while (implement_load_trigger_cnt) {
        if (1 != cupkee_stream_push(s, 1, &implement_data)) {
            implement_load_trigger_cnt = 0;
            break;
        }
        count++;
    }
    return count;
}

static void test_readable_init(void)
{
    cupkee_event_emitter_t emitter;
    cupkee_stream_t stream;

    CU_ASSERT(0 <= cupkee_event_emitter_init(&emitter, NULL));

    CU_ASSERT(0 != cupkee_stream_init_readable(NULL, &emitter, 32, read_implement_idle));
    CU_ASSERT(0 != cupkee_stream_init_readable(&stream, &emitter, 32, NULL));

    // init without emitter
    CU_ASSERT(0 == cupkee_stream_init_readable(&stream, NULL, 32, read_implement_idle));

    // init with emitter
    CU_ASSERT(0 == cupkee_stream_init_readable(&stream, &emitter, 32, read_implement_idle));
}

static void test_read_nopush(void)
{
    cupkee_stream_t stream;
    uint8_t buf[32];

    CU_ASSERT(0 == cupkee_stream_init_readable(&stream, NULL, 32, read_implement_idle));

    CU_ASSERT(0 == cupkee_stream_read(&stream, 10, buf));
}

static void test_read_immediately(void)
{
    cupkee_stream_t stream;
    uint8_t buf[30];

    // fill rx cache with 1
    implement_data = 1;

    CU_ASSERT(0 == cupkee_stream_init_readable(&stream, NULL, 30, read_implement_immediately));

    // 0 should be return, if want more then rx_size_max
    CU_ASSERT(0 == cupkee_stream_read(&stream, 100, buf));

    memset(buf, 0, 30);

    // appends 2 to rx cache, after read
    implement_data = 2;
    CU_ASSERT(10 == cupkee_stream_read(&stream, 10, buf));
    CU_ASSERT(buf[0] == 1 && buf[9] == 1);

    // appends 3 to rx cache, after read
    implement_data = 3;
    CU_ASSERT(10 == cupkee_stream_read(&stream, 10, buf + 10));
    CU_ASSERT(buf[10] == 1 && buf[19] == 1);

    memset(buf, 0, 30);
    CU_ASSERT(30 == cupkee_stream_read(&stream, 30, buf));
    CU_ASSERT(buf[0] == 1 && buf[9] == 1);
    CU_ASSERT(buf[10] == 2 && buf[19] == 2);
    CU_ASSERT(buf[20] == 3 && buf[29] == 3);
}

static void test_read_trigger(void)
{
    cupkee_stream_t stream;
    uint8_t buf[32];

    CU_ASSERT(0 == cupkee_stream_init_readable(&stream, NULL, 32, read_implement_trigger));

    implement_load_trigger_cnt = 0;
    // rx cache is empty, trigger load
    CU_ASSERT(0 == cupkee_stream_read(&stream, 10, buf));
    CU_ASSERT(1 == implement_load_trigger_cnt);

    // full stream rx buffer with 1
    implement_data = 1;
    CU_ASSERT(32 == read_implement_load(&stream));
    CU_ASSERT(0 == implement_load_trigger_cnt);

    memset(buf, 0, 32);
    CU_ASSERT(10 == cupkee_stream_read(&stream, 10, buf));
    CU_ASSERT(buf[0] == 1 && buf[9] == 1);
    // load should be trigger, since full cache be read
    CU_ASSERT(1 == implement_load_trigger_cnt);

    memset(buf, 0, 32);
    CU_ASSERT(10 == cupkee_stream_read(&stream, 10, buf));
    CU_ASSERT(buf[0] == 1 && buf[9] == 1);
    // load should not be trigger
    CU_ASSERT(1 == implement_load_trigger_cnt);

    // Cached less than want, should return nothing
    memset(buf, 0, 32);
    CU_ASSERT(0 == cupkee_stream_read(&stream, 13, buf));
    CU_ASSERT(buf[0] == 0 && buf[9] == 0);

    // load should not be trigger
    CU_ASSERT(1 == implement_load_trigger_cnt);

    CU_ASSERT(20 == read_implement_load(&stream));
    CU_ASSERT(0 == implement_load_trigger_cnt);


    // Cached equal want
    // Should trigger load ?
    CU_ASSERT(32 == cupkee_stream_read(&stream, 32, buf));

    CU_ASSERT(32 == read_implement_load(&stream));
}

CU_pSuite test_sys_stream(void)
{
    CU_pSuite suite = CU_add_suite("system stream", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "readable init",     test_readable_init);
        CU_add_test(suite, "read no push",      test_read_nopush);
        CU_add_test(suite, "read immediately",  test_read_immediately);
        CU_add_test(suite, "read trigger",      test_read_trigger);
    }

    return suite;
}


