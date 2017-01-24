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

#include "lex.h"

#define CURR_CH     (lex->curr_ch)
#define NEXT_CH     (lex->next_ch)

/*
static int to_number(const char *s, int len)
{
    int ret = 0;
    int i;

    for (i = 0; i < len; i++) {
        ret = ret * 10 + (s[i] - '0');
    }

    return ret;
}
*/

static void lex_get_next_ch(lexer_t *lex)
{
    int ch;

    if (lex->line_pos >= lex->line_end) {
        lex->line_end = 0;
        lex->line_pos = 0;
        ch = 0;
    } else {
        ch = lex->line_buf[lex->line_pos ++];
    }

    if (lex->curr_ch == '\n') {
        lex->line++;
        lex->col = 0;
    } else {
        lex->col++;
    }

    lex->curr_ch = lex->next_ch;
    lex->next_ch = ch;
}

static int lex_chk_token_type(const char *str, int len)
{
    switch(len) {
    case 2:
        if (0 == strcmp("if", str)) return TOK_IF;
        if (0 == strcmp("in", str)) return TOK_IN;
    case 3:
        if (0 == strcmp("def", str)) return TOK_DEF;
        if (0 == strcmp("var", str)) return TOK_VAR;
        if (0 == strcmp("NaN", str)) return TOK_NAN;
        if (0 == strcmp("try", str)) return TOK_TRY;
    case 4:
        if (0 == strcmp("else", str)) return TOK_ELSE;
        if (0 == strcmp("elif", str)) return TOK_ELIF;
        if (0 == strcmp("true", str)) return TOK_TRUE;
        if (0 == strcmp("null", str)) return TOK_NULL;
    case 5:
        if (0 == strcmp("false", str)) return TOK_FALSE;
        if (0 == strcmp("while", str)) return TOK_WHILE;
        if (0 == strcmp("break", str)) return TOK_BREAK;
        if (0 == strcmp("catch", str)) return TOK_CATCH;
        if (0 == strcmp("throw", str)) return TOK_THROW;
    case 6:
        if (0 == strcmp("return", str)) return TOK_RET;
    case 8:
        if (0 == strcmp("continue", str)) return TOK_CONTINUE;
        if (0 == strcmp("function", str)) return TOK_DEF;
    case 9:
        if (0 == strcmp("undefined", str)) return TOK_UND;
    default:
        return TOK_ID;
    }
}

static void lex_eat_comments(lexer_t *lex)
{
    if ('#' == CURR_CH) {
        do {
            lex_get_next_ch(lex);
        } while (CURR_CH && '\n' != CURR_CH && '\r' != CURR_CH);
    } else
    if ('/' == NEXT_CH) {
        do {
            lex_get_next_ch(lex);
        } while (CURR_CH && '\n' != CURR_CH && '\r' != CURR_CH);
    } else
    if ('*' == NEXT_CH) {
        lex_get_next_ch(lex);
        lex_get_next_ch(lex);
        do {
            lex_get_next_ch(lex);
        } while (CURR_CH && !('*' == CURR_CH && '/' == NEXT_CH));
        lex_get_next_ch(lex);
        lex_get_next_ch(lex);
    }
}

static void lex_get_id_token(lexer_t *lex)
{
    int len  = 0;

    do {
        if (len + 1 < lex->token_buf_size) {
            lex->token_buf[len++] = CURR_CH;
        }
        lex_get_next_ch(lex);
    } while (isalnum(CURR_CH) || '_' == CURR_CH || '$' == CURR_CH);

    lex->token_buf[len] = 0;
    lex->token_len = len;
    lex->curr_tok = lex_chk_token_type(lex->token_buf, len);
}

static void lex_get_str_token(lexer_t *lex)
{
    int term = CURR_CH;
    int len  = 0;

    // Eat the head ' Or "
    lex_get_next_ch(lex);

    while (CURR_CH != term) {
        int ch = CURR_CH;

        if (ch == '\\') {
            lex_get_next_ch(lex);
            ch = CURR_CH;
            if (ch == 't') {
                ch = '\t';
            } else
            if (ch == 'r') {
                ch = '\r';
            } else
            if (ch == 'n') {
                ch = '\n';
            }
        }

        if (!ch) {
            lex->curr_tok = TOK_EOF;
            return;
        }
        if (len + 1 < lex->token_buf_size) {
            lex->token_buf[len++] = ch;
        }
        lex_get_next_ch(lex);
    }

    // Eat the tail '\'' Or '"'
    lex_get_next_ch(lex);

    lex->token_buf[len] = 0;
    lex->token_len = len;
    lex->curr_tok = TOK_STR;
}

static int lex_get_num_digit(lexer_t *lex, int len)
{
    while (isdigit(CURR_CH)) {
        if (len + 1 < lex->token_buf_size) {
            lex->token_buf[len++] = CURR_CH;
        } else {
            break;
        }
        lex_get_next_ch(lex);
    }

    return len;
}

static void lex_get_num_token(lexer_t *lex)
{
    int len = 0;

    len = lex_get_num_digit(lex, len);

    if (CURR_CH == '.' && isdigit(NEXT_CH)) {
        lex->token_buf[len++] = '.';
        lex_get_next_ch(lex);
        len = lex_get_num_digit(lex, len);
    }

    if (CURR_CH == 'E' || CURR_CH == 'e') {
        lex->token_buf[len++] = 'E';
        lex_get_next_ch(lex);
        if (CURR_CH == '-') {
            lex_get_next_ch(lex);
            lex->token_buf[len++] = '-';
        }
        if (CURR_CH == '+') {
            lex_get_next_ch(lex);
        }
        len = lex_get_num_digit(lex, len);
    }

    lex->token_buf[len] = 0;
    lex->token_len = len;
    lex->curr_tok = TOK_NUM;
}

static void lex_get_next_token(lexer_t *lex)
{
    int tok;

TOKEN_LOCATE:
    // eat space
    while (isspace(CURR_CH)) {
        lex_get_next_ch(lex);
    }

    tok = CURR_CH;
    // eat comments
    if (tok == '#' || (tok == '/' && (NEXT_CH == '/' || NEXT_CH == '*'))) {
        lex_eat_comments(lex);
        goto TOKEN_LOCATE;
    }

    if (isalpha(tok) || '_' == tok || '$' == tok) {
        lex_get_id_token(lex);
    } else
    if ('\'' == tok || '"' == tok) {
        lex_get_str_token(lex);
    } else
    if (isdigit(tok)) {
        lex_get_num_token(lex);
    } else {
        if (tok) {
            lex_get_next_ch(lex);

            if (CURR_CH == '=') {
                switch (tok) {
                case '!': lex_get_next_ch(lex); tok = TOK_NE; break;
                case '>': lex_get_next_ch(lex); tok = TOK_GE; break;
                case '<': lex_get_next_ch(lex); tok = TOK_LE; break;
                case '=': lex_get_next_ch(lex); tok = TOK_EQ; break;

                case '-': lex_get_next_ch(lex); tok = TOK_SUBASSIGN; break;
                case '+': lex_get_next_ch(lex); tok = TOK_ADDASSIGN; break;
                case '*': lex_get_next_ch(lex); tok = TOK_MULASSIGN; break;
                case '/': lex_get_next_ch(lex); tok = TOK_DIVASSIGN; break;
                case '%': lex_get_next_ch(lex); tok = TOK_MODASSIGN; break;
                case '&': lex_get_next_ch(lex); tok = TOK_ANDASSIGN; break;
                case '|': lex_get_next_ch(lex); tok = TOK_ORASSIGN; break;
                case '^': lex_get_next_ch(lex); tok = TOK_XORASSIGN; break;
                case '~': lex_get_next_ch(lex); tok = TOK_NOTASSIGN; break;
                default: break;
                }
            } else
            if (tok == '+' && CURR_CH == '+') {
                lex_get_next_ch(lex);
                tok = TOK_INC;
            } else
            if (tok == '-' && CURR_CH == '-') {
                lex_get_next_ch(lex);
                tok = TOK_DEC;
            } else
            if (tok == '&' && CURR_CH == '&') {
                lex_get_next_ch(lex);
                tok = TOK_LOGICAND;
            } else
            if (tok == '|' && CURR_CH == '|') {
                lex_get_next_ch(lex);
                tok = TOK_LOGICOR;
            } else
            if (tok == '>' && CURR_CH == '>') {
                lex_get_next_ch(lex);
                if (CURR_CH == '=') {
                    lex_get_next_ch(lex);
                    tok = TOK_RSHIFTASSIGN;
                } else {
                    tok = TOK_RSHIFT;
                }
            } else
            if (tok == '<' && CURR_CH == '<') {
                lex_get_next_ch(lex);
                if (CURR_CH == '=') {
                    lex_get_next_ch(lex);
                    tok = TOK_LSHIFTASSIGN;
                } else {
                    tok = TOK_LSHIFT;
                }
            }
        } else {
            // Token is Eof
        }

        lex->curr_tok = tok;
    }
}

int lex_init(lexer_t *lex, const char *input, char *(*more)(void))
{
    if (lex && input) {
        lex->line_buf_size = strlen(input);
        lex->line_buf = (char *)input;
        lex->token_buf_size = TOKEN_MAX_SIZE;
        lex->line_more = more;

        lex->line_end = lex->line_buf_size;
        lex->line_pos = 0;
        lex->curr_tok = TOK_EOF;
        lex->token_len = 0;

        lex->line = 0;
        lex->col = 0;
        lex_get_next_ch(lex);
        lex_get_next_ch(lex);
        lex_get_next_token(lex);

        return 0;
    }

    return -1;
}

int lex_deinit(lexer_t *lex)
{
    (void) lex;
    return 0;
}

int lex_token(lexer_t *lex, token_t *tok)
{
    if (tok) {
        tok->type = lex->curr_tok;
        tok->line = lex->line;
        tok->col  = lex->col;
        tok->text = lex->token_buf;

        if (tok->type == TOK_ID || tok->type == TOK_STR) {
            tok->value = lex->token_len;
        //} else
        //if (tok->type == TOK_NUM) {
        //    tok->value = to_number(lex->token_buf, lex->token_len);
        }
    }

    return lex->curr_tok;
}

int lex_match(lexer_t *lex, int tok)
{
    if (lex->curr_tok == TOK_EOF && lex->line_more) {
        lex->line_more();
        return 0;
        //lex_init(lex, lex->line_more(), lex->line_more);
    }

    if (lex->curr_tok == tok) {
        lex_get_next_token(lex);
        return 1;
    } else {
        return 0;
    }
}

int lex_position(lexer_t *lex, int *line, int *col)
{
    if (lex) {
        if (line) *line = lex->line;
        if (col) *col = lex->col;
        return 0;
    } else {
        return -1;
    }
}

