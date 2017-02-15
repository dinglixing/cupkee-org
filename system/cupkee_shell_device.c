/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016-2017 Lixing Ding <ding.lixing@gmail.com>

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

#include <cupkee.h>

#include "cupkee_shell_misc.h"

static int device_is_true(intptr_t ptr);
static void device_op_prop(void *env, intptr_t ptr, val_t *name, val_t *prop);
static void device_op_elem(void *env, intptr_t devid, val_t *which, val_t *elem);

static char *category_names[3] = {
    "M", "S", "B"
};

static const val_foreign_op_t device_op = {
    .is_true = device_is_true,
    .prop = device_op_prop,
    .elem = device_op_elem,
};

static const char *device_category_name(uint8_t category)
{
    if (category < DEVICE_CATEGORY_MAX) {
        return category_names[category];
    } else {
        return "?";
    }
}

static void device_list(void)
{
    const cupkee_device_desc_t *desc;
    int i;

    console_log_sync("%8s%6s%6s%s:%s\r\n", "DEVICE", "CONF", "INST", "TYPE", "CATEGORY");
    for (i = 0, desc = device_entrys[0]; desc; desc = device_entrys[++i]) {
        console_log_sync("%8s%6d%6d%d:%s\r\n",
                desc->name,
                desc->conf_num,
                hw_device_instances(desc->type),
                desc->type,
                device_category_name(desc->category));
    }
}

static int device_is_true(intptr_t ptr)
{
    (void) ptr;

    return 0;
}

static void device_op_prop(void *env, intptr_t ptr, val_t *name, val_t *prop)
{
    (void) env;
    (void) ptr;
    (void) name;

    val_set_undefined(prop);
}

static void device_op_elem(void *env, intptr_t ptr, val_t *which, val_t *elem)
{
    (void) env;
    (void) ptr;
    (void) which;

    val_set_undefined(elem);
}

val_t native_device_create(env_t *env, int ac, val_t *av)
{
    cupkee_device_t *device = NULL;
    const char *name;
    int instance;

    if (ac == 0) {
        device_list();
        return VAL_UNDEFINED;
    }

    name = val_2_cstring(av);
    if (!name) {
        return VAL_UNDEFINED;
    }

    if (ac > 1 && val_is_number(av + 1)) {
        instance = val_2_integer(av + 1);
    } else {
        instance = 0;
    }

    device = cupkee_device_request(name, instance);
    if (device) {
        return val_create(env, &device_op, (intptr_t) device);
    } else {
        return VAL_UNDEFINED;
    }
}

