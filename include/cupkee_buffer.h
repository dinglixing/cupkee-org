/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016,2017 Lixing Ding <ding.lixing@gmail.com>

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

#ifndef __CUPKEE_BUFFER_INC__
#define __CUPKEE_BUFFER_INC__

void cupkee_buffer_init(void);

void   *cupkee_buffer_alloc(size_t size);
void   cupkee_buffer_release(void *b);
void   cupkee_buffer_reset(void *b);
size_t cupkee_buffer_capacity(void *b);
size_t cupkee_buffer_space(void *b);
size_t cupkee_buffer_length(void *b);
int    cupkee_buffer_is_empty(void *b);
int    cupkee_buffer_is_full(void *b);
int    cupkee_buffer_push(void *b, uint8_t d);
int    cupkee_buffer_shift(void *b, uint8_t *d);
int    cupkee_buffer_take(void *b, size_t n, void *buf);
int    cupkee_buffer_give(void *b, size_t n, const void *buf);

#endif /* __CUPKEE_BUFFER_INC__ */

