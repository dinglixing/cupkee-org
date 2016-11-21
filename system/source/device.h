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

#ifndef __DEVICE_INC__
#define __DEVICE_INC__

#include <cupkee.h>

void device_setup(void);
void device_event_proc(env_t *env, int event);

static inline int device_param_stream(int ac, val_t *av, void **addr, int *size) {
    if (ac) {
        if (val_is_buffer(av)) {
            *addr = buffer_addr(av);
            *size = buffer_size(av);
        } else
        if ((*size = string_len(av)) < 0) {
            return 0;
        } else {
            *addr = (void *) val_2_cstring(av);
        }
        return 1;
    }
    return 0;
}

static inline int device_param_int(int ac, val_t *av, int *n) {
    if (ac > 0 && val_is_number(av)) {
        *n = val_2_integer(av);
        return 1;
    } else {
        return 0;
    }
}

#endif /* __DEVICE_INC__ */

