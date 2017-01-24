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

#define HEAP_SIZE     (1024 * 400)
#define STACK_SIZE    (1024)
#define EXE_MEM_SPACE (1024 * 100)
#define SYM_MEM_SPACE (1024 * 4)
#define MEM_SIZE      (STACK_SIZE * sizeof(val_t) + HEAP_SIZE + EXE_MEM_SPACE + SYM_MEM_SPACE)

static uint8_t memory[MEM_SIZE];

static int panda_binary(const char *input, void *mem_ptr, int mem_size, int heap_size, int stack_size)
{
    env_t env;
    val_t *res;
    int err, size;
    uint8_t *binary;
    image_info_t ef;

    binary = file_load(input, &size);
    if (!binary) {
        return -1;
    }

    if (0 != image_load(&ef, binary, size)) {
        file_release((void *)input, size);
        return -1;
    }

    if (0 != interp_env_init_image (&env, mem_ptr, mem_size, NULL, heap_size, NULL, stack_size, &ef)) {
        file_release((void *)input, size);
        return -1;
    }
    native_init(&env);

    err = interp_execute_image(&env, &res);
    if (err < 0) {
        printf("error: %d\n", err);
    }

    file_release((void *)input, size);

    return err;
}

static int panda_string(const char *input, void *mem_ptr, int mem_size, int heap_size, int stack_size)
{
    env_t env;
    val_t *res;
    int err, size;

    input = file_load(input, &size);
    if (!input) {
        return -1;
    }

    if(0 != interp_env_init_interpreter(&env, mem_ptr, mem_size, NULL, heap_size, NULL, stack_size)) {
        file_release((void *)input, size);
        return -1;
    }
    native_init(&env);

    err = interp_execute_string(&env, input, &res);
    if (err < 0) {
        printf("error: %d\n", err);
    }

    file_release((void *)input, size);

    return err;
}

static inline int interpreter(const char *input, int ac, char **av) {
    char *suffix;

    suffix = rindex(input, '.');
    if (suffix && !strcmp(suffix, ".pdc")) {
        return panda_binary(input, memory, MEM_SIZE, HEAP_SIZE, STACK_SIZE);
    } else {
        return panda_string(input, memory, MEM_SIZE, HEAP_SIZE, STACK_SIZE);
    }
}

int main(int ac, char **av)
{
    char *input;
    int   error;

    if (ac == 1) {
        printf("Usage: %s <input>\n", av[0]);
        return 0;
    }
    input = av[1];

    error = interpreter(input, ac - 1, av + 1);
    if (error < 0) {
        printf("execute %s fail:%d\n", input, error);
    }

    return error ? 1 : 0;
}

