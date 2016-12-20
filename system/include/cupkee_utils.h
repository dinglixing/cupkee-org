/*
MIT License

This file is part of cupkee project.

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

#ifndef __CUPKEE_UTILS_INC__
#define __CUPKEE_UTILS_INC__

void *cupkee_buf_alloc(void);
void  cupkee_buf_release(void *b);
void  cupkee_buf_reset(void *b);
int cupkee_buf_is_empty(void *b);
int cupkee_buf_is_full(void *b);
int cupkee_buf_push(void *b, uint8_t d);
int cupkee_buf_shift(void *b, uint8_t *d);
int cupkee_buf_gets(void *b, int n, void *buf);
int cupkee_buf_puts(void *b, int n, void *buf);

#endif /* __CUPKEE_UTILS_INC__ */

