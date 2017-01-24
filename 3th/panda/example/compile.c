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

#include "example.h"

#define MEM_SIZE  10240
static char *MEM_PTR[MEM_SIZE];

static int set_output_name(const char *input, void *buf, int sz)
{
    int pos = file_base_name(input, buf, sz - 5);

    if (pos < 1) {
        return -1;
    }

    memcpy(buf + pos, ".pdc", 5);

    return 0;
}

#define OUTPUT_NAME_MAX     (128)

static int compile(const char *input, void *mem_ptr, int mem_size)
{
    char *output  = (char *) mem_ptr;
    void *cpl_mem, *exe_mem;
    env_t env;
    int cpl_mem_sz, exe_mem_sz, input_sz;
    int exe_sz;

    exe_mem_sz = (mem_size / 3) & (~0xf);
    cpl_mem_sz = mem_size - OUTPUT_NAME_MAX - exe_mem_sz;

    exe_mem = mem_ptr + OUTPUT_NAME_MAX;
    cpl_mem = mem_ptr + OUTPUT_NAME_MAX + exe_mem_sz;

    if (0 != set_output_name(input, output, OUTPUT_NAME_MAX)) {
        return -1;
    }

    if (0 != compile_env_init(&env, cpl_mem, cpl_mem_sz)) {
        return -1;
    }

    native_init(&env);

    input = file_load(input, &input_sz);
    if (!input) {
        return -1;
    }

    exe_sz = compile_exe(&env, input, exe_mem, exe_mem_sz);
    file_release((void *)input, input_sz);

    if (exe_sz <= 0) {
        return -1;
    } else {
        return file_store(output, exe_mem, exe_sz);
    }
}

int main(int ac, char **av)
{
    int   error;

    if (ac == 1) {
        printf("Usage: %s <input>\n", av[0]);
        return 0;
    }

    if (0 > (error = compile(av[1], MEM_PTR, MEM_SIZE))) {
        printf("compile: %s fail:%d\n", av[1], error);
    }

    return error ? 1 : 0;
}

