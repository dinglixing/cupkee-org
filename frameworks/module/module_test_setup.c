/*
MIT License

This file is part of cupkee project.

Copyright (c) 2017 Lixing Ding <ding.lixing@gmail.com>

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

#include "module_test.h"

static val_t native_module_add(env_t *env, int ac, val_t *av)
{
    (void) env;
    (void) ac;

    int a = val_2_integer(av);
    int b = val_2_integer(av + 1);

    return val_mk_number(a + b);
}

static const native_t native_entries[] = {
    {"add",   native_module_add},
    {"print", native_print},
    {"clearInterval",   native_clear_interval},
};

void module_test_setup(void)
{
    module_example_init();
}

int module_test_native_number(void)
{
    return sizeof(native_entries) / sizeof(native_t);
}

const native_t *module_test_native_entries(void)
{
    return native_entries;
}

const char *module_test_script(void)
{
    return "\
print('test mod add:') \
if (10 == add(1, 9)) { \
  print('OK') \
} else { \
  print('Fail') \
}";
}

