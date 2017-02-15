/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016-2017 Lixing Ding <ding.lixing@gmail.com>

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

#include "cupkee.h"
#include "rbuff.h"

#define HISTORY_MAX         16
#define HISTORY_BUF_SIZE    256

typedef struct history_index_t {
    uint16_t pos;
    uint16_t len;
} history_index_t;

static rbuff_t history_idx;
static rbuff_t history_buf;
static int16_t history_cursor;

static history_index_t history_indexs[HISTORY_MAX];
static char            history_memory[HISTORY_BUF_SIZE];

static int history_shift(void)
{
    int pos = rbuff_shift(&history_idx);
    history_index_t *index;

    if (pos < 0) {
        return 0;
    }

    index = &history_indexs[pos];
    rbuff_remove_left(&history_buf, index->len);

    return 1;
}

static int history_push(int len, const char *data)
{
    int i, start, pos = rbuff_push(&history_idx);
    history_index_t *index;

    if (pos < 0) {
        return -1;
    }
    index = &history_indexs[pos];

    start = rbuff_end(&history_buf);

    for (i = 0, pos = rbuff_push(&history_buf);
         i < len && pos >= 0;
         i++, pos = rbuff_push(&history_buf)) {
        history_memory[pos] = data[i];
    }

    index->pos = start;
    index->len = i;

    return i;
}

int cupkee_history_init(void)
{
    rbuff_init(&history_idx, HISTORY_MAX);
    rbuff_init(&history_buf, HISTORY_BUF_SIZE);
    history_cursor = 0;

    return 0;
}

int cupkee_history_push(int len, const char *data)
{
    if (len > HISTORY_BUF_SIZE) {
        return 0;
    }

    while(!rbuff_has_space(&history_idx, 1) || !rbuff_has_space(&history_buf, len)) {
        history_shift();
    }

    history_push(len, data);

    // Make cursour at the end of historys
    history_cursor = rbuff_end(&history_idx);

    return 1;
}

int cupkee_history_load(int dir)
{
    history_index_t *index;
    unsigned i, start;

    history_cursor += dir;
    if (history_cursor < 0) {
        history_cursor = 0;
        return CON_EXECUTE_DEF;
    }

    if (history_cursor >= rbuff_end(&history_idx)){
        history_cursor = rbuff_end(&history_idx);
        return CON_EXECUTE_DEF;
    }
    index = &history_indexs[_rbuff_get(&history_idx, history_cursor)];

    console_input_clean();

    start = index->pos;
    for (i = 0; i < index->len; i++) {
        int pos = rbuff_get(&history_buf, start + i);

        if (pos < 0) {
            // assert(0);
            break;
        }

        console_input_char(history_memory[pos]);
    }

    return CON_PREVENT_DEF;
}

