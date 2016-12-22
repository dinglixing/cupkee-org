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

#ifndef __TEST_INC__
#define __TEST_INC__

#include "CUnit.h"
#include "CUnit_Basic.h"

#include <hardware.h>
#include <cupkee.h>

int  test_cupkee_reset(void);
int  test_cupkee_start(const char *init);

void test_reply_show(int on);
int  test_cupkee_run_with_reply(const char *input, const char *expected, int try);
int  test_cupkee_run_without_reply(const char *input, int try_max);

CU_pSuite test_hello(void);
CU_pSuite test_util_buffer(void);
CU_pSuite test_system_misc(void);
CU_pSuite test_device_pin(void);
CU_pSuite test_device_adc(void);
CU_pSuite test_device_uart(void);

#endif /* __TEST_INC__ */
