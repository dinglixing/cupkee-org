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

#ifndef __LANG_INTERP_INC__
#define __LANG_INTERP_INC__

#include "config.h"

#include "val.h"
#include "env.h"
#include "executable.h"

int interp_env_init_interactive(env_t *env, void *mem_ptr, int mem_size, void *heap_ptr, int heap_size, val_t *stack_ptr, int stack_size);
int interp_env_init_interpreter(env_t *env, void *mem_ptr, int mem_size, void *heap_ptr, int heap_size, val_t *stack_ptr, int stack_size);
int interp_env_init_image(env_t *env, void *mem_ptr, int mem_size, void *heap_ptr, int heap_size, val_t *stack_ptr, int stack_size, image_info_t *image);

int interp_execute_interactive(env_t *env, const char *input, char *(*input_more)(void), val_t **v);
int interp_execute_string(env_t *env, const char *input, val_t **result);
int interp_execute_image(env_t *env, val_t **result);

val_t interp_execute_call(env_t *env, int ac);

#endif /* __LANG_INTERP_INC__ */

