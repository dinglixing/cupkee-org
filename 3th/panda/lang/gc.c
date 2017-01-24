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

#include "val.h"
#include "heap.h"
#include "gc.h"
#include "type_string.h"
#include "type_function.h"
#include "type_array.h"
#include "type_object.h"
#include "type_buffer.h"

#define MAGIC_BYTE(x) (*((uint8_t *)(x)))
#define ADDR_VALUE(x) (*((void **)(x)))

static scope_t *heap_dup_scope(heap_t *heap, scope_t *scope)
{
    scope_t *dup;
    val_t   *buf = NULL;

    //dup = heap_alloc(heap, sizeof(scope_t) + sizeof(val_t) * scope->num);
    dup = heap_alloc(heap, scope_mem_space(scope));
    buf = (val_t *)(dup + 1);

    //printf("%s: free %d\n", __func__, heap->free);
    memcpy(dup, scope, sizeof(scope_t));
    memcpy(buf, scope->var_buf, sizeof(val_t) * scope->num);
    dup->var_buf = buf;

    ADDR_VALUE(scope) = dup;
    return dup;
}

static object_t *heap_dup_object(heap_t *heap, object_t *obj)
{
    object_t *dup;
    intptr_t *keys;
    val_t    *vals;

    //dup = heap_alloc(heap, sizeof(scope_t) + sizeof(val_t) * scope->num);
    dup = heap_alloc(heap, object_mem_space(obj));
    keys = (intptr_t *) (dup + 1);
    vals = (val_t *)(keys + obj->prop_size);

    //printf("%s: free %d\n", __func__, heap->free);
    memcpy(dup, obj, sizeof(object_t));
    memcpy(keys, obj->keys, sizeof(intptr_t) * obj->prop_num);
    memcpy(vals, obj->vals, sizeof(val_t) * obj->prop_num);
    dup->keys = keys;
    dup->vals = vals;

    ADDR_VALUE(obj) = dup;

    return dup;
}

static array_t *heap_dup_array(heap_t *heap, array_t *a)
{
    array_t *dup;
    val_t   *vals;

    //dup = heap_alloc(heap, sizeof(scope_t) + sizeof(val_t) * scope->num);
    dup = heap_alloc(heap, array_mem_space(a));
    vals = (val_t *)(dup + 1);

    //printf("%s: free %d\n", __func__, heap->free);
    memcpy(dup, a, sizeof(array_t));
    memcpy(vals, array_values(a), sizeof(val_t) * array_len(a));
    dup->elems = vals;

    ADDR_VALUE(a) = dup;

    return dup;
}

static intptr_t heap_dup_string(heap_t *heap, intptr_t str)
{
    int size = string_mem_space(str);
    void *dup = heap_alloc(heap, size);

    //printf("%s: free %d, %d, %s\n", __func__, heap->free, size, (char *)(str + 3));
    //printf("[str size: %d, '%s']", size, (char *)str + 3);
    memcpy(dup, (void*)str, size);
    //printf("[dup size: %d, '%s']", string_mem_space((intptr_t)dup), dup + 3);

    ADDR_VALUE(str) = dup;
    return (intptr_t) dup;
}

static void *heap_dup_buffer(heap_t *heap, type_buffer_t *buf)
{
    int size = buffer_mem_space(buf);
    void *dup = heap_alloc(heap, size);

    //printf("%s: free %d, %d, %s\n", __func__, heap->free, size, (char *)(str + 3));
    //printf("[str size: %d, '%s']", size, (char *)str + 3);
    memcpy(dup, (void*)buf, size);
    //printf("[dup size: %d, '%s']", string_mem_space((intptr_t)dup), dup + 3);

    ADDR_VALUE(buf) = dup;
    return dup;
}

static intptr_t heap_dup_foreign(heap_t *heap, val_foreign_t *foreign)
{
    int size = foreign_mem_space(foreign);
    void *dup = heap_alloc(heap, size);

    //printf("%s: free %d, %d, %s\n", __func__, heap->free, size, (char *)(str + 3));
    //printf("[str size: %d, '%s']", size, (char *)str + 3);
    memcpy(dup, (void*)foreign, size);
    //printf("[dup size: %d, '%s']", string_mem_space((intptr_t)dup), dup + 3);

    ADDR_VALUE(foreign) = dup;
    return (intptr_t) dup;
}

static intptr_t heap_dup_function(heap_t *heap, intptr_t func)
{
    function_t *dup = heap_alloc(heap, function_mem_space((function_t*)func));

    //printf("%s: free %d\n", __func__, heap->free);
    memcpy(dup, (void*)func, sizeof(function_t));

    ADDR_VALUE(func) = dup;
    return (intptr_t) dup;
}

scope_t *gc_copy_scope(heap_t *heap, scope_t *scope)
{
    if (!scope || heap_is_owned(heap, scope)) {
        return scope;
    }

    if (MAGIC_BYTE(scope) != MAGIC_SCOPE) {
        return ADDR_VALUE(scope);
    }

    return heap_dup_scope(heap, scope);
}

static inline intptr_t gc_copy_string(heap_t *heap, intptr_t str)
{
    if (!str || heap_is_owned(heap, (void*)str)) {
        return (intptr_t) str;
    }

    if (MAGIC_BYTE(str) != MAGIC_STRING) {
        return (intptr_t) ADDR_VALUE(str);
    }

    return heap_dup_string(heap, str);
}

static inline intptr_t gc_copy_function(heap_t *heap, intptr_t func)
{
    if (!func || heap_is_owned(heap, (void *)func)) {
        return (intptr_t) func;
    }

    if (MAGIC_BYTE(func) != MAGIC_FUNCTION) {
        return (intptr_t) ADDR_VALUE(func);
    }

    return heap_dup_function(heap, func);
}

static inline array_t *gc_copy_array(heap_t *heap, array_t *a)
{
    if (!a || heap_is_owned(heap, a)) {
        return a;
    }

    if (MAGIC_BYTE(a) != MAGIC_ARRAY) {
        return ADDR_VALUE(a);
    }

    return heap_dup_array(heap, a);
}

static inline void *gc_copy_buffer(heap_t *heap, type_buffer_t *buf)
{
    if (!buf || heap_is_owned(heap, (void*)buf)) {
        return buf;
    }

    if (MAGIC_BYTE(buf) != MAGIC_BUFFER) {
        return ADDR_VALUE(buf);
    }

    return heap_dup_buffer(heap, buf);
}

static inline object_t *gc_copy_object(heap_t *heap, object_t *obj)
{
    if (!obj || MAGIC_BYTE(obj) == MAGIC_OBJECT_STATIC || heap_is_owned(heap, obj)) {
        return obj;
    }

    if (MAGIC_BYTE(obj) != MAGIC_OBJECT) {
        return ADDR_VALUE(obj);
    }

    return heap_dup_object(heap, obj);
}

static inline intptr_t gc_copy_foreign(heap_t *heap, val_foreign_t *foreign)
{
    if (!foreign || heap_is_owned(heap, foreign)) {
        return (intptr_t) foreign;
    }

    if (MAGIC_BYTE(foreign) != MAGIC_FOREIGN) {
        return (intptr_t) ADDR_VALUE(foreign);
    }

    return heap_dup_foreign(heap, foreign);
}

void gc_copy_vals(heap_t *heap, int vc, val_t *vp)
{
    int i = 0;

    while (i < vc) {
        val_t *v = vp + i;

        if (val_is_heap_string(v)) {
            val_set_heap_string(v, gc_copy_string(heap, val_2_intptr(v)));
        } else
        if (val_is_script(v)) {
            val_set_script(v, gc_copy_function(heap, val_2_intptr(v)));
        } else
        if (val_is_object(v)) {
            val_set_object(v, (intptr_t)gc_copy_object(heap, (object_t *)val_2_intptr(v)));
        } else
        if (val_is_array(v)) {
            val_set_array(v, (intptr_t)gc_copy_array(heap, (array_t *)val_2_intptr(v)));
        } else
        if (val_is_buffer(v)) {
            val_set_buffer(v, gc_copy_buffer(heap, (type_buffer_t *)val_2_intptr(v)));
        } else
        if (val_is_foreign(v)) {
            val_set_foreign(v, (intptr_t)gc_copy_foreign(heap, (val_foreign_t *)val_2_intptr(v)));
        }
        i++;
    }
}

void gc_scan(heap_t *heap)
{
    uint8_t*base = heap->base;
    int     scan = 0;

    while(scan < heap->free) {
        uint8_t magic = base[scan];

        switch(magic) {
        case MAGIC_STRING:
            scan += string_mem_space((intptr_t)(base + scan));
            break;
        case MAGIC_FUNCTION: {
            function_t *func = (function_t *)(base + scan);
            scan += function_mem_space(func);

            func->super = gc_copy_scope(heap, func->super);

            break;
            }
        case MAGIC_SCOPE: {
            scope_t *scope = (scope_t *) (base + scan);
            scan += scope_mem_space(scope);

            scope->super = gc_copy_scope(heap, scope->super);
            gc_copy_vals(heap, scope->num, scope->var_buf);

            break;
            }
        case MAGIC_OBJECT: {
            object_t *obj = (object_t *) (base + scan);
            scan += object_mem_space(obj);

            obj->proto = gc_copy_object(heap, obj->proto);
            gc_copy_vals(heap, obj->prop_num, obj->vals);

            break;
            }
        case MAGIC_ARRAY: {
            array_t *array= (array_t*) (base + scan);
            scan += array_mem_space(array);

            gc_copy_vals(heap, array_len(array), array_values(array));

            break;
            }
        case MAGIC_BUFFER:
            scan += buffer_mem_space((type_buffer_t *)(base + scan));
            break;
        case MAGIC_FOREIGN:
            scan += foreign_mem_space((val_foreign_t *) (base + scan));
            break;
        default: break;
        }
    }
}

