/*
MIT License

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

#include "cunit/CUnit.h"
#include "cunit/CUnit_Basic.h"

#include "lang/interp.h"
#include "lang/type_buffer.h"

#define STACK_SIZE      128
#define HEAP_SIZE       4096

#define EXE_MEM_SPACE   4096
#define SYM_MEM_SPACE   1024
#define MEMORY_SIZE     (sizeof(val_t) * STACK_SIZE + HEAP_SIZE + EXE_MEM_SPACE + SYM_MEM_SPACE)

uint8_t memory[MEMORY_SIZE];

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
    env_t env;
    val_t *res;
    native_t native_entry[] = {
        {"Buffer", buffer_native_create},
    };

    CU_ASSERT_FATAL(0 == interp_env_init_interactive(&env, memory, MEMORY_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE));
    CU_ASSERT(0 == env_native_set(&env, native_entry, 1));

    CU_ASSERT(0 < interp_execute_string(&env, "var a, b;", &res)   && val_is_undefined(res));

    CU_ASSERT(0 < interp_execute_string(&env, "a = Buffer(10);", &res) && val_is_buffer(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a.length() == 10;", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "b = Buffer('hello');", &res) && val_is_buffer(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b.length() == 5", &res) && val_is_true(res));


    CU_ASSERT(0 < interp_execute_string(&env, "b[0] == 104", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b[1] == 101", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b[2] == 108", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b[3] == 108", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b[4] == 111", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b[5]", &res) && val_is_undefined(res));
}

static void test_write(void)
{
    env_t env;
    val_t *res;
    native_t native_entry[] = {
        {"Buffer", buffer_native_create},
    };

    CU_ASSERT_FATAL(0 == interp_env_init_interactive(&env, memory, MEMORY_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE));
    CU_ASSERT(0 == env_native_set(&env, native_entry, 1));

    CU_ASSERT(0 < interp_execute_string(&env, "var b;", &res)   && val_is_undefined(res));

    CU_ASSERT(0 < interp_execute_string(&env, "b = Buffer(10);", &res) && val_is_buffer(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b.length() == 10;", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "b.writeInt(-1) == 1;", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b.writeInt(-1, 2) == 3;", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "b.writeInt(-1, 0, 2) == 2;", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b.writeInt(-1, 0, 4) == 4;", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "b.writeInt(-1, 6, 4) == 10;", &res) && val_is_true(res));
}

static void test_read(void)
{
    env_t env;
    val_t *res;
    native_t native_entry[] = {
        {"Buffer", buffer_native_create},
    };

    CU_ASSERT_FATAL(0 == interp_env_init_interactive(&env, memory, MEMORY_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE));
    CU_ASSERT(0 == env_native_set(&env, native_entry, 1));

    CU_ASSERT(0 < interp_execute_string(&env, "var a, b;", &res)   && val_is_undefined(res));

    CU_ASSERT(0 < interp_execute_string(&env, "a = Buffer(10);", &res) && val_is_buffer(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a.length() == 10;", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "b = Buffer('hello');", &res) && val_is_buffer(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b.length() == 5", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "b.readInt(0) == 104", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b.readInt(0, 2) == 25960", &res) && val_is_true(res)); // Little Endian
    CU_ASSERT(0 < interp_execute_string(&env, "b.readInt(0, 2, 0) == 25960", &res) && val_is_true(res)); // Little Endian
    CU_ASSERT(0 < interp_execute_string(&env, "b.readInt(0, 2, 1) == 26725", &res) && val_is_true(res)); // Big Endian
    CU_ASSERT(0 < interp_execute_string(&env, "b.readInt(0, 4) == 1819043176", &res) && val_is_true(res)); // Big Endian
    CU_ASSERT(0 < interp_execute_string(&env, "b.readInt(0, 4, 0) == 1819043176", &res) && val_is_true(res)); // Big Endian
    CU_ASSERT(0 < interp_execute_string(&env, "b.readInt(0, 4, 1) == 1751477356", &res) && val_is_true(res)); // Little Endian

    CU_ASSERT(0 < interp_execute_string(&env, "a.writeInt(-1) == 1;", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a.writeInt(-2, 1) == 2;", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a.readInt() == -1", &res) && val_is_true(res)); // Little Endian
    CU_ASSERT(0 < interp_execute_string(&env, "a.readInt(0, 1) == -1", &res) && val_is_true(res)); // Little Endian
    CU_ASSERT(0 < interp_execute_string(&env, "a.readInt(1, 1) == -2", &res) && val_is_true(res)); // Little Endian

    CU_ASSERT(0 < interp_execute_string(&env, "a.writeInt(-3, 2, 2) == 4;", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a.readInt(2, 2) == -3", &res) && val_is_true(res)); // Little Endian
    CU_ASSERT(0 < interp_execute_string(&env, "a.writeInt(-4, 4, 4) == 8;", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a.readInt(4, 4) == -4", &res) && val_is_true(res)); // Little Endian

    CU_ASSERT(0 < interp_execute_string(&env, "a.writeInt(-3, 2, 2, 1) == 4;", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a.readInt(2, 2) != -3", &res) && val_is_true(res)); // Little Endian
    CU_ASSERT(0 < interp_execute_string(&env, "a.readInt(2, 2, 1) == -3", &res) && val_is_true(res)); // Little Endian

    CU_ASSERT(0 < interp_execute_string(&env, "a.writeInt(-4, 4, 4, 1) == 8;", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a.readInt(4, 4) != -4", &res) && val_is_true(res)); // Little Endian
    CU_ASSERT(0 < interp_execute_string(&env, "a.readInt(4, 4, 1) == -4", &res) && val_is_true(res)); // Little Endian
}

static void test_slice(void)
{
    env_t env;
    val_t *res;
    native_t native_entry[] = {
        {"Buffer", buffer_native_create},
    };

    CU_ASSERT_FATAL(0 == interp_env_init_interactive(&env, memory, MEMORY_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE));
    CU_ASSERT(0 == env_native_set(&env, native_entry, 1));

    CU_ASSERT(0 < interp_execute_string(&env, "var b, d;", &res)   && val_is_undefined(res));

    CU_ASSERT(0 < interp_execute_string(&env, "b = Buffer('hello');", &res) && val_is_buffer(res));
    CU_ASSERT(0 < interp_execute_string(&env, "b.length() == 5", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "d = b.slice(1)", &res) && val_is_buffer(res));
    CU_ASSERT(0 < interp_execute_string(&env, "d.length() == 4", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "d = b.slice(1, 3)", &res) && val_is_buffer(res));
    CU_ASSERT(0 < interp_execute_string(&env, "d.length() == 2", &res) && val_is_true(res));
}

CU_pSuite test_lang_type_buffer(void)
{
    CU_pSuite suite = CU_add_suite("TYPE: Buffer", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "create", test_create);
        CU_add_test(suite, "write",  test_write);
        CU_add_test(suite, "read",   test_read);
        CU_add_test(suite, "slice",  test_slice);
    }
    return suite;
}

