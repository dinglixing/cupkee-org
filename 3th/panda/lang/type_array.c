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

#include "interp.h"

#include "type_number.h"
#include "type_string.h"
#include "type_array.h"

static array_t *array_space_extend_tail(env_t *env, val_t *self, int n)
{
    array_t *a = (array_t *)val_2_intptr(self);
    val_t *elems;
    int len;

    if (a->elem_size - a->elem_end > n) {
        return a;
    }
    len = array_len(a);

    if (a->elem_size -len > n) {
        memmove(a->elems, a->elems + a->elem_bgn, len);
        a->elem_bgn = 0;
        a->elem_end = len;
        return a;
    } else {
        int size = SIZE_ALIGN_16(len + n);
        if (size > UINT16_MAX) {
            env_set_error(env, ERR_ResourceOutLimit);
            return NULL;
        }
        elems = env_heap_alloc(env, size * sizeof(val_t));
        if (elems) {
            a = (array_t *)val_2_intptr(self);
            memcpy(elems, a->elems + a->elem_bgn, sizeof(val_t) * len);
            a->elem_size = size;
            a->elem_bgn = 0;
            a->elem_end = len;
            return a;
        } else {
            return NULL;
        }
    }
}

static array_t *array_space_extend_head(env_t *env, val_t *self, int n)
{
    array_t *a = (array_t *)val_2_intptr(self);
    val_t *elems;
    int len;

    if (a->elem_bgn > n) {
        return a;
    }
    len = array_len(a);

    if (a->elem_size - len > n) {
        n = a->elem_size - a->elem_end;
        memmove(a->elems + a->elem_bgn + n, a->elems + a->elem_bgn, len);
        a->elem_bgn += n;
        a->elem_end += n;
        return a;
    } else {
        int size = SIZE_ALIGN_16(len + n);
        if (size > UINT16_MAX) {
            env_set_error(env, ERR_ResourceOutLimit);
            return NULL;
        }
        elems = env_heap_alloc(env, size * sizeof(val_t));
        if (elems) {
            a = (array_t *)val_2_intptr(self);
            memcpy(elems + size - len, a->elems + a->elem_bgn, sizeof(val_t) * len);
            a->elem_size = size;
            a->elem_bgn = size - len;
            a->elem_end = size;
            return a;
        } else {
            return NULL;
        }
    }
}

array_t *_array_create(env_t *env, int n)
{
    array_t *array;
    int size = n < DEF_ELEM_SIZE ? DEF_ELEM_SIZE : n;

    if (size > UINT16_MAX) {
        env_set_error(env, ERR_ResourceOutLimit);
        return NULL;
    }

    array = env_heap_alloc(env, sizeof(array_t) + sizeof(val_t) * size);
    if (array) {
        val_t *vals = (val_t *)(array + 1);

        array->magic = MAGIC_ARRAY;
        array->age = 0;
        array->elem_size = size;
        array->elem_bgn  = 0;
        array->elem_end  = n;
        array->elems = vals;
    }

    return array;
}

intptr_t array_create(env_t *env, int ac, val_t *av)
{
    array_t *array = _array_create(env, ac);

    if (array) {
        memcpy(array->elems + array->elem_bgn, av, sizeof(val_t) * ac);
    }

    return (intptr_t) array;
}

val_t *array_elem_ref(val_t *self, int i)
{
    array_t *array = (array_t *) val_2_intptr(self);

    if (i >= 0 && i < array_len(array)) {
        return array->elems + (array->elem_bgn + i);
    } else {
        return NULL;
    }
}

void array_elem_val(val_t *self, int i, val_t *elem)
{
    val_t *ref = array_elem_ref(self, i);

    if (ref) {
        *elem = *ref;
    } else {
        val_set_undefined(elem);
    }
}

val_t array_push(env_t *env, int ac, val_t *av)
{
    if (ac > 1 && val_is_array(av)) {
        int n = ac - 1;
        array_t *a = array_space_extend_tail(env, av, n);

        if (a) {
            memcpy(a->elems + a->elem_end, av + 1, sizeof(val_t) * n);
            a->elem_end += n;
            return val_mk_number(array_len(a));
        }
    } else {
        env_set_error(env, ERR_InvalidInput);
    }
    return val_mk_undefined();
}

val_t array_unshift(env_t *env, int ac, val_t *av)
{
    if (ac > 1 && val_is_array(av)) {
        int n = ac - 1;
        array_t *a = array_space_extend_head(env, av, n);

        if (a) {
            memcpy(a->elems + a->elem_bgn - n, av + 1, sizeof(val_t) * n);
            a->elem_bgn -= n;
            return val_mk_number(array_len(a));
        }
    } else {
        env_set_error(env, ERR_InvalidInput);
    }
    return val_mk_undefined();
}

val_t array_pop(env_t *env, int ac, val_t *av)
{
    if (ac > 0 && val_is_array(av)) {
        array_t *a = (array_t *)val_2_intptr(av);

        if (array_len(a)) {
            return a->elems[--a->elem_end];
        }
    } else {
        env_set_error(env, ERR_InvalidInput);
    }
    return val_mk_undefined();
}

val_t array_shift(env_t *env, int ac, val_t *av)
{
    if (ac > 0 && val_is_array(av)) {
        array_t *a = (array_t *)val_2_intptr(av);

        if (array_len(a)) {
            return a->elems[a->elem_bgn++];
        }
    } else {
        env_set_error(env, ERR_InvalidInput);
    }
    return val_mk_undefined();
}

val_t array_foreach(env_t *env, int ac, val_t *av)
{
    if (ac > 1 && val_is_array(av) && val_is_function(av + 1)) {
        array_t *a = (array_t *)val_2_intptr(av);
        int i, max = array_len(a);

        for (i = 0; i < max && !env->error; i++) {
            val_t key = val_mk_number(i);

            env_push_call_argument(env, &key);
            env_push_call_argument(env, array_values(a) + i);
            env_push_call_function(env, av + 1);

            interp_execute_call(env, 2);
        }
    }

    return val_mk_undefined();
}

val_t array_length(env_t *env, int ac, val_t *av)
{
    int len;

    (void) env;

    if (ac > 0 && val_is_array(av)) {
        array_t *a = (array_t *)val_2_intptr(av);
        len = array_len(a);
    } else {
        len = 0;
    }

    return val_mk_number(len);
}

