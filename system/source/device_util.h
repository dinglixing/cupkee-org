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

#ifndef __DEVICE_UTIL_INC__
#define __DEVICE_UTIL_INC__

int device_string_map(const char *name, int max, const char **str_list);
int device_string_map_var(val_t *in, int max, const char **str_list);

int device_set_uint8(val_t *val, uint8_t *conf);
int device_set_uint16(val_t *val, uint16_t *conf);
int device_set_uint32(val_t *val, uint32_t *conf);
int device_set_option(val_t *val, uint8_t *conf, int max, const char **opt_list);

void device_get_option(val_t *opt, int i, int max, const char **opt_list);

int device_convert_data(val_t *data, void **addr, int *size);

static inline int device_param_int(int ac, val_t *av, int *n) {
    if (ac > 0 && val_is_number(av)) {
        *n = val_2_integer(av);
        return 1;
    } else {
        return 0;
    }
}

#endif /* __DEVICE_UTIL_INC__ */

