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
#include "type_object.h"

static object_t object_proto;

static intptr_t object_prop_keys[3] = {(intptr_t)"length", (intptr_t)"toString", (intptr_t)"foreach"};
static val_t object_prop_vals[3];

static val_t *object_add_prop(env_t *env, object_t *obj, intptr_t symbal) {
    val_t *vals;
    intptr_t *keys;

    if (obj->prop_size <= obj->prop_num) {
        int size;

        if (obj->prop_size >= UINT16_MAX) {
            env_set_error(env, ERR_ResourceOutLimit);
            return NULL;
        }
        size = obj->prop_size * 2;
        size = size < UINT16_MAX ? size : UINT16_MAX;

        keys = (intptr_t*) env_heap_alloc(env, sizeof(intptr_t) * size + sizeof(val_t) * size);
        if (!keys) {
            env_set_error(env, ERR_NotEnoughMemory);
            return NULL;
        }
        vals = (val_t *)(keys + size);

        memcpy(keys, obj->keys, sizeof(intptr_t) * obj->prop_num);
        memcpy(vals, obj->vals, sizeof(val_t) * obj->prop_num);
        obj->keys = keys;
        obj->vals = vals;
    } else {
        vals = obj->vals;
        keys = obj->keys;
    }

    keys[obj->prop_num] = symbal;
    return vals + obj->prop_num++;
}

static val_t *object_find_prop(object_t *obj, intptr_t symbal) {
    object_t *cur = obj;
    int i;

    while (cur) {
        intptr_t *keys = cur->keys;
        for (i = 0; i < cur->prop_num; i++) {
            if (keys[i] == symbal) {
                return cur->vals + i;
            }
        }
        cur = cur->proto;
    }
    return NULL;
}

static val_t *object_find_prop_owned(object_t *obj, intptr_t symbal) {
    int i;

    for (i = 0; i < obj->prop_num; i++) {
        if (obj->keys[i] == symbal) {
            return obj->vals + i;
        }
    }
    return NULL;
}

static inline void object_static_register(env_t *env, object_t *o) {
    int i;

    for (i = 0; i < o->prop_num; i++) {
        env_symbal_add_static(env, (const char *)o->keys[i]);
    }
}

static val_t object_length(env_t *env, int ac, val_t *av)
{
    (void) env;

    if (ac > 0 && val_is_object(av)) {
        object_t *o = (object_t *)val_2_intptr(av);
        return val_mk_number(o->prop_num);
    } else {
        return val_mk_undefined();
    }
}

static val_t object_to_string(env_t *env, int ac, val_t *obj)
{
    (void) env;
    (void) ac;
    (void) obj;
    return val_mk_foreign_string((intptr_t)"Object");
}

static val_t object_foreach(env_t *env, int ac, val_t *av)
{
    if (ac > 1 && val_is_object(av) && val_is_function(av + 1)) {
        object_t *o = (object_t *)val_2_intptr(av);
        int i, max = o->prop_num;

        for (i = 0; i < max && !env->error; i++) {
            val_t key = val_mk_foreign_string(o->keys[i]);

            env_push_call_argument(env, &key);
            env_push_call_argument(env, o->vals + i);
            env_push_call_function(env, av + 1);

            interp_execute_call(env, 2);
        }
    }

    return val_mk_undefined();
}

intptr_t object_create(env_t *env, int n, val_t *av)
{
    object_t *obj;
    int size;

    if ((n & 1) || n > UINT16_MAX * 2) {
        return 0;
    }

    size = n / 2;
    size = size < DEF_PROP_SIZE ? DEF_PROP_SIZE : size;

    obj = (object_t *) env_heap_alloc(env, sizeof(object_t) + sizeof(intptr_t) * size + sizeof(val_t) * size);
    if (obj) {
        int i = 0, off = 0;

        obj->magic = MAGIC_OBJECT;
        obj->age = 0;
        obj->prop_size = size;
        obj->prop_num = n / 2;
        obj->keys = (intptr_t *)(obj + 1);
        obj->vals = (val_t *)(obj->keys + size);
        obj->proto = &object_proto;
        while (i < n) {
            val_t *k = av + i++;
            val_t *val = av + i++;
            const char *name = val_2_cstring(k);
            intptr_t key;

            if (!name) {
                return 0;
            }

            key = env_symbal_get(env, name);
            if (!key) {
                key = env_symbal_insert(env, name, !val_is_foreign_string(k));
            }

            if (!key) {
                return 0;
            }
            obj->keys[off] = key;
            obj->vals[off] = *val;

            off++;
        }
    }

    return (intptr_t) obj;
}

int objects_env_init(env_t *env)
{
    object_t *Object    = &object_proto;

    object_prop_vals[0] = val_mk_native((intptr_t) object_length);
    object_prop_vals[1] = val_mk_native((intptr_t) object_to_string);
    object_prop_vals[2] = val_mk_native((intptr_t) object_foreach);
    Object->magic = MAGIC_OBJECT_STATIC;
    Object->proto = NULL;
    Object->prop_num = 3;
    Object->keys = object_prop_keys;
    Object->vals = object_prop_vals;
    object_static_register(env, &object_proto);

    return env->error;
}

val_t *object_prop_ref(env_t *env, val_t *self, val_t *key)
{
    object_t *obj = (object_t *) val_2_intptr(self);
    const char *name = val_2_cstring(key);
    val_t *prop = NULL;

    if (name) {
        intptr_t sym_id = env_symbal_get(env, name);

        if (sym_id) {
            prop = object_find_prop_owned(obj, sym_id);
            if (prop) {
                return prop;
            }
            prop = object_add_prop(env, obj, sym_id);
        } else {
            prop = object_add_prop(env, obj, env_symbal_add(env, name));
        }

        if (prop) {
            val_set_undefined(prop);
        }
    }
    return prop;
}

void object_prop_val(env_t *env, val_t *self, val_t *key, val_t *prop)
{
    const char *name = val_2_cstring(key);
    object_t *obj;

    if (!name) {
        val_set_undefined(prop);
        return;
    }
    obj = (object_t *) val_2_intptr(self);
    if (obj) {
        val_t *v = object_find_prop(obj, env_symbal_get(env, name));
        if (v) {
            *prop = *v;
        } else {
            val_set_undefined(prop);
        }
    } else {
        val_set_undefined(prop);
        env_set_error(env, ERR_SysError);
    }
}

int object_iter_next(object_iter_t *it, const char **name, val_t **v)
{
    if (it->cur < it->obj->prop_num) {
        int id = it->cur++;

        *name = (const char *)(it->obj->keys[id]);
        *v = it->obj->vals + id;

        return 1;
    } else {
        return 0;
    }
}

