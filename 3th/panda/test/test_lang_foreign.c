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

static int op_is_true(intptr_t data)
{
    return data != 0;
}

static int op_is_equal(intptr_t data, val_t *av)
{
    if (val_is_number(av)) {
        return data == val_2_double(av);
    } else {
        return 0;
    }
}

static int op_is_gt(intptr_t data, val_t *av)
{
    if (val_is_number(av)) {
        return data > val_2_double(av);
    } else {
        return 0;
    }
}

static int op_is_ge(intptr_t data, val_t *av)
{
    if (val_is_number(av)) {
        return data >= val_2_double(av);
    } else {
        return 0;
    }
}

static int op_is_lt(intptr_t data, val_t *av)
{
    if (val_is_number(av)) {
        return data < val_2_double(av);
    } else {
        return 0;
    }
}

static int op_is_le(intptr_t data, val_t *av)
{
    if (val_is_number(av)) {
        return data <= val_2_double(av);
    } else {
        return 0;
    }
}

static void op_neg(void *env, intptr_t data, val_t *result)
{
    val_set_number(result, data);
}

static void op_not(void *env, intptr_t data, val_t *result)
{
    val_set_number(result, data);
}

static void op_inc(void *env, intptr_t data, val_t *result)
{
    val_set_number(result, data);
}

static void op_incp(void *env, intptr_t data, val_t *result)
{
    val_set_number(result, data + 1);
}

static void op_dec(void *env, intptr_t data, val_t *result)
{
    val_set_number(result, data);
}

static void op_decp(void *env, intptr_t data, val_t *result)
{
    val_set_number(result, data - 1);
}

static void op_mul(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 1);
}

static void op_div(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 2);
}

static void op_mod(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 3);
}

static void op_add(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 4);
}

static void op_sub(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 5);
}

static void op_and(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 6);
}

static void op_or(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 7);
}

static void op_xor(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 8);
}

static void op_rshift(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 9);
}

static void op_lshift(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 10);
}

static void op_elem(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 11);
}

static void op_prop(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, data + 12);
}

static void op_set(void *env, intptr_t data, val_t *av, val_t *result)
{
    val_set_number(result, 0);
}

static val_t *prop_ref(void *env, intptr_t data, val_t *av)
{
    return NULL;
}

static val_t *elem_ref(void *env, intptr_t data, val_t *av)
{
    return NULL;
}

static const val_foreign_op_t test_op = {
    .is_true = op_is_true,
    .is_equal = op_is_equal,
    .is_gt = op_is_gt,
    .is_ge = op_is_ge,
    .is_lt = op_is_lt,
    .is_le = op_is_le,
    .neg = op_neg,
    .not = op_not,
    .inc = op_inc,
    .incp = op_incp,
    .dec = op_dec,
    .decp = op_decp,
    .mul = op_mul,
    .div = op_div,
    .mod = op_mod,
    .add = op_add,
    .sub = op_sub,
    .and = op_and,
    .or  = op_or,
    .xor = op_xor,
    .rshift = op_rshift,
    .lshift = op_lshift,
    .prop = op_prop,
    .elem = op_elem,
    .set = op_set,
    .prop_ref = prop_ref,
    .elem_ref = elem_ref,
};

val_t test_native_foreign(env_t *env, int ac, val_t *av)
{
    intptr_t data = 0;

    if (ac > 0 && val_is_number(av)) {
        data = val_2_double(av);
    }
    return val_create(env, &test_op, data);
}

static void test_foreign_simple(void)
{
    env_t env;
    val_t *res;
    native_t native_entry[] = {
        {"Foreign", test_native_foreign},
    };

    CU_ASSERT_FATAL(0 == interp_env_init_interactive(&env, env_buf, ENV_BUF_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE));
    CU_ASSERT(0 == env_native_set(&env, native_entry, 1));

    CU_ASSERT(0 < interp_execute_string(&env, "var f = Foreign(1)", &res) && val_is_undefined(res));

    CU_ASSERT(0 < interp_execute_string(&env, "f", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "f == 1", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f != 2", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f != 1", &res) && !val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f == 2", &res) && !val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "f > 0", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f >= 1", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f < 2", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f <= 1", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "f == 1", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "-f == 1", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "~f == 1", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "f++ == 1", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "++f == 2", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f-- == 1", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "--f == 0", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "f * 1 == 2", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f / 2 == 3", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f % 4 == 4", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f + 0 == 5", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f - 1 == 6", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "f & 1 == 7", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f | 2 == 8", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f ^ 4 == 9", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "f >> 1 == 10", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f << 1 == 11", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f[0] == 12", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f.a == 13", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "f = 2", &res) && val_is_number(res) && 0 == val_2_double(res));



    //CU_ASSERT(0 < interp_execute_string(&env, "f.is(Foreign)", &res) && val_is_true(res));

    env_deinit(&env);
}

static void test_foreign_gc(void)
{
    env_t env;
    val_t *res;
    native_t native_entry[] = {
        {"Foreign", test_native_foreign},
    };

    CU_ASSERT_FATAL(0 == interp_env_init_interactive(&env, env_buf, ENV_BUF_SIZE, NULL, HEAP_SIZE, NULL, STACK_SIZE));
    CU_ASSERT(0 == env_native_set(&env, native_entry, 1));

    CU_ASSERT(0 < interp_execute_string(&env, "var foreign = Foreign(0)", &res) && val_is_undefined(res));
    CU_ASSERT(0 < interp_execute_string(&env, "foreign", &res) && !val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "var n = 0;", &res) && val_is_undefined(res));
    CU_ASSERT(0 < interp_execute_string(&env, "var b = 'world', c = 'hello';", &res) && val_is_undefined(res));
    CU_ASSERT(0 < interp_execute_string(&env, "var d = c + ' ', e = b + '.', f;", &res) && val_is_undefined(res));
    CU_ASSERT(0 < interp_execute_string(&env, "var a = [b, c, 0];", &res) && val_is_undefined(res));
    CU_ASSERT(0 < interp_execute_string(&env, "var o = {a: b, b: c};", &res) && val_is_undefined(res));
    CU_ASSERT(0 < interp_execute_string(&env, "def add(a, b) {var n = 10; while(n) {n = n-1; a+b} return a + b}", &res) && val_is_function(res));
    CU_ASSERT(0 < interp_execute_string(&env, "def join(){return e + c}", &res) && val_is_function(res));

    CU_ASSERT(0 < interp_execute_string(&env, "a[0] == b", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a[1] == c", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "o.a == b", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "o.b == c", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "while (n < 100) { f = add(d, e); n = n + 1}", &res) && val_is_undefined(res));
    CU_ASSERT(0 < interp_execute_string(&env, "n == 100", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "d", &res) && val_is_string(res));
    CU_ASSERT(0 < interp_execute_string(&env, "e", &res) && val_is_string(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f", &res) && val_is_string(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f == 'hello world.'", &res) && val_is_boolean(res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "n = 0", &res) && val_is_number(res));
    CU_ASSERT(0 < interp_execute_string(&env, "while (n < 1000) { f = join(); n = n + 1}", &res));
    CU_ASSERT(0 < interp_execute_string(&env, "join() == 'world.hello'", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "join() == e + c", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "f == e + c", &res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "n = 0", &res) && val_is_number(res));
    CU_ASSERT(0 < interp_execute_string(&env, "while (n < 1000) { f = o.a + o.b; n = n + 1}", &res));
    CU_ASSERT(0 < interp_execute_string(&env, "f == 'worldhello'", &res) && val_is_boolean(res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "n = 0", &res) && val_is_number(res));
    CU_ASSERT(0 < interp_execute_string(&env, "while (n < 1000) { f = a[0] + a[1]; n = n + 1; a[2] = n;}", &res));
    CU_ASSERT(0 < interp_execute_string(&env, "f == 'worldhello'", &res) && val_is_boolean(res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a[2] == 1000", &res) && val_is_boolean(res) && val_is_true(res));

    CU_ASSERT(0 < interp_execute_string(&env, "b == 'world'", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "c == 'hello'", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "o.a == b", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "o.b == c", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a[0] == b", &res) && val_is_true(res));
    CU_ASSERT(0 < interp_execute_string(&env, "a[1] == c", &res) && val_is_true(res));

    // foreign should keep ok
    CU_ASSERT(0 < interp_execute_string(&env, "foreign", &res) && !val_is_true(res));

    env_deinit(&env);
}

CU_pSuite test_lang_type_foreign(void)
{
    CU_pSuite suite = CU_add_suite("TYPE: foreign", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "foreign simple",       test_foreign_simple);
        CU_add_test(suite, "foreign gc",           test_foreign_gc);
        if (0) {
        }
    }

    return suite;
}

