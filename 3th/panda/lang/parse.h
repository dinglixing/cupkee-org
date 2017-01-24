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


#ifndef __LANG_PARSE_INC__
#define __LANG_PARSE_INC__

#include "config.h"

#include "lex.h"
#include "ast.h"
#include "heap.h"

enum {
    PARSE_EOF = 0,
    PARSE_FAIL,
    PARSE_SIMPLE,
    PARSE_COMPOSE,
    PARSE_ENTER_BLOCK,
    PARSE_LEAVE_BLOCK
};

struct parser_t;
typedef struct parse_event_t {
    int type;
    int line, col;
    struct parser_t *psr;
} parse_event_t;

typedef struct parser_t {
    int      error;
    lexer_t  lex;
    heap_t   heap;
    void (*usr_cb) (void *, parse_event_t *);
    void *usr_data;
} parser_t;

typedef void (*parse_callback_t)(void *u, parse_event_t *e);

static inline int parse_init(parser_t *psr, const char *input, char *(*more)(void), void *mem, int size) {
    if (psr && input && mem) {
        psr->error = 0;
        lex_init(&psr->lex, input, more);
        heap_init(&psr->heap, mem, size);
        psr->usr_cb = NULL;
        psr->usr_data = NULL;
        return 0;
    } else {
        return -1;
    }
}

static inline void parse_set_cb(parser_t *psr, parse_callback_t cb, void *data) {
    if (psr) {
        psr->usr_cb = cb;
        psr->usr_data = data;
    }
}

static inline void parse_disable_more(parser_t *psr) {
    psr->lex.line_more = NULL;
}

expr_t *parse_expr(parser_t *psr);
stmt_t *parse_stmt(parser_t *psr);
stmt_t *parse_stmt_multi(parser_t *psr);

static inline int parse_position(parser_t *psr, int *line, int *col) {
    return lex_position(&psr->lex, line, col);
}
static inline int parse_token(parser_t *psr, token_t *token) {
    return lex_token(&psr->lex, token);
}
static inline int parse_match(parser_t *psr, int tok) {
//    printf("match: %d(%c)\n", tok, tok);
    return lex_match(&psr->lex, tok);
}

#endif /* __LANG_PARSE_INC__ */

