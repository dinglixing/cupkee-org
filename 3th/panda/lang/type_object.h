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


#ifndef __LANG_OBJECT_INC__
#define __LANG_OBJECT_INC__

#include "config.h"

#include "val.h"
#include "env.h"
#include "err.h"

#define MAGIC_OBJECT (MAGIC_BASE + 7)
#define MAGIC_OBJECT_STATIC (MAGIC_BASE + 9)

typedef struct object_t {
    uint8_t magic;
    uint8_t age;
    uint8_t reserved[2];
    uint16_t prop_size;
    uint16_t prop_num;
    struct object_t   *proto;
    intptr_t *keys;
    val_t    *vals;
} object_t;

typedef struct object_iter_t {
    object_t *obj;
    int       cur;
} object_iter_t;

int objects_env_init(env_t *env);

static inline int object_is_true(val_t *v) {
    object_t *o = (object_t *)val_2_intptr(v);
    return o->prop_num;
};
intptr_t object_create(env_t *env, int n, val_t *av);
void   object_prop_val(env_t *env, val_t *self, val_t *key, val_t *prop);
val_t *object_prop_ref(env_t *env, val_t *self, val_t *key);

static inline int object_mem_space(object_t *o) {
    return SIZE_ALIGN(sizeof(object_t) + (sizeof(intptr_t) + sizeof(val_t)) * o->prop_size);
};

static inline void _object_iter_init(object_iter_t *it, object_t *obj) {
    it->obj = obj;
    it->cur = 0;
};

static inline int object_iter_init(object_iter_t *it, val_t *obj)
{
    if (val_is_object(obj)) {
        _object_iter_init(it, (object_t *)val_2_intptr(obj));
        return 0;
    }
    return -1;
}

int object_iter_next(object_iter_t *it, const char **k, val_t **v);

#endif /* __LANG_OBJECT_INC__ */

