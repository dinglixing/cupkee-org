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

static int test_setup()
{
    return 0;
}

static int test_clean()
{
    return 0;
}

static void test_common(void)
{
    lexer_t lex;
    token_t tok;
    char *input =  "\
    \t    \r \n\
    # comments 1\r\n\
    +-*/%\n\
    // comments 2\r\n\
    += -= *= /= %= &= |= ^= ~= >>= <<= >> << && ||\n\
    ' bbbb\" ' \" abcd' &$|!\" \n\
    12345 09876\n\
    /* comments 3\r\n comments 3 continue*/\
    abc a12 _11 a_b _a_ $1 $_a \n\
    undefined null NaN true false var def return while break continue in if elif else try catch throw\n";

    CU_ASSERT(0 == lex_init(&lex, input, NULL));

    CU_ASSERT(lex_match(&lex, '+'));
    CU_ASSERT(lex_match(&lex, '-'));
    CU_ASSERT(lex_match(&lex, '*'));
    CU_ASSERT(lex_match(&lex, '/'));
    CU_ASSERT(lex_match(&lex, '%'));

    CU_ASSERT(lex_match(&lex, TOK_ADDASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_SUBASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_MULASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_DIVASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_MODASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_ANDASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_ORASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_XORASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_NOTASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_RSHIFTASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_LSHIFTASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_RSHIFT));
    CU_ASSERT(lex_match(&lex, TOK_LSHIFT));
    CU_ASSERT(lex_match(&lex, TOK_LOGICAND));
    CU_ASSERT(lex_match(&lex, TOK_LOGICOR));

    CU_ASSERT(TOK_STR == lex_token(&lex, &tok) && 0 == strcmp(tok.text, " bbbb\" "));
    CU_ASSERT(lex_match(&lex, TOK_STR));
    CU_ASSERT(TOK_STR == lex_token(&lex, &tok) && 0 == strcmp(tok.text, " abcd' &$|!"));
    CU_ASSERT(lex_match(&lex, TOK_STR));

    CU_ASSERT(TOK_NUM == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "12345"));
    CU_ASSERT(lex_match(&lex, TOK_NUM));
    CU_ASSERT(TOK_NUM == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "09876"));
    CU_ASSERT(lex_match(&lex, TOK_NUM));

    CU_ASSERT(TOK_ID == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "abc"));
    CU_ASSERT(lex_match(&lex, TOK_ID));
    CU_ASSERT(TOK_ID == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "a12"));
    CU_ASSERT(lex_match(&lex, TOK_ID));
    CU_ASSERT(TOK_ID == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "_11"));
    CU_ASSERT(lex_match(&lex, TOK_ID));
    CU_ASSERT(TOK_ID == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "a_b"));
    CU_ASSERT(lex_match(&lex, TOK_ID));
    CU_ASSERT(TOK_ID == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "_a_"));
    CU_ASSERT(lex_match(&lex, TOK_ID));
    CU_ASSERT(TOK_ID == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "$1"));
    CU_ASSERT(lex_match(&lex, TOK_ID));
    CU_ASSERT(TOK_ID == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "$_a"));
    CU_ASSERT(lex_match(&lex, TOK_ID));

    CU_ASSERT(lex_match(&lex, TOK_UND));
    CU_ASSERT(lex_match(&lex, TOK_NULL));
    CU_ASSERT(lex_match(&lex, TOK_NAN));
    CU_ASSERT(lex_match(&lex, TOK_TRUE));
    CU_ASSERT(lex_match(&lex, TOK_FALSE));
    CU_ASSERT(lex_match(&lex, TOK_VAR));
    CU_ASSERT(lex_match(&lex, TOK_DEF));
    CU_ASSERT(lex_match(&lex, TOK_RET));
    CU_ASSERT(lex_match(&lex, TOK_WHILE));
    CU_ASSERT(lex_match(&lex, TOK_BREAK));
    CU_ASSERT(lex_match(&lex, TOK_CONTINUE));
    CU_ASSERT(lex_match(&lex, TOK_IN));
    CU_ASSERT(lex_match(&lex, TOK_IF));
    CU_ASSERT(lex_match(&lex, TOK_ELIF));
    CU_ASSERT(lex_match(&lex, TOK_ELSE));
    CU_ASSERT(lex_match(&lex, TOK_TRY));
    CU_ASSERT(lex_match(&lex, TOK_CATCH));
    CU_ASSERT(lex_match(&lex, TOK_THROW));

    CU_ASSERT(0 == lex_deinit(&lex));
}

static void test_mch_tok(void)
{
    lexer_t lex;

    CU_ASSERT(0 == lex_init(&lex, "++ -- << >> += -= *= /= %= &= |= ^= != == && ||", NULL));

    CU_ASSERT(lex_match(&lex, TOK_INC));
    CU_ASSERT(lex_match(&lex, TOK_DEC));
    CU_ASSERT(lex_match(&lex, TOK_LSHIFT));
    CU_ASSERT(lex_match(&lex, TOK_RSHIFT));
    CU_ASSERT(lex_match(&lex, TOK_ADDASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_SUBASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_MULASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_DIVASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_MODASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_ANDASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_ORASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_XORASSIGN));
    CU_ASSERT(lex_match(&lex, TOK_NE));
    CU_ASSERT(lex_match(&lex, TOK_EQ));
    CU_ASSERT(lex_match(&lex, TOK_LOGICAND));
    CU_ASSERT(lex_match(&lex, TOK_LOGICOR));
}

static void test_floating_number(void)
{
    lexer_t lex;
    token_t tok;

    CU_ASSERT(0 == lex_init(&lex, "1.5 1e-3 123E+12 0.1e2", NULL));
    CU_ASSERT(TOK_NUM == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "1.5"));
    CU_ASSERT(lex_match(&lex, TOK_NUM));

    CU_ASSERT(TOK_NUM == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "1E-3"));
    CU_ASSERT(lex_match(&lex, TOK_NUM));

    CU_ASSERT(TOK_NUM == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "123E12"));
    CU_ASSERT(lex_match(&lex, TOK_NUM));

    CU_ASSERT(TOK_NUM == lex_token(&lex, &tok) && 0 == strcmp(tok.text, "0.1E2"));
    CU_ASSERT(lex_match(&lex, TOK_NUM));
}

CU_pSuite test_lang_lex_entry()
{
    CU_pSuite suite = CU_add_suite("lang lex", test_setup, test_clean);

    if (suite) {
        CU_add_test(suite, "common",            test_common);
        CU_add_test(suite, "multiCh token",     test_mch_tok);
        CU_add_test(suite, "floating number",   test_floating_number);
    }

    return suite;
}

