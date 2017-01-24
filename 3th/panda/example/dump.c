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

static int dump_image(const char *file)
{
    int err, size;
    image_info_t image;
    uint8_t *binary;

    binary = file_load(file, &size);
    if (!binary) {
        printf("file load fail: %s\n", file);
        return -1;
    }

    if (0 == (err = image_load(&image, binary, size))) {
        int i;
        double *numbers = image_number_entry(&image);

        printf("================ executable image file ==================\n");
        printf("+ Version  : %d\n", 0);
        printf("+ AddrSize : %d\n", image.addr_size == ADDRSIZE_32 ? 32 : 64);
        printf("+ ByteOrder: %s\n", image.byte_order == LE ? "LittleEndian" : "BigEndian");
        printf("+ Number count: %d\n", image.num_cnt);
        printf("+ String count: %d\n", image.str_cnt);
        printf("+ Function count: %d\n", image.fn_cnt);
        printf("-------------------- static numbers ---------------------\n");
        for (i = 0; i < image.num_cnt; i++) {
            printf("N[%d] %f\n", i, numbers[i]);
        }
        printf("-------------------- static strings ---------------------\n");
        for (i = 0; i < image.str_cnt; i++) {
            printf("S[%d] %s\n", i, image_get_string(&image, i));
        }
        printf("----------------------- functions -----------------------\n");
        for (i = 0; i < image.fn_cnt; i++) {
            const uint8_t *entry = image_get_function(&image, i);
            const uint8_t *code = executable_func_get_code(entry);
            uint32_t size = executable_func_get_code_size(entry);
            int off = 0;

            printf("\n* Function[%d] %c\n", i, executable_func_is_closure(entry) ? '*' : ' ');
            printf("* variables: %u, arguments: %u, stack_need: %u, code_size: %u\n",
                    executable_func_get_var_cnt(entry), executable_func_get_arg_cnt(entry),
                    executable_func_get_stack_high(entry), size);
            while (off < size) {
                const char *name;
                int p1, p2, pos = off;
                int n = bcode_parse(code, &off, &name, &p1, &p2);
                if (n == 2) {
                    printf("[%4d] %s %d %d\n", pos, name, p1, p2);
                } else if (n == 1) {
                    printf("[%4d] %s %d\n", pos, name, p1);
                } else {
                    printf("[%4d] %s\n", pos, name);
                }
            }
        }
        printf("========================= end ===========================\n");
    } else {
        printf("Invalid image file: %s\n", file);
    }

    file_release((void *)binary, size);

    return err;
}

int main(int ac, char **av)
{
    int   error;

    if (ac == 1) {
        printf("Usage: %s <input>\n", av[0]);
        return 0;
    }

    error = dump_image(av[1]);
    if (error < 0) {
        printf("dump: %s fail:%d\n", av[1], error);
    }

    return error ? 1 : 0;
}

