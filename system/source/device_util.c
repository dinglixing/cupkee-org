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

#include "device.h"

int device_string_map(const char *name, int max, const char **str_list)
{
    int id;

    if (!name)
        return -1;

    for (id = 0; id < max && str_list[id]; id++) {
        if (!strcmp(name, str_list[id])) {
            return id;
        }
    }
    return -1;
}

int device_string_map_var(val_t *in, int max, const char **str_list)
{
    if (val_is_number(in)) {
        int id = val_2_integer(in);

        return (id >= max) ? -1 : id;
    } else {
        return device_string_map(val_2_cstring(in), max, str_list);
    }
}

void device_get_option(val_t *opt, int i, int max, const char **opt_list)
{
    if (i >= max) {
        i = 0;
    }

    val_set_foreign_string(opt, (intptr_t) opt_list[i]);
}

int device_set_uint8(val_t *val, uint8_t *conf)
{
    if (val_is_number(val)) {
        *conf = val_2_integer(val);
        return CUPKEE_OK;
    }
    return CUPKEE_EINVAL;
}

int device_set_uint16(val_t *val, uint16_t *conf)
{
    if (val_is_number(val)) {
        *conf = val_2_integer(val);
        return CUPKEE_OK;
    }
    return CUPKEE_EINVAL;
}

int device_set_uint32(val_t *val, uint32_t *conf)
{
    if (val_is_number(val)) {
        *conf = val_2_integer(val);
        return CUPKEE_OK;
    }
    return CUPKEE_EINVAL;
}

int device_set_option(val_t *val, uint8_t *conf, int max, const char **opt_list)
{
    int opt = device_string_map_var(val, max, opt_list);

    if (opt >= 0) {
        *conf = opt;
        return CUPKEE_OK;
    }
    return CUPKEE_EINVAL;
}

int device_convert_data(val_t *data, void **addr, int *size)
{
    if (val_is_buffer(data)) {
        *size = _val_buffer_size(data);
        *addr = _val_buffer_addr(data);
    } else
    if ((*size = string_len(data)) < 0) {
        return 0;
    } else {
        *addr = (void *) val_2_cstring(data);
    }
    return 1;
}

