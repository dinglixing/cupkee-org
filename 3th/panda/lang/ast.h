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

#ifndef __LANG_AST_INC__
#define __LANG_AST_INC__

#include "config.h"

#include "lex.h"

enum EXPR_TYPE {
    // factor expression
    EXPR_ID = 1,
    EXPR_NUM,
    EXPR_NAN,
    EXPR_UND,
    EXPR_NULL,
    EXPR_TRUE,
    EXPR_FALSE,
    EXPR_FUNCPROC,
    EXPR_STRING,

    // unary expression
    EXPR_NEG,
    EXPR_NOT,
    EXPR_LOGIC_NOT,

    EXPR_INC,
    EXPR_INC_PRE,
    EXPR_DEC,
    EXPR_DEC_PRE,

    EXPR_ARRAY,
    EXPR_DICT,

    // binary expression
    EXPR_MUL,
    EXPR_DIV,
    EXPR_MOD,
    EXPR_ADD,
    EXPR_SUB,

    EXPR_LSHIFT,
    EXPR_RSHIFT,

    EXPR_AND,
    EXPR_OR,
    EXPR_XOR,

    EXPR_TNE,
    EXPR_TEQ,
    EXPR_TGT,
    EXPR_TGE,
    EXPR_TLT,
    EXPR_TLE,
    EXPR_TIN,

    EXPR_LOGIC_AND,
    EXPR_LOGIC_OR,

    EXPR_ASSIGN,
    EXPR_ADD_ASSIGN,
    EXPR_SUB_ASSIGN,
    EXPR_MUL_ASSIGN,
    EXPR_DIV_ASSIGN,
    EXPR_MOD_ASSIGN,
    EXPR_AND_ASSIGN,
    EXPR_OR_ASSIGN,
    EXPR_XOR_ASSIGN,
    EXPR_NOT_ASSIGN,
    EXPR_LSHIFT_ASSIGN,
    EXPR_RSHIFT_ASSIGN,

    EXPR_COMMA,

    EXPR_PROP,
    EXPR_ELEM,
    EXPR_CALL,

    EXPR_TERNARY,

    // form
    EXPR_FUNCDEF,
    EXPR_FUNCHEAD,
    EXPR_PAIR,

    EXPR_DUMMY
};

enum STMT_TYPE {
    STMT_EXPR,
    STMT_IF,
    STMT_VAR,
    STMT_RET,
    STMT_WHILE,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_THROW,
    STMT_TRY,
    STMT_PASS,
};

struct expr_t;
typedef struct stmt_t {
    int type;

    struct expr_t *expr;
    struct stmt_t *block;
    struct stmt_t *other;

    struct stmt_t *next;
} stmt_t;

typedef struct expr_t {
    int type;
    int line, col;
    union {
        union {
            char   *str;
            double  num;
            stmt_t *proc;
        } data;
        struct {
            struct expr_t *lft;
            struct expr_t *rht;
        } child;
    } body;
} expr_t;


static inline const char * ast_expr_text(expr_t *e) {
    return e->body.data.str;
}

static inline double ast_expr_num(expr_t *e) {
    return e->body.data.num;
}

static inline stmt_t * ast_expr_stmt(expr_t *e) {
    return e->body.data.proc;
}

static inline int ast_expr_type(expr_t *e) {
    return e->type;
}

static inline expr_t *ast_expr_lft(expr_t *e) {
    return e->body.child.lft;
}

static inline expr_t *ast_expr_rht(expr_t *e) {
    return e->body.child.rht;
}

static inline void ast_expr_set_lft(expr_t *e, expr_t *lft) {
    e->body.child.lft = lft;
}

static inline void ast_expr_set_rht(expr_t *e, expr_t *rht) {
    e->body.child.rht = rht;
}

void ast_traveral_expr(expr_t *e, void (*cb)(void *, expr_t *), void *ud);

#endif /* __LANG_AST_INC__ */

