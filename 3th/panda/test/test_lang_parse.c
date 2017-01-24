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

#include "test_util.h"

#include "lang/lex.h"
#include "lang/parse.h"

#define L_ ast_expr_lft
#define R_ ast_expr_rht
#define TEXT ast_expr_text
#define NUMBER ast_expr_num
#define STMT ast_expr_stmt

#define PSR_BUF_SIZE    8192
static uint8_t heap_buf[PSR_BUF_SIZE];

static int test_setup()
{
    return 0;
}

static int test_clean()
{
    return 0;
}

static void test_expr_factor(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, " [a, b] \n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_ARRAY);
    CU_ASSERT(L_(expr) && ast_expr_type(L_((expr))) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));
    CU_ASSERT(R_(expr) && ast_expr_type(R_((expr))) == EXPR_ID && !strcmp("b", TEXT(R_(expr))));

    parse_init(&psr, " true false \n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_TRUE);

    CU_ASSERT(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_FALSE);

    parse_init(&psr, " undefined null NaN \n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_UND);

    CU_ASSERT(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_NULL);

    CU_ASSERT(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_NAN);

    parse_init(&psr, " 'null' hello 12345\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_STRING && !strcmp("null", TEXT(expr)));

    CU_ASSERT(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_ID && !strcmp("hello", TEXT(expr)));

    CU_ASSERT(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_NUM && NUMBER(expr) == 12345);

    parse_init(&psr, " (a + b) \n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_ADD);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID && !strcmp("b", TEXT(R_(expr))));

    parse_init(&psr, " {a: 1, b : 2} \n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_DICT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_PAIR);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_PAIR);

    CU_ASSERT(L_(L_(expr)) && ast_expr_type(L_(L_((expr)))) == EXPR_ID && !strcmp("a", TEXT(L_(L_(expr)))));
    CU_ASSERT(R_(L_(expr)) && ast_expr_type(R_(L_((expr)))) == EXPR_NUM && 1 == NUMBER(R_(L_(expr))));
    CU_ASSERT(L_(R_(expr)) && ast_expr_type(L_(R_((expr)))) == EXPR_ID && !strcmp("b", TEXT(L_(R_(expr)))));
    CU_ASSERT(R_(R_(expr)) && ast_expr_type(R_(R_((expr)))) == EXPR_NUM && 2 == NUMBER(R_(R_(expr))));
}

static void test_expr_primary(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, " a.b a.b.c \n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_PROP);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID && !strcmp("b", TEXT(R_(expr))));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_PROP);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID && !strcmp("c", TEXT(R_(expr))));
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_PROP);
    CU_ASSERT(L_(L_(expr)) && ast_expr_type(L_(L_(expr))) == EXPR_ID && !strcmp("a", TEXT(L_(L_(expr)))));
    CU_ASSERT(R_(L_(expr)) && ast_expr_type(R_(L_(expr))) == EXPR_ID && !strcmp("b", TEXT(R_(L_(expr)))));

    parse_init(&psr, " a[b] a[b][c] \n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_ELEM);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID && !strcmp("b", TEXT(R_(expr))));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_ELEM);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID && !strcmp("c", TEXT(R_(expr))));
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ELEM);
    CU_ASSERT(L_(L_(expr)) && ast_expr_type(L_(L_(expr))) == EXPR_ID && !strcmp("a", TEXT(L_(L_(expr)))));
    CU_ASSERT(R_(L_(expr)) && ast_expr_type(R_(L_(expr))) == EXPR_ID && !strcmp("b", TEXT(R_(L_(expr)))));

    parse_init(&psr, " a(b) a(b)(c) \n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_CALL);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID && !strcmp("b", TEXT(R_(expr))));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_CALL);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID && !strcmp("c", TEXT(R_(expr))));
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_CALL);
    CU_ASSERT(L_(L_(expr)) && ast_expr_type(L_(L_(expr))) == EXPR_ID && !strcmp("a", TEXT(L_(L_(expr)))));
    CU_ASSERT(R_(L_(expr)) && ast_expr_type(R_(L_(expr))) == EXPR_ID && !strcmp("b", TEXT(R_(L_(expr)))));

    // f(a)[b].c[d](e)
    parse_init(&psr, "f(a)[b].c[d](e)\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_CALL);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID && !strcmp("e", TEXT(R_(expr))));
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ELEM);
    CU_ASSERT(R_(L_(expr)) && ast_expr_type(R_(L_(expr))) == EXPR_ID && !strcmp("d", TEXT(R_(L_(expr)))));
    CU_ASSERT(L_(L_(expr)) && ast_expr_type(L_(L_(expr))) == EXPR_PROP);
    CU_ASSERT(R_(L_(L_(expr))) && ast_expr_type(R_(L_(L_(expr)))) == EXPR_ID && !strcmp("c", TEXT(R_(L_(L_(expr))))));
    CU_ASSERT(L_(L_(L_(expr))) && ast_expr_type(L_(L_(L_(expr)))) == EXPR_ELEM);
    CU_ASSERT(R_(L_(L_(L_(expr)))) && ast_expr_type(R_(L_(L_(L_(expr))))) == EXPR_ID && !strcmp("b", TEXT(R_(L_(L_(L_(expr)))))));
    CU_ASSERT(L_(L_(L_(L_(expr)))) && ast_expr_type(L_(L_(L_(L_(expr))))) == EXPR_CALL);
    CU_ASSERT(R_(L_(L_(L_(L_(expr))))) && ast_expr_type(R_(L_(L_(L_(L_(expr)))))) == EXPR_ID && !strcmp("a", TEXT(R_(L_(L_(L_(L_(expr))))))));
    CU_ASSERT(L_(L_(L_(L_(L_(expr))))) && ast_expr_type(L_(L_(L_(L_(L_(expr)))))) == EXPR_ID && !strcmp("f", TEXT(L_(L_(L_(L_(L_(expr))))))));
}

static void test_expr_selfop(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "a++", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_INC);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));

    parse_init(&psr, "a--", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_DEC);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));

    parse_init(&psr, "++a", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_INC_PRE);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));

    parse_init(&psr, "--a", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_DEC_PRE);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));

    parse_init(&psr, "~--a", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_NOT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_DEC_PRE);

    parse_init(&psr, "--a * 1", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_MUL);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_DEC_PRE);

    parse_init(&psr, "~--a", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_NOT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_DEC_PRE);

    parse_init(&psr, "a-- * 1", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_MUL);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_DEC);

    parse_init(&psr, "a.b--", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_DEC);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_PROP);

    parse_init(&psr, "--a.b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_DEC_PRE);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_PROP);

    parse_init(&psr, "a[0]--", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_DEC);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ELEM);

    parse_init(&psr, "--a[0]", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_DEC_PRE);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ELEM);

    parse_init(&psr, "--a()", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "--(a + 1)", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "--true", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "--false", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "--undefined", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "--0", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "--'hello'", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "a()++", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "(a + 1)++", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "true++", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "false++", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "undefined++", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "0++", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
    parse_init(&psr, "'hello'++", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT(0 == (expr = parse_expr(&psr)));
}

static void test_expr_unary(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "-a ~b !c !!d", NULL, heap_buf, PSR_BUF_SIZE);

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_NEG);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_NOT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("b", TEXT(L_(expr))));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_LOGIC_NOT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("c", TEXT(L_(expr))));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_LOGIC_NOT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_LOGIC_NOT);
    CU_ASSERT(L_(L_(expr)) && ast_expr_type(L_(L_(expr))) == EXPR_ID
                && !strcmp("d", TEXT(L_(L_(expr)))));
}

static void test_expr_mul(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "a * b 1 / 2 c % 2", NULL, heap_buf, PSR_BUF_SIZE);

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_MUL);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID && !strcmp("b", TEXT(R_(expr))));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_DIV);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_NUM && 1 == NUMBER(L_(expr)));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && 2 == NUMBER(R_(expr)));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_MOD);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("c", TEXT(L_(expr))));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && 2 == NUMBER(R_(expr)));
}

static void test_expr_add(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "a + b 1 - 2 c+1-2*f", NULL, heap_buf, PSR_BUF_SIZE);

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_ADD);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID && !strcmp("b", TEXT(R_(expr))));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_SUB);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_NUM && 1 == NUMBER(L_(expr)));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && 2 == NUMBER(R_(expr)));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_SUB);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ADD);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_MUL);
    CU_ASSERT(L_(L_(expr)) && ast_expr_type(L_(L_(expr))) == EXPR_ID);
    CU_ASSERT(L_(L_(expr)) && 0 == strcmp(TEXT(L_(L_(expr))), "c"));
    CU_ASSERT(R_(L_(expr)) && NUMBER(R_(L_(expr))) == 1);
    CU_ASSERT(L_(R_(expr)) && NUMBER(L_(R_(expr))) == 2);
    CU_ASSERT(R_(R_(expr)) && 0 == strcmp(TEXT(R_(R_(expr))), "f"));
}

static void test_expr_shift(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "1<<1 1>>2 1<<1+2 1+2>>1", NULL, heap_buf, PSR_BUF_SIZE);

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_LSHIFT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_NUM && 1 == NUMBER(L_(expr)));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && 1 == NUMBER(R_(expr)));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_RSHIFT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_NUM && 1 == NUMBER(L_(expr)));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && 2 == NUMBER(R_(expr)));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_LSHIFT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_NUM && NUMBER(L_(expr)) == 1);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ADD);
    CU_ASSERT(L_(R_(expr)) && NUMBER(L_(R_(expr))) == 1);
    CU_ASSERT(R_(R_(expr)) && NUMBER(R_(R_(expr))) == 2);

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_RSHIFT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ADD);
    CU_ASSERT(L_(L_(expr)) && NUMBER(L_(L_(expr))) == 1);
    CU_ASSERT(R_(L_(expr)) && NUMBER(R_(L_(expr))) == 2);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && NUMBER(R_(expr)) == 1);
}

static void test_expr_test(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "1>1 2<2 3>=3 4<=4 5!=5 6==6 a in b\n", NULL, heap_buf, PSR_BUF_SIZE);

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_TGT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_NUM && 1 == NUMBER(L_(expr)));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && 1 == NUMBER(R_(expr)));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_TLT);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_NUM && 2 == NUMBER(L_(expr)));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && 2 == NUMBER(R_(expr)));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_TGE);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_NUM && 3 == NUMBER(L_(expr)));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && 3 == NUMBER(R_(expr)));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_TLE);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_NUM && 4 == NUMBER(L_(expr)));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && 4 == NUMBER(R_(expr)));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_TNE);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_NUM && 5 == NUMBER(L_(expr)));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && 5 == NUMBER(R_(expr)));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_TEQ);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_NUM && 6 == NUMBER(L_(expr)));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_NUM && 6 == NUMBER(R_(expr)));

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_TIN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID && !strcmp("a", TEXT(L_(expr))));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID && !strcmp("b", TEXT(R_(expr))));
}

static void test_expr_logic(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "a&&b||c&&d", NULL, heap_buf, PSR_BUF_SIZE);

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_LOGIC_OR);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_LOGIC_AND);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_LOGIC_AND);
    CU_ASSERT(L_(L_(expr)) && ast_expr_type(L_(L_(expr))) == EXPR_ID);
    CU_ASSERT(L_(L_(expr)) && 0 == strcmp(TEXT(L_(L_(expr))), "a"));
    CU_ASSERT(R_(L_(expr)) && ast_expr_type(R_(L_(expr))) == EXPR_ID);
    CU_ASSERT(R_(L_(expr)) && 0 == strcmp(TEXT(R_(L_(expr))), "b"));
    CU_ASSERT(L_(R_(expr)) && ast_expr_type(L_(R_(expr))) == EXPR_ID);
    CU_ASSERT(L_(R_(expr)) && 0 == strcmp(TEXT(L_(R_(expr))), "c"));
    CU_ASSERT(R_(R_(expr)) && ast_expr_type(R_(R_(expr))) == EXPR_ID);
    CU_ASSERT(R_(R_(expr)) && 0 == strcmp(TEXT(R_(R_(expr))), "d"));

}

static void test_expr_ternary(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "a ? b : c", NULL, heap_buf, PSR_BUF_SIZE);

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_TERNARY);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(L_(expr) && 0 == strcmp(TEXT(L_(expr)), "a"));

    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_PAIR);
    CU_ASSERT(L_(R_(expr)) && ast_expr_type(L_(R_(expr))) == EXPR_ID);
    CU_ASSERT(L_(R_(expr)) && 0 == strcmp(TEXT(L_(R_(expr))), "b"));
    CU_ASSERT(R_(R_(expr)) && ast_expr_type(R_(R_(expr))) == EXPR_ID);
    CU_ASSERT(R_(R_(expr)) && 0 == strcmp(TEXT(R_(R_(expr))), "c"));

}

static void test_expr_assign(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "a = b = c", NULL, heap_buf, PSR_BUF_SIZE);

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(L_(expr) && 0 == strcmp(TEXT(L_(expr)), "a"));

    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ASSIGN);
    CU_ASSERT(L_(R_(expr)) && ast_expr_type(L_(R_(expr))) == EXPR_ID);
    CU_ASSERT(L_(R_(expr)) && 0 == strcmp(TEXT(L_(R_(expr))), "b"));
    CU_ASSERT(R_(R_(expr)) && ast_expr_type(R_(R_(expr))) == EXPR_ID);
    CU_ASSERT(R_(R_(expr)) && 0 == strcmp(TEXT(R_(R_(expr))), "c"));

}

static void test_expr_op_assign(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "a += b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_ADD_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

    parse_init(&psr, "a += b + c", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_ADD_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ADD);

    parse_init(&psr, "a += b += c", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_ADD_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ADD_ASSIGN);

    parse_init(&psr, "a -= b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_SUB_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

    parse_init(&psr, "a *= b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_MUL_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

    parse_init(&psr, "a /= b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_DIV_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

    parse_init(&psr, "a %= b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_MOD_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

    parse_init(&psr, "a &= b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_AND_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

    parse_init(&psr, "a |= b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_OR_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

    parse_init(&psr, "a ^= b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_XOR_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

    parse_init(&psr, "a ~= b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_NOT_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

    parse_init(&psr, "a <<= b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_LSHIFT_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

    parse_init(&psr, "a >>= b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_RSHIFT_ASSIGN);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);
}

static void test_expr_comma(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "a, b, c", NULL, heap_buf, PSR_BUF_SIZE);

    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_COMMA);
    CU_ASSERT_FATAL(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(L_(expr) && 0 == strcmp(TEXT(L_(expr)), "a"));

    CU_ASSERT_FATAL(R_(expr) && ast_expr_type(R_(expr)) == EXPR_COMMA);
    CU_ASSERT(L_(R_(expr)) && ast_expr_type(L_(R_(expr))) == EXPR_ID);
    CU_ASSERT(L_(R_(expr)) && 0 == strcmp(TEXT(L_(R_(expr))), "b"));
    CU_ASSERT(R_(R_(expr)) && ast_expr_type(R_(R_(expr))) == EXPR_ID);
    CU_ASSERT(R_(R_(expr)) && 0 == strcmp(TEXT(R_(R_(expr))), "c"));

}

static void test_expr_funcdef(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "def (x) return x + b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_FUNCDEF);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_FUNCHEAD);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_FUNCPROC);
    CU_ASSERT(!L_(L_(expr)));
    CU_ASSERT(R_(L_(expr)) && ast_expr_type(R_(L_(expr))) == EXPR_ID);

    parse_init(&psr, "def f() {x = x + 1 return x * 2}\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_FUNCDEF);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_FUNCHEAD);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_FUNCPROC);
    CU_ASSERT(L_(L_(expr)) && ast_expr_type(L_(L_(expr))) == EXPR_ID);
    CU_ASSERT(!R_(L_(expr)));

    parse_init(&psr, "def () 100\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_FUNCDEF);
    CU_ASSERT(!L_(expr));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_FUNCPROC);

    parse_init(&psr, "def f(x) {x = x + 1 return x * 2}\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_FUNCDEF);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_FUNCHEAD);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_FUNCPROC);
    CU_ASSERT(L_(L_(expr)) && ast_expr_type(L_(L_(expr))) == EXPR_ID);
    CU_ASSERT(R_(L_(expr)) && ast_expr_type(R_(L_(expr))) == EXPR_ID);
    CU_ASSERT(STMT(R_(expr)) && STMT(R_(expr))->next);

    parse_init(&psr, "def () {}\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_FUNCDEF);
    CU_ASSERT(!L_(expr));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_FUNCPROC);

    parse_init(&psr, "def () fn()\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_FUNCDEF);
    CU_ASSERT(!L_(expr));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_FUNCPROC);

    parse_init(&psr, "def () {;}\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_FUNCDEF);
    CU_ASSERT(!L_(expr));
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_FUNCPROC);
}

static void test_expr_funcall(void)
{
    parser_t psr;
    expr_t   *expr;

    parse_init(&psr, "a()\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_CALL);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) == NULL);

    parse_init(&psr, "a ()\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_CALL);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) == NULL);

    parse_init(&psr, "a (b)\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_CALL);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

    parse_init(&psr, "a (b, c)\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_CALL);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_COMMA);

    parse_init(&psr, "a ((def () fn()), 1)\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_CALL);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ID);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_COMMA);

    parse_init(&psr, "(def(a)fn(a);)(x)\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_CALL);
    CU_ASSERT(L_(expr) && ast_expr_type(L_(expr)) == EXPR_FUNCDEF);
    CU_ASSERT(R_(expr) && ast_expr_type(R_(expr)) == EXPR_ID);

}

static void test_expr_array(void)
{
    parser_t psr;
    expr_t   *expr;
    expr_t   *elem[4];

    parse_init(&psr, "[]", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_ARRAY);
    CU_ASSERT(L_(expr) == NULL && R_(expr) == NULL);


    parse_init(&psr, "[a, 'a', [1], {a: 1, b: 2}]", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));

    CU_ASSERT(ast_expr_type(expr) == EXPR_ARRAY);
    elem[3] = R_(expr);
    CU_ASSERT_FATAL(L_(expr) && ast_expr_type(L_(expr)) == EXPR_ARRAY);
    elem[2] = R_(L_(expr));
    CU_ASSERT_FATAL(L_(L_(expr)) && ast_expr_type(L_(L_(expr))) == EXPR_ARRAY);
    elem[1] = R_(L_(L_(expr)));
    elem[0] = L_(L_(L_(expr)));

    CU_ASSERT(ast_expr_type(elem[0]) == EXPR_ID);
    CU_ASSERT(ast_expr_type(elem[1]) == EXPR_STRING);
    CU_ASSERT(ast_expr_type(elem[2]) == EXPR_ARRAY);
    CU_ASSERT(ast_expr_type(elem[3]) == EXPR_DICT);

    CU_ASSERT(ast_expr_type(L_(elem[2])) == EXPR_NUM);
    CU_ASSERT(R_(elem[2]) == NULL);
}

static void test_expr_dict(void)
{
    parser_t psr;
    expr_t   *expr;
    expr_t   *prop[4];


    parse_init(&psr, "{}", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));
    CU_ASSERT(ast_expr_type(expr) == EXPR_DICT);
    CU_ASSERT(L_(expr) == NULL && R_(expr) == NULL);


    parse_init(&psr, "{a: a, 'b': 'a', c: [1, 2], d: {a: 1}}", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (expr = parse_expr(&psr)));

    CU_ASSERT(ast_expr_type(expr) == EXPR_DICT);
    prop[3] = R_(expr);
    CU_ASSERT_FATAL(L_(expr) && ast_expr_type(L_(expr)) == EXPR_DICT);
    prop[2] = R_(L_(expr));
    CU_ASSERT_FATAL(L_(L_(expr)) && ast_expr_type(L_(L_(expr))) == EXPR_DICT);
    prop[1] = R_(L_(L_(expr)));
    prop[0] = L_(L_(L_(expr)));

    CU_ASSERT(ast_expr_type(prop[0]) == EXPR_PAIR);
    CU_ASSERT(ast_expr_type(prop[1]) == EXPR_PAIR);
    CU_ASSERT(ast_expr_type(prop[2]) == EXPR_PAIR);
    CU_ASSERT(ast_expr_type(prop[3]) == EXPR_PAIR);

    CU_ASSERT(ast_expr_type(L_(prop[0])) == EXPR_ID);
    CU_ASSERT(ast_expr_type(R_(prop[0])) == EXPR_ID);

    CU_ASSERT(ast_expr_type(L_(prop[1])) == EXPR_STRING);
    CU_ASSERT(ast_expr_type(R_(prop[1])) == EXPR_STRING);

    CU_ASSERT(ast_expr_type(L_(prop[2])) == EXPR_ID);
    CU_ASSERT(ast_expr_type(R_(prop[2])) == EXPR_ARRAY);

    CU_ASSERT(ast_expr_type(L_(prop[3])) == EXPR_ID);
    CU_ASSERT(ast_expr_type(R_(prop[3])) == EXPR_DICT);
}

static void test_stmt_simple(void)
{
    parser_t psr;
    stmt_t   *stmt;

    // expression
    parse_init(&psr, "a + b;\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_EXPR);
    CU_ASSERT(stmt->expr->type == EXPR_ADD);

    parse_init(&psr, "a - b\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_EXPR);
    CU_ASSERT(stmt->expr->type == EXPR_SUB);

    // break
    parse_init(&psr, "break\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_BREAK);
    CU_ASSERT(!stmt->expr);

    // continue
    parse_init(&psr, "continue", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_CONTINUE);
    CU_ASSERT(!stmt->expr);

    // var
    parse_init(&psr, "var a = 1, b = c = 0", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_VAR);
    CU_ASSERT(stmt->expr->type == EXPR_COMMA);

    // return
    parse_init(&psr, "return a + b", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_RET);
    CU_ASSERT(stmt->expr->type == EXPR_ADD);

    parse_init(&psr, "return;", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_RET);
    CU_ASSERT(!stmt->expr);
}

static void test_stmt_if(void)
{
    parser_t psr;
    stmt_t   *stmt;
    char     *input = "\
    if (a + b)\n\
       a = a - b\n\
    else {\n\
       a = b - a\n\
       b = b - a\n\
    }\n";

    parse_init(&psr, input, NULL, heap_buf, PSR_BUF_SIZE);

    // expression
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_IF);
    CU_ASSERT(stmt->expr->type == EXPR_ADD);
    CU_ASSERT(stmt->block != NULL);
    CU_ASSERT(stmt->other != NULL);
}

static void test_stmt_while(void)
{
    parser_t psr;
    stmt_t   *stmt;
    char     *input = "\
    while (a > b) {\n\
       a = a - 1\n\
       b = b + 1\n\
    }\n";

    parse_init(&psr, input, NULL, heap_buf, PSR_BUF_SIZE);

    // expression
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_WHILE);
    CU_ASSERT(stmt->expr->type == EXPR_TGT);
}

static void test_stmt_try(void)
{
    parser_t psr;
    stmt_t   *stmt;
    char     *input = "\
    try\n\
       a = a / b\n\
    catch {\n\
       a = b - a\n\
       b = b - a\n\
    }\n";
    parse_init(&psr, input, NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT_FATAL(stmt->type == STMT_TRY);
    CU_ASSERT(stmt->block != NULL);
    CU_ASSERT(stmt->other != NULL);
    CU_ASSERT(stmt->other->next != NULL);

    input = "\
    try {\n\
        b = 0;\n\
        a = a / b;\n\
    } catch (err) {\n\
        print(err);\n\
        throw Error('test');\n\
    }\n";
    parse_init(&psr, input, NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_TRY);
    CU_ASSERT(stmt->expr != NULL);
    CU_ASSERT(stmt->block != NULL);
    CU_ASSERT(stmt->block->next != NULL);
    CU_ASSERT(stmt->other != NULL);

    // throw
    parse_init(&psr, "throw;\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_THROW);
    CU_ASSERT(stmt->expr == NULL);

    parse_init(&psr, "throw Error('some error')\n", NULL, heap_buf, PSR_BUF_SIZE);
    CU_ASSERT_FATAL(0 != (stmt = parse_stmt(&psr)));
    CU_ASSERT(stmt->type == STMT_THROW);
    CU_ASSERT(stmt->expr != NULL);
}

CU_pSuite test_lang_parse_entry()
{
    CU_pSuite suite = CU_add_suite("lang parse", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "parse expression factor",   test_expr_factor);
        CU_add_test(suite, "parse expression primary",  test_expr_primary);
        CU_add_test(suite, "parse expression selfop",   test_expr_selfop);
        CU_add_test(suite, "parse expression unary",    test_expr_unary);
        CU_add_test(suite, "parse expression multi",    test_expr_mul);
        CU_add_test(suite, "parse expression add",      test_expr_add);
        CU_add_test(suite, "parse expression shift",    test_expr_shift);
        CU_add_test(suite, "parse expression test",     test_expr_test);
        CU_add_test(suite, "parse expression logic",    test_expr_logic);
        CU_add_test(suite, "parse expression ternary",  test_expr_ternary);
        CU_add_test(suite, "parse expression assign",   test_expr_assign);
        CU_add_test(suite, "parse expression x assign", test_expr_op_assign);
        CU_add_test(suite, "parse expression comma",    test_expr_comma);
        CU_add_test(suite, "parse expression funcdef",  test_expr_funcdef);
        CU_add_test(suite, "parse expression funcall",  test_expr_funcall);

        CU_add_test(suite, "parse expression array",    test_expr_array);
        CU_add_test(suite, "parse expression dict",     test_expr_dict);

        CU_add_test(suite, "parse statements simple",   test_stmt_simple);
        CU_add_test(suite, "parse statements if",       test_stmt_if);
        CU_add_test(suite, "parse statements while",    test_stmt_while);
        CU_add_test(suite, "parse statements try",      test_stmt_try);
    }

    return suite;
}

