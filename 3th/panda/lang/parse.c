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

#include "err.h"
#include "ast.h"
#include "lex.h"
#include "parse.h"

static expr_t *parse_expr_funcdef(parser_t *psr);
static expr_t *parse_expr_form_parenth(parser_t *psr);
static expr_t *parse_expr_form_array(parser_t *psr);
static expr_t *parse_expr_form_dict(parser_t *psr);
static expr_t *parse_expr_form_attr(parser_t *psr, expr_t *e);
static expr_t *parse_expr_form_elem(parser_t *psr, expr_t *e);
static expr_t *parse_expr_form_call(parser_t *psr, expr_t *e);
static expr_t *parse_expr_form_pair(parser_t *psr);
static expr_t *parse_expr_form_unary(parser_t *psr, int type, expr_t *lft);
static expr_t *parse_expr_form_binary(parser_t *psr, int type, expr_t *lft, expr_t *rht);
static stmt_t *parse_stmt_block(parser_t *psr);

static parse_event_t parse_event;

static void parse_fail(parser_t *psr, int err)
{
    psr->error = err;
    if (psr->usr_cb) {
        parse_event.type = PARSE_FAIL;
        parse_event.psr  = psr;
        parse_position(psr, &parse_event.line, &parse_event.col);

        psr->usr_cb(psr->usr_data, &parse_event);
    }
}

static void parse_post(parser_t *psr, int type)
{
    if (psr->usr_cb) {
        parse_event.type = type;
        parse_event.psr  = psr;
        psr->usr_cb(psr->usr_data, &parse_event);
    }
}

static inline char *parse_strdup(parser_t *psr, const char *s) {
    int size  = strlen(s) + 1;
    char *dup = heap_alloc(&psr->heap, size);

    if (dup) {
        memcpy(dup, s, size);
    }
    return dup;
}

static inline expr_t *parse_expr_alloc_type(parser_t *psr, int type) {
    expr_t *e = (expr_t *) heap_alloc(&psr->heap, sizeof(expr_t));

    if (e) {
        e->type = type;
        e->body.child.lft = NULL;
        e->body.child.rht = NULL;
    }

    return e;
}

static inline expr_t *parse_expr_alloc_str(parser_t *psr, int type, const char *text) {
    expr_t *e = (expr_t *) parse_expr_alloc_type(psr, type);

    if (e) {
        char *str = parse_strdup(psr, text);

        if (!str) {
            return NULL;
        }
        e->body.data.str = str;
    }
    return e;
}

static inline expr_t *parse_expr_alloc_num(parser_t *psr, const char *text) {
    expr_t *e = (expr_t *) parse_expr_alloc_type(psr, EXPR_NUM);

    if (e) {
        e->body.data.num = atof(text);
    }
    return e;
}

static inline expr_t *parse_expr_alloc_proc(parser_t *psr, stmt_t * s) {
    expr_t *e = (expr_t *) parse_expr_alloc_type(psr, EXPR_FUNCPROC);

    if (e) {
        e->body.data.proc = s;
    }

    return e;
}

static inline stmt_t *parse_stmt_alloc_0(parser_t *psr, int type) {
    stmt_t *s = (stmt_t *) heap_alloc(&psr->heap, sizeof(stmt_t));

    if (s) {
        s->type = type;
        s->expr = NULL;
        s->block = NULL;
        s->other = NULL;
        s->next = NULL;
    }

    return s;
}

static inline stmt_t *parse_stmt_alloc_1(parser_t *psr, int t, expr_t *e) {
    stmt_t *s = (stmt_t *) heap_alloc(&psr->heap, sizeof(stmt_t));

    if (s) {
        s->type = t;
        s->expr = e;
        s->block = NULL;
        s->other = NULL;
        s->next = NULL;
    }

    return s;
}

static inline stmt_t *parse_stmt_alloc_2(parser_t *psr, int t, expr_t *e, stmt_t *block) {
    stmt_t *s = (stmt_t *) heap_alloc(&psr->heap, sizeof(stmt_t));

    if (s) {
        s->type = t;
        s->expr = e;
        s->block = block;
        s->other = NULL;
        s->next = NULL;
    }

    return s;
}

static inline stmt_t *parse_stmt_alloc_3(parser_t *psr, int t, expr_t *e, stmt_t *block, stmt_t *other) {
    stmt_t *s = (stmt_t *) heap_alloc(&psr->heap, sizeof(stmt_t));

    if (s) {
        s->type = t;
        s->expr = e;
        s->block = block;
        s->other = other;
        s->next = NULL;
    }

    return s;
}

static expr_t *parse_expr_factor(parser_t *psr)
{
    token_t token;
    int tok = parse_token(psr, &token);
    expr_t *expr = NULL;

    switch (tok) {
        case TOK_EOF:   parse_fail(psr, ERR_InvalidToken); break;
        case '(':       expr = parse_expr_form_parenth(psr); break;
        case '[':       expr = parse_expr_form_array(psr); break;
        case '{':       expr = parse_expr_form_dict(psr); break;
        case TOK_DEF:   expr = parse_expr_funcdef(psr) ; break;
        case TOK_ID:    if(!(expr = parse_expr_alloc_str(psr, EXPR_ID, token.text))) parse_fail(psr, ERR_NotEnoughMemory); parse_match(psr, tok); break;
        case TOK_NUM:   if(!(expr = parse_expr_alloc_num(psr, token.text))) parse_fail(psr, ERR_NotEnoughMemory); parse_match(psr, tok); break;
        case TOK_STR:   if(!(expr = parse_expr_alloc_str(psr, EXPR_STRING, token.text))) parse_fail(psr, ERR_NotEnoughMemory); parse_match(psr, tok); break;
        case TOK_UND:   if(!(expr = parse_expr_alloc_type(psr, EXPR_UND))) parse_fail(psr, ERR_NotEnoughMemory); parse_match(psr, tok); break;
        case TOK_NAN:   if(!(expr = parse_expr_alloc_type(psr, EXPR_NAN))) parse_fail(psr, ERR_NotEnoughMemory); parse_match(psr, tok); break;
        case TOK_NULL:  if(!(expr = parse_expr_alloc_type(psr, EXPR_NULL))) parse_fail(psr, ERR_NotEnoughMemory); parse_match(psr, tok); break;
        case TOK_TRUE:  if(!(expr = parse_expr_alloc_type(psr, EXPR_TRUE))) parse_fail(psr, ERR_NotEnoughMemory); parse_match(psr, tok); break;
        case TOK_FALSE: if(!(expr = parse_expr_alloc_type(psr, EXPR_FALSE))) parse_fail(psr, ERR_NotEnoughMemory); parse_match(psr, tok); break;
        default:        parse_fail(psr, ERR_InvalidToken); break;
    }

    return expr;
}

static expr_t *parse_expr_primary(parser_t *psr)
{
    expr_t *expr = parse_expr_factor(psr);
    int tok = parse_token(psr, NULL);

    //if (expr && expr->type != EXPR_ID) {
    //    return expr;
    //}

    while (expr && (tok == '.' || tok == '[' || tok == '(')) {
        if (tok == '.') {
            expr = parse_expr_form_attr(psr, expr);
        } else
        if (tok == '[') {
            expr = parse_expr_form_elem(psr, expr);
        } else {
            /*
            if (expr->type != EXPR_ID && expr->type != EXPR_PROP && expr->type != EXPR_ELEM && expr->type != EXPR_CALL) {
                break;
            }
            */
            expr = parse_expr_form_call(psr, expr);
        }
        tok = parse_token(psr, NULL);
    }

    return expr;
}

static expr_t *parse_expr_selfop(parser_t *psr)
{
    int tok = parse_token(psr, NULL);
    expr_t *expr;

    if (tok == TOK_INC) {
        parse_match(psr, tok);
        expr = parse_expr_form_unary(psr, EXPR_INC_PRE, parse_expr_primary(psr));
    } else
    if (tok == TOK_DEC) {
        parse_match(psr, tok);
        expr = parse_expr_form_unary(psr, EXPR_DEC_PRE, parse_expr_primary(psr));
    } else {
        expr = parse_expr_primary(psr);
        if (parse_match(psr, TOK_INC)) {
            expr = parse_expr_form_unary(psr, EXPR_INC, expr);
        } else
        if (parse_match(psr, TOK_DEC)){
            expr = parse_expr_form_unary(psr, EXPR_DEC, expr);
        } else {
            return expr;
        }
    }

    if (expr) {
        int type = ast_expr_lft(expr)->type;

        if (type != EXPR_ID && type != EXPR_PROP && type != EXPR_ELEM) {
            parse_fail(psr, ERR_InvalidLeftValue);
            return NULL;
        }
    }

    return expr;
}

static expr_t *parse_expr_unary(parser_t *psr)
{
    expr_t *expr;
    int tok = parse_token(psr, NULL);

    if (tok == '!') {
        parse_match(psr, tok);
        expr = parse_expr_form_unary(psr, EXPR_LOGIC_NOT, parse_expr_unary(psr));
    } else
    if (tok == '-' || tok == '~') {
        parse_match(psr, tok);
        expr = parse_expr_form_unary(psr, tok == '-' ? EXPR_NEG : EXPR_NOT,
                                     parse_expr_unary(psr));
    } else {
        expr = parse_expr_selfop(psr);
    }

    return expr;
}

static expr_t *parse_expr_mul(parser_t *psr)
{
    expr_t *expr = parse_expr_unary(psr);
    int tok = parse_token(psr, NULL);

    while (expr && (tok == '*' || tok == '/' || tok == '%')) {
        int type = tok == '*' ? EXPR_MUL : tok == '/' ? EXPR_DIV : EXPR_MOD;

        parse_match(psr, tok);
        expr = parse_expr_form_binary(psr, type, expr, parse_expr_unary(psr));

        tok = parse_token(psr, NULL);
    }

    return expr;
}

static expr_t *parse_expr_add(parser_t *psr)
{
    expr_t *expr = parse_expr_mul(psr);
    int tok = parse_token(psr, NULL);

    while (expr && (tok == '+' || tok == '-')) {
        parse_match(psr, tok);
        expr = parse_expr_form_binary(psr, tok == '+' ? EXPR_ADD : EXPR_SUB,
                                      expr, parse_expr_mul(psr));
        tok = parse_token(psr, NULL);
    }

    return expr;
}

static expr_t *parse_expr_shift(parser_t *psr)
{
    expr_t *expr = parse_expr_add(psr);
    int tok = parse_token(psr, NULL);

    while (expr && (tok == TOK_RSHIFT || tok == TOK_LSHIFT)) {
        parse_match(psr, tok);
        expr = parse_expr_form_binary(psr, tok == TOK_RSHIFT ? EXPR_RSHIFT : EXPR_LSHIFT,
                                      expr, parse_expr_add(psr));
        tok = parse_token(psr, NULL);
    }

    return expr;
}

static expr_t *parse_expr_aand (parser_t *psr)
{
    expr_t *expr = parse_expr_shift(psr);
    int tok = parse_token(psr, NULL);

    while (expr && (tok == '&' || tok == '|' || tok == '^')) {
        parse_match(psr, tok);
        expr = parse_expr_form_binary(psr, tok == '&' ? EXPR_AND : tok == '|' ? EXPR_OR : EXPR_XOR,
                                      expr, parse_expr_shift(psr));
        tok = parse_token(psr, NULL);
    }

    return expr;
}

static expr_t *parse_expr_test (parser_t *psr)
{
    expr_t *expr = parse_expr_aand(psr);
    int tok = parse_token(psr, NULL);

    while (expr && (tok == '>' || tok == '<' || tok == TOK_NE ||
                    tok == TOK_EQ || tok == TOK_GE || tok == TOK_LE || tok == TOK_IN)) {
        int type;

        parse_match(psr, tok);
        switch(tok) {
            case '>': type = EXPR_TGT; break;
            case '<': type = EXPR_TLT; break;
            case TOK_NE: type = EXPR_TNE; break;
            case TOK_EQ: type = EXPR_TEQ; break;
            case TOK_GE: type = EXPR_TGE; break;
            case TOK_LE: type = EXPR_TLE; break;
            default: type = EXPR_TIN;
        }

        expr = parse_expr_form_binary(psr, type, expr, parse_expr_aand(psr));
        tok = parse_token(psr, NULL);
    }

    return expr;
}

static expr_t *parse_expr_logic_and(parser_t *psr)
{
    expr_t *expr = parse_expr_test(psr);
    int tok = parse_token(psr, NULL);

    if (expr && (tok == TOK_LOGICAND)) {
        parse_match(psr, tok);
        expr = parse_expr_form_binary(psr, EXPR_LOGIC_AND, expr, parse_expr_logic_and(psr));
    }

    return expr;
}

static expr_t *parse_expr_logic_or(parser_t *psr)
{
    expr_t *expr = parse_expr_logic_and(psr);
    int tok = parse_token(psr, NULL);

    if (expr && (tok == TOK_LOGICOR)) {
        parse_match(psr, tok);
        expr = parse_expr_form_binary(psr, EXPR_LOGIC_OR, expr, parse_expr_logic_or(psr));
    }

    return expr;
}

static expr_t *parse_expr_ternary(parser_t *psr)
{
    expr_t *expr = parse_expr_logic_or(psr);
    int tok = parse_token(psr, NULL);

    if (expr && tok == '?') {
        parse_match(psr, tok);
        expr = parse_expr_form_binary(psr, EXPR_TERNARY, expr, parse_expr_form_pair(psr));
    }

    return expr;
}

static expr_t *parse_expr_assign(parser_t *psr)
{
    expr_t *expr = parse_expr_ternary(psr);

    if (expr) {
        int tok = parse_token(psr, NULL);
        int type;

        if (tok == '=') {
            type = EXPR_ASSIGN;
        } else
        if (tok >= TOK_ADDASSIGN && tok <= TOK_RSHIFTASSIGN) {
            type = EXPR_ADD_ASSIGN + (tok - TOK_ADDASSIGN);
        } else {
            return expr;
        }

        parse_match(psr, tok);
        if (expr->type != EXPR_ID && expr->type != EXPR_PROP && expr->type != EXPR_ELEM) {
            parse_fail(psr, ERR_InvalidLeftValue);
            return NULL;
        }

        expr = parse_expr_form_binary(psr, type, expr, parse_expr_assign(psr));
    }

    return expr;
}

static expr_t *parse_expr_pair(parser_t *psr)
{
    int tok = parse_token(psr, NULL);
    expr_t *expr;

    if (tok != TOK_ID && tok != TOK_STR) {
        parse_fail(psr, ERR_InvalidToken);
        return NULL;
    }

    expr = parse_expr_factor(psr);
    if (expr) {
        if (!parse_match(psr, ':')) {
            parse_fail(psr, ERR_InvalidToken);
            return NULL;
        }
        expr = parse_expr_form_binary(psr, EXPR_PAIR, expr, parse_expr_assign(psr));
    }

    return expr;
}

static expr_t *parse_expr_vardef(parser_t *psr)
{
    int tok = parse_token(psr, NULL);
    expr_t *expr;

    if (tok != TOK_ID) {
        parse_fail(psr, ERR_InvalidToken);
        return NULL;
    }

    expr = parse_expr_factor(psr);
    if (expr && parse_match(psr, '=')) {
        expr = parse_expr_form_binary(psr, EXPR_ASSIGN, expr, parse_expr_assign(psr));
    }

    return expr;
}

static expr_t *parse_expr_vardef_list(parser_t *psr)
{
    expr_t *expr = parse_expr_vardef(psr);

    if (expr && parse_match(psr, ',')) {
        expr = parse_expr_form_binary(psr, EXPR_COMMA, expr, parse_expr_vardef_list(psr));
    }

    return expr;
}

static expr_t *parse_expr_comma(parser_t *psr)
{
    expr_t *expr = parse_expr_assign(psr);

    if (expr && ',' == parse_token(psr, NULL)) {
        parse_match(psr, ',');
        expr = parse_expr_form_binary(psr, EXPR_COMMA, expr, parse_expr_comma(psr));
    }

    return expr;
}

static expr_t *parse_expr_funcdef(parser_t *psr)
{
    expr_t *name = NULL, *args = NULL, *head = NULL, *proc = NULL;
    stmt_t *block = NULL;

    parse_match(psr, TOK_DEF);

    if (parse_token(psr, NULL) == TOK_ID) {
        if (!(name = parse_expr_factor(psr))) {
            return NULL;
        }
    }

    if (!parse_match(psr, '(')) {
        parse_fail(psr, ERR_InvalidToken);
        goto DO_ERROR;
    }

    if (!parse_match(psr, ')')) {
        if (!(args = parse_expr_vardef_list(psr))) {
            goto DO_ERROR;
        }
        if (!parse_match(psr, ')')) {
            parse_fail(psr, ERR_InvalidToken);
            goto DO_ERROR;
        }
    }

    if (!(block = parse_stmt_block(psr))) {
        goto DO_ERROR;
    }

    if (name || args) {
        if (!(head = parse_expr_alloc_type(psr, EXPR_FUNCHEAD))) {
            parse_fail(psr, ERR_NotEnoughMemory);
            goto DO_ERROR;
        }
        ast_expr_set_lft(head, name);
        ast_expr_set_rht(head, args);
    }

    if (!(proc = parse_expr_alloc_proc(psr, block))) {
        parse_fail(psr, ERR_NotEnoughMemory);
        goto DO_ERROR;
    }

    return parse_expr_form_binary(psr, EXPR_FUNCDEF, head, proc);

DO_ERROR:
    return NULL;
}

static expr_t *parse_expr_form_attr(parser_t *psr, expr_t *lft)
{
    parse_match(psr, '.');

    if (TOK_ID != parse_token(psr, NULL)) {
        parse_fail(psr, ERR_InvalidToken);
        return NULL;
    }

    return parse_expr_form_binary(psr, EXPR_PROP, lft, parse_expr_factor(psr));
}

static expr_t *parse_expr_form_elem(parser_t *psr, expr_t *lft)
{
    expr_t *expr;

    parse_match(psr, '[');

    expr = parse_expr_form_binary(psr, EXPR_ELEM, lft, parse_expr_ternary(psr));
    if (expr) {
        if (!parse_match(psr, ']')) {
            parse_fail(psr, ERR_InvalidToken);
            return NULL;
        }
    }

    return expr;
}

static expr_t *parse_expr_form_call(parser_t *psr, expr_t *lft)
{
    expr_t *expr;

    parse_match(psr, '(');

    if (parse_match(psr, ')')) {
        expr = parse_expr_form_unary(psr, EXPR_CALL, lft);
    } else {
        expr = parse_expr_form_binary(psr, EXPR_CALL, lft, parse_expr_comma(psr));
        if (expr && ! parse_match(psr, ')')) {
            parse_fail(psr, ERR_InvalidToken);
            return NULL;
        }
    }

    return expr;
}

static expr_t *parse_expr_form_parenth(parser_t *psr)
{
    expr_t *expr;

    // should not empty
    parse_match(psr, '(');
    expr = parse_expr_comma(psr);
    if (expr) {
        if (!parse_match(psr, ')')) {
            parse_fail(psr, ERR_InvalidToken);
            return NULL;
        }
    }
    return expr;
}

static expr_t *parse_expr_form_array(parser_t *psr)
{
    expr_t *expr;

    parse_match(psr, '[');

    // empty array
    if (parse_match(psr, ']')) {
        if (NULL == (expr = parse_expr_alloc_type(psr, EXPR_ARRAY))) {
            parse_fail(psr, ERR_NotEnoughMemory);
        }
        return expr;
    }

    expr = parse_expr_assign(psr);
    if (!expr) {
        return NULL;
    }

    if (!parse_match(psr, ',')) {
        expr = parse_expr_form_unary(psr, EXPR_ARRAY, expr);
    } else {
        expr = parse_expr_form_binary(psr, EXPR_ARRAY, expr, parse_expr_assign(psr));
        while (expr && parse_match(psr, ',')) {
            expr = parse_expr_form_binary(psr, EXPR_ARRAY, expr, parse_expr_assign(psr));
        }
    }

    if (expr && !parse_match(psr, ']')) {
        parse_fail(psr, ERR_InvalidToken);
        return NULL;
    }

    return expr;
}

static expr_t *parse_expr_form_dict(parser_t *psr)
{
    expr_t *expr;

    parse_match(psr, '{');

    // empty dict
    if (parse_match(psr, '}')) {
        if (NULL == (expr = parse_expr_alloc_type(psr, EXPR_DICT))) {
            parse_fail(psr, ERR_NotEnoughMemory);
        }
        return expr;
    }

    if (NULL == (expr = parse_expr_pair(psr))) {
        return expr;
    }

    if (!parse_match(psr, ',')) {
        expr = parse_expr_form_unary(psr, EXPR_DICT, expr);
    } else {
        expr = parse_expr_form_binary(psr, EXPR_DICT, expr, parse_expr_pair(psr));
        while (expr && parse_match(psr, ',')) {
            expr = parse_expr_form_binary(psr, EXPR_DICT, expr, parse_expr_pair(psr));
        }
    }

    if (expr && !parse_match(psr, '}')) {
        parse_fail(psr, ERR_InvalidToken);
        return NULL;
    }

    return expr;
}

static expr_t *parse_expr_form_pair(parser_t *psr)
{
    expr_t *expr = parse_expr_ternary(psr);

    if (expr) {
        if (!parse_match(psr, ':')) {
            parse_fail(psr, ERR_InvalidToken);
            return NULL;
        }
        expr = parse_expr_form_binary(psr, EXPR_PAIR, expr, parse_expr_ternary(psr));
    }

    return expr;
}

static expr_t *parse_expr_form_unary(parser_t *psr, int type, expr_t *lft)
{
    expr_t *expr;

    if (!lft) {
        return NULL;
    }

    if (NULL == (expr = parse_expr_alloc_type(psr, type))) {
        parse_fail(psr, ERR_NotEnoughMemory);
    } else {
        ast_expr_set_lft(expr, lft);
    }

    return expr;
}

static expr_t *parse_expr_form_binary(parser_t *psr, int type, expr_t *lft, expr_t *rht)
{
    expr_t *expr;

    if (!rht) {
        return NULL;
    }

    if (NULL == (expr = parse_expr_alloc_type(psr, type))) {
        parse_fail(psr, ERR_NotEnoughMemory);
    } else {
        ast_expr_set_lft(expr, lft);
        ast_expr_set_rht(expr, rht);
    }

    return expr;
}

static stmt_t *parse_stmt_block(parser_t *psr)
{
    stmt_t *s = NULL;

    parse_post(psr, PARSE_ENTER_BLOCK);
    if (parse_match(psr, '{')) {
        stmt_t *last, *curr;

        while (!parse_match(psr, '}')) {
            // eat empty statements ...
            if (parse_match(psr, ';')) {
                continue;
            }

            if (!(curr = parse_stmt(psr))) {
                return NULL;
            }

            if (s) {
                last = last->next = curr;
            } else {
                s = last = curr;
            }
        }

        if (!psr->error && !s) {
            if (!(s= parse_stmt_alloc_0(psr, STMT_PASS))) {
                parse_fail(psr, ERR_NotEnoughMemory);
            }
        }
    } else {
        if (!(s = parse_stmt(psr))) {
            if (psr->error == 0) {
                parse_fail(psr, ERR_InvalidToken);
            }
            return NULL;
        }
    }
    parse_post(psr, PARSE_LEAVE_BLOCK);

    return s;
}

static stmt_t *parse_stmt_if(parser_t *psr)
{
    expr_t *cond = NULL;
    stmt_t *block = NULL;
    stmt_t *other = NULL;
    stmt_t *s;

    parse_match(psr, TOK_IF);

    if (!(cond = parse_expr(psr))) {
        return NULL;
    }

    if (!(block = parse_stmt_block(psr))) {
        return NULL;
    }

    if (parse_match(psr, TOK_ELSE)) {
        if (!(other = parse_stmt_block(psr))) {
            return NULL;
        }
    }

    if (other) {
        s = parse_stmt_alloc_3(psr, STMT_IF, cond, block, other);
    } else {
        s = parse_stmt_alloc_2(psr, STMT_IF, cond, block);
    }

    if (!s) {
        parse_fail(psr, ERR_NotEnoughMemory);
    }

    return s;
}

static stmt_t *parse_stmt_try(parser_t *psr)
{
    stmt_t *block = NULL;
    stmt_t *other = NULL;
    expr_t *error = NULL;
    stmt_t *s;

    parse_match(psr, TOK_TRY);

    if (!(block = parse_stmt_block(psr))) {
        return NULL;
    }

    if (!parse_match(psr, TOK_CATCH)) {
        parse_fail(psr, ERR_InvalidToken);
        return NULL;
    }

    if (parse_match(psr, '(')) {
        token_t token;
        if (parse_token(psr, &token) != TOK_ID) {
            parse_fail(psr, ERR_InvalidToken);
            return NULL;
        }
        if(!(error = parse_expr_alloc_str(psr, EXPR_ID, token.text))) {
            parse_fail(psr, ERR_NotEnoughMemory);
            return NULL;
        }
        parse_match(psr, TOK_ID);

        if (!parse_match(psr, ')')) {
            parse_fail(psr, ERR_InvalidToken);
            return NULL;
        }
    }

    if (!(other = parse_stmt_block(psr))) {
        return NULL;
    }

    s = parse_stmt_alloc_3(psr, STMT_TRY, error, block, other);
    if (!s) {
        parse_fail(psr, ERR_NotEnoughMemory);
    }

    return s;
}

static stmt_t *parse_stmt_var(parser_t *psr)
{
    expr_t *expr;

    parse_match(psr, TOK_VAR);
    expr = parse_expr_vardef_list(psr);
    if (expr) {
        stmt_t *s;

        if (';' == parse_token(psr, NULL)) {
            parse_match(psr, ';');
        }

        if (NULL != (s = parse_stmt_alloc_1(psr, STMT_VAR, expr))) {
            return s;
        } else {
            parse_fail(psr, ERR_NotEnoughMemory);
        }
    }

    return NULL;
}

static stmt_t *parse_stmt_ret(parser_t *psr)
{
    expr_t *expr = NULL;
    stmt_t *s;

    parse_match(psr, TOK_RET);

    if (!parse_match(psr, ';')) {
        if (NULL == (expr = parse_expr(psr))) {
            return NULL;
        }
        if (';' == parse_token(psr, NULL)) {
            parse_match(psr, ';');
        }
    }

    if (!(s = parse_stmt_alloc_1(psr, STMT_RET, expr))) {
        parse_fail(psr, ERR_NotEnoughMemory);
    }
    return s;
}

static stmt_t *parse_stmt_while(parser_t *psr)
{
    expr_t *cond = NULL;
    stmt_t *block = NULL;
    stmt_t *s;

    parse_match(psr, TOK_WHILE);

    if (!(cond = parse_expr(psr))) {
        return NULL;
    }

    if (!(block = parse_stmt_block(psr))) {
        return NULL;
    }

    s = parse_stmt_alloc_2(psr, STMT_WHILE, cond, block);
    if (!s) {
        parse_fail(psr, ERR_NotEnoughMemory);
    }

    return s;
}

static stmt_t *parse_stmt_throw(parser_t *psr)
{
    expr_t *expr = NULL;
    stmt_t *s;

    parse_match(psr, TOK_THROW);

    if (!parse_match(psr, ';')) {
        if (NULL == (expr = parse_expr(psr))) {
            return NULL;
        }
        parse_match(psr, ';');
    }

    if (!(s = parse_stmt_alloc_1(psr, STMT_THROW, expr))) {
        parse_fail(psr, ERR_NotEnoughMemory);
    }
    return s;
}

static stmt_t *parse_stmt_break(parser_t *psr)
{
    stmt_t *s;

    parse_match(psr, TOK_BREAK);
    if (parse_token(psr, NULL) == ';') {
        parse_match(psr, ';');
    }

    if (!(s = parse_stmt_alloc_0(psr, STMT_BREAK))) {
        parse_fail(psr, ERR_NotEnoughMemory);
    }
    return s;
}

static inline stmt_t *parse_stmt_continue(parser_t *psr)
{
    stmt_t *s;

    parse_match(psr, TOK_CONTINUE);
    if (parse_token(psr, NULL) == ';') {
        parse_match(psr, ';');
    }

    if (!(s = parse_stmt_alloc_0(psr, STMT_CONTINUE))) {
        parse_fail(psr, ERR_NotEnoughMemory);
    }
    return s;
}

static stmt_t *parse_stmt_expr(parser_t *psr)
{
    expr_t *expr = parse_expr(psr);

    while (parse_match(psr, ';'))
        ;

    if (expr) {
        stmt_t *s = parse_stmt_alloc_1(psr, STMT_EXPR, expr);
        if (s) {
            return s;
        }
        parse_fail(psr, ERR_NotEnoughMemory);
    }

    return NULL;
}

expr_t *parse_expr(parser_t *psr)
{
    return parse_expr_comma(psr);
}

stmt_t *parse_stmt(parser_t *psr)
{
    int tok = parse_token(psr, NULL);

    switch (tok) {
        case TOK_EOF:       parse_post(psr, PARSE_EOF); return NULL;
        case TOK_IF:        parse_post(psr, PARSE_COMPOSE); return parse_stmt_if(psr);
        case TOK_TRY:       parse_post(psr, PARSE_SIMPLE); return parse_stmt_try(psr);
        case TOK_VAR:       parse_post(psr, PARSE_SIMPLE); return parse_stmt_var(psr);
        case TOK_RET:       parse_post(psr, PARSE_SIMPLE); return parse_stmt_ret(psr);
        case TOK_WHILE:     parse_post(psr, PARSE_COMPOSE); return parse_stmt_while(psr);
        case TOK_BREAK:     parse_post(psr, PARSE_SIMPLE); return parse_stmt_break(psr);
        case TOK_THROW:     parse_post(psr, PARSE_SIMPLE); return parse_stmt_throw(psr);
        case TOK_CONTINUE:  parse_post(psr, PARSE_SIMPLE); return parse_stmt_continue(psr);
        default:            parse_post(psr, PARSE_SIMPLE); return parse_stmt_expr(psr);
    }
}

stmt_t *parse_stmt_multi(parser_t *psr)
{
    stmt_t *head = NULL, *last, *curr;

    while (!psr->error && !parse_match(psr, 0)) {
        while (parse_match(psr, ';'));

        if (!(curr = parse_stmt(psr))) {
            return NULL;
        }

        if (head) {
            last = last->next = curr;
        } else {
            last = head = curr;
        }
    }

    return head;
}

