
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

#include "lang/type_function.h"
#include "lang/interp.h"


#define STACK_SIZE      128
#define HEAP_SIZE       4096

#define EXE_MEM_SPACE   4096
#define SYM_MEM_SPACE   1024
#define ENV_BUF_SIZE    (sizeof(val_t) * STACK_SIZE + HEAP_SIZE + EXE_MEM_SPACE + SYM_MEM_SPACE)

uint8_t env_buf[ENV_BUF_SIZE];

static int test_setup()
{
    return 0;
}

static int test_clean()
{
    return 0;
}

static val_t ref[4];
static int gc_count = 0;
static void gc_callback()
{
    gc_count++;
}

static val_t test_async_register(env_t *env, int ac, val_t *av)
{
    int i;

    (void) env;
    (void) ac;
    (void) av;

    if (ac > 0) {
        for (i = 0; i < 4; i++) {
            if (val_is_undefined(ref + i)) {
                ref[i] = *av;
                return val_mk_number(i);
            }
        }
    }

    return val_mk_undefined();
}

static val_t test_async_call(env_t *env, val_t *fn)
{
    if (val_is_native(fn)) {
        function_native_t native = (function_native_t) val_2_intptr(fn);
        return native(env, 0, NULL);
    } else {
        env_push_call_function(env, fn);
        return interp_execute_call(env, 0);
    }
}

static void test_async_common(void)
{
    env_t env;
    val_t *res;
    native_t native_entry[] = {
        {"register", test_async_register}
    };
    int i;

    for (i = 0; i < 4; i++) {
        val_set_undefined(ref + i);
    }

    CU_ASSERT_FATAL(0 == interp_env_init_interactive(&env, env_buf, ENV_BUF_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE));
    CU_ASSERT(0 == env_native_set(&env, native_entry, 1));
    CU_ASSERT(0 == env_reference_set(&env, ref, 4));
    CU_ASSERT(0 == env_callback_set(&env, gc_callback));

    CU_ASSERT(0 < interp_execute_string(&env, "register", &res) && val_is_function(res));
    CU_ASSERT(0 < interp_execute_string(&env, "var n = 0, a = [], o = {}, s;", &res) && val_is_undefined(res));
    CU_ASSERT(0 < interp_execute_string(&env, "def fn() {n += 1}", &res) && val_is_function(res));
    CU_ASSERT(0 < interp_execute_string(&env, "register(fn)", &res) && val_is_number(res) && 0 == val_2_double(res));

    for (i = 0; i < 4; i++) {
        val_t *fn = ref + i;
        if (val_is_function(fn)) {
            test_async_call(&env, fn);
            val_set_undefined(fn);
        }
    }
    CU_ASSERT(0 < interp_execute_string(&env, "n == 1", &res) && val_is_true(res));

    env_deinit(&env);
}

CU_pSuite test_lang_async_entry()
{
    CU_pSuite suite = CU_add_suite("lang async execute", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "async common", test_async_common);
    }

    return suite;
}

