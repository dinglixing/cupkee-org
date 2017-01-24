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

static void print_value(val_t *v)
{
    if (val_is_number(v)) {
        char buf[32];
        if (*v & 0xffff) {
            snprintf(buf, 32, "%f", val_2_double(v));
        } else {
            snprintf(buf, 32, "%d", (int)val_2_double(v));
        }
        output(buf);
    } else
    if (val_is_boolean(v)) {
        output(val_2_intptr(v) ? "true" : "false");
    } else
    if (val_is_string(v)) {
        output("\"");
        output(val_2_cstring(v));
        output("\"");
    } else
    if (val_is_undefined(v)) {
        output("undefined");
    } else
    if (val_is_nan(v)) {
        output("NaN");
    } else
    if (val_is_function(v)) {
        char buf[32];
        snprintf(buf, 32, "function:%ld", val_2_intptr(v));
        output(buf);
    } else {
        output("object");
    }
}

static val_t print(env_t *env, int ac, val_t *av)
{
    int i;

    for (i = 0; i < ac; i++) {
        if (i > 0) {
            output(" ");
        }
        print_value(av+i);
    }
    output("\n");

    return val_mk_undefined();
}

static native_t native_entry[] = {
    {"print", print}
};

int native_init(env_t *env)
{
    return env_native_set(env, native_entry, 1);
}

