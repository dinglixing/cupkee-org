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

#include "test_util.h"

static const char *lines[256];
static int max = 0;
static int cur = 0;
static int start = 0;

void test_clr_line(void)
{
    max = 0;
    cur = 0;
    start = 0;
}

void test_set_line(const char *line)
{
    if (max < 256) {
        lines[max++] = line;
    }
}

int test_get_line(void *buf, int size)
{
    const char *line_cur = lines[cur];
    int end, lft;

    if (cur >= max) {
        return 0;
    }

    end = strlen(line_cur);
    lft = end - start;
    if (lft > size) {
        memcpy(buf, line_cur + start, size);
        start += size;
        return size;
    } else {
        memcpy(buf, line_cur + start, lft);
        start = 0;
        cur++;

        return lft;
    }
}

struct sbuf_t {
    int  pos, end;
    char *buf;
};

