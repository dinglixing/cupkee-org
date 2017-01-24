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

#ifndef __LANG_VAL_INC__
#define __LANG_VAL_INC__

#include "config.h"

typedef uint64_t val_t;

typedef union {
    val_t   v;
    double  d;
} valnum_t;

extern const val_t _Undefined;
extern const val_t _True;
extern const val_t _False;
extern const val_t _NaN;
extern const val_t _Infinity;

/*
 *  Double-precision floating-point number, IEEE 754
 *
 *  64 bit (8 bytes) in total
 *  1  bit sign
 *  11 bits exponent
 *  52 bits mantissa
 *      7         6        5        4        3        2        1        0
 *  seeeeeee|eeeemmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm
 *
 * If an exponent is all-1 and mantissa is all-0, then it is an INFINITY:
 *  11111111|11110000|00000000|00000000|00000000|00000000|00000000|00000000
 *
 * If an exponent is all-1 and mantissa's MSB is 1, it is a quiet NaN:
 *  11111111|11111000|00000000|00000000|00000000|00000000|00000000|00000000
 *
 * object_t NaN-packing:
 *    sign and exponent is 0xfff
 *    4 bits specify type (tag), must be non-zero
 *    48 bits specify value
 *
 *  11111111|1111tttt|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv
 *   NaN marker |type|  48-bit placeholder for values: pointers, strings
 *
 * On 64-bit platforms, pointers are really 48 bit only, so they can fit,
 * provided they are sign extended
 */
#define MAKE_TAG(s, t) \
  ((val_t)(s) << 63 | (val_t) 0x7ff0 <<48 | (val_t)(t) <<48)

#define TYPE_NUM            0       // number or infinity
#define TYPE_STR_I          1       // string (inner/inline)
#define TYPE_STR_H          2       // string (heap)
#define TYPE_STR_F          3       // string (foreign)
#define TYPE_BOOL           4       // boolean
#define TYPE_FUNC           5       // function script
#define TYPE_FUNC_C         6       // function c
#define TYPE_UND            7       // undefined
#define TYPE_NAN            8       // not a number
#define TYPE_ARRAY          9       // array
#define TYPE_BUF            10      // buffer
#define TYPE_ERR            11      // error
#define TYPE_DATE           12      // date
#define TYPE_OBJ            13      // object
#define TYPE_FOREIGN        14      // object (foreign)
#define TYPE_REF            15      // reference to variable
#define TYPE_MAX            16

#define TAG_INFINITE        MAKE_TAG(0, TYPE_NUM)
#define TAG_STRING_I        MAKE_TAG(1, TYPE_STR_I)
#define TAG_STRING_H        MAKE_TAG(1, TYPE_STR_H)
#define TAG_STRING_F        MAKE_TAG(1, TYPE_STR_F)
#define TAG_BOOLEAN         MAKE_TAG(1, TYPE_BOOL)
#define TAG_FUNC_SCRIPT     MAKE_TAG(1, TYPE_FUNC)
#define TAG_FUNC_NATIVE     MAKE_TAG(1, TYPE_FUNC_C)

#define TAG_UNDEFINED       MAKE_TAG(1, TYPE_UND)
#define TAG_NAN             MAKE_TAG(1, TYPE_NAN)

#define TAG_ARRAY           MAKE_TAG(1, TYPE_ARRAY)
#define TAG_BUFFER          MAKE_TAG(1, TYPE_BUF)
#define TAG_ERROR           MAKE_TAG(1, TYPE_ERR)
#define TAG_DATE            MAKE_TAG(1, TYPE_DATE)
#define TAG_OBJECT          MAKE_TAG(1, TYPE_OBJ)
#define TAG_FOREIGN         MAKE_TAG(1, TYPE_FOREIGN)
#define TAG_REFERENCE       MAKE_TAG(1, TYPE_REF)

#define NUM_MASK            MAKE_TAG(0, 0xF)
#define TAG_MASK            MAKE_TAG(1, 0xF)
#define VAL_MASK            (~MAKE_TAG(1, 0xF))

#define VAL_UNDEFINED       TAG_UNDEFINED
#define VAL_NULL            TAG_OBJECT
#define VAL_INFINITE        TAG_INFINITE
#define VAL_NAN             TAG_NAN
#define VAL_TRUE            (TAG_BOOLEAN | 1)
#define VAL_FALSE           (TAG_BOOLEAN)

#define MAGIC_FOREIGN       (MAGIC_BASE + 15)

typedef struct val_foreign_op_t {
    int (*is_true)(intptr_t self);
    int (*is_equal)(intptr_t self, val_t *av);
    int (*is_gt)(intptr_t self, val_t *av);
    int (*is_ge)(intptr_t self, val_t *av);
    int (*is_lt)(intptr_t self, val_t *av);
    int (*is_le)(intptr_t self, val_t *av);
    void (*neg)(void *env, intptr_t self, val_t *result);
    void (*not)(void *env, intptr_t self, val_t *result);
    void (*inc)(void *env, intptr_t self, val_t *result);
    void (*dec)(void *env, intptr_t self, val_t *result);
    void (*incp)(void *env, intptr_t self, val_t *result);
    void (*decp)(void *env, intptr_t self, val_t *result);
    void (*mul)(void *env, intptr_t self, val_t *av, val_t *result);
    void (*div)(void *env, intptr_t self, val_t *av, val_t *result);
    void (*mod)(void *env, intptr_t self, val_t *av, val_t *result);
    void (*add)(void *env, intptr_t self, val_t *av, val_t *result);
    void (*sub)(void *env, intptr_t self, val_t *av, val_t *result);
    void (*and)(void *env, intptr_t self, val_t *av, val_t *result);
    void (*or) (void *env, intptr_t self, val_t *av, val_t *result);
    void (*xor)(void *env, intptr_t self, val_t *av, val_t *result);
    void (*lshift)(void *env, intptr_t self, val_t *av, val_t *result);
    void (*rshift)(void *env, intptr_t self, val_t *av, val_t *result);
    void (*prop)(void *env, intptr_t self, val_t *av, val_t *result);
    void (*elem)(void *env, intptr_t self, val_t *av, val_t *result);
    void (*set)(void *env, intptr_t self, val_t *av, val_t *result);
    val_t *(*prop_ref)(void *env, intptr_t self, val_t *av);
    val_t *(*elem_ref)(void *env, intptr_t self, val_t *av);
} val_foreign_op_t;

typedef struct val_foreign_t {
    uint8_t magic;
    uint8_t age;
    uint8_t reserved[2];
    intptr_t  data;
    const val_foreign_op_t *op;
} val_foreign_t;

static inline
int val_type(val_t *v) {
    int type = (*v) >> 48;
    if ((type & 0x7ff0) != 0x7ff0) {
        return 0;
    } else {
        return type & 0xf;
    }
}

static inline double val_2_double(val_t *v) {
    return ((valnum_t*)v)->d;
}

static inline int val_2_integer(val_t *v) {
    return (int) (((valnum_t*)v)->d);
}

static inline intptr_t val_2_intptr(val_t *v) {
    return (intptr_t)(*v & VAL_MASK);
}

static inline void val_2_reference(val_t *v, uint8_t *id, uint8_t *generation) {
    uint8_t *p = (uint8_t *)v;
#if SYS_BYTE_ORDER == LE
    *id = p[1]; *generation = p[0];
#else
    *id = p[6]; *generation = p[7];
#endif
}

#define double_2_val(d) (((valnum_t*)&(d))->v)

static inline int val_is_infinity(val_t *v) {
    return (*v & NUM_MASK) == TAG_INFINITE;
}

static inline int val_is_number(val_t *v) {
    return (*v & TAG_INFINITE) != TAG_INFINITE;
}

static inline int val_is_inline_string(val_t *v) {
    return (*v & TAG_MASK) == TAG_STRING_I;
}

static inline int val_is_heap_string(val_t *v) {
    return (*v & TAG_MASK) == TAG_STRING_H;
}

static inline int val_is_foreign_string(val_t *v) {
    return (*v & TAG_MASK) == TAG_STRING_F;
}

static inline int val_is_string(val_t *v) {
    return val_is_heap_string(v) || val_is_foreign_string(v) || val_is_inline_string(v);
}

static inline int val_is_boolean(val_t *v) {
    return (*v & TAG_MASK) == TAG_BOOLEAN;
}

static inline int val_is_script(val_t *v) {
    return (*v & TAG_MASK) == TAG_FUNC_SCRIPT;
}

static inline int val_is_native(val_t *v) {
    return (*v & TAG_MASK) == TAG_FUNC_NATIVE;
}

static inline int val_is_function(val_t *v) {
    return val_is_script(v) || val_is_native(v);
}

static inline int val_is_undefined(val_t *v) {
    return (*v & TAG_MASK) == TAG_UNDEFINED;
}

static inline int val_is_nan(val_t *v) {
    return (*v & TAG_MASK) == TAG_NAN;
}

static inline int val_is_array(val_t *v) {
    return (*v & TAG_MASK) == TAG_ARRAY;
}

static inline int val_is_buffer(val_t *v) {
    return (*v & TAG_MASK) == TAG_BUFFER;
}

static inline int val_is_object(val_t *v) {
    return (*v & TAG_MASK) == TAG_OBJECT;
}

static inline int val_is_foreign(val_t *v) {
    return (*v & TAG_MASK) == TAG_FOREIGN;
}

static inline int val_is_reference(val_t *v) {
    return (*v & TAG_MASK) == TAG_REFERENCE;
}

static inline const char *val_2_cstring(val_t *v) {
    uint64_t t = *v & TAG_MASK;

    if (t == TAG_STRING_I) {
        return ((const char *)v) + 4;
    } else
    if (t == TAG_STRING_F) {
        return (const char *) val_2_intptr(v);
    } else
    if (t == TAG_STRING_H) {
        return (const char *) (val_2_intptr(v) + 4);
    } else {
        return NULL;
    }
}

static inline val_t val_mk_number(double d) {
    return double_2_val(d);
}

static inline val_t val_mk_foreign_string(intptr_t s) {
    return TAG_STRING_F | s;
}

static inline val_t val_mk_heap_string(intptr_t s) {
    return TAG_STRING_H | s;
}

static inline val_t val_mk_boolean(int v) {
    return TAG_BOOLEAN | (!!v);
}

static inline val_t val_mk_script(intptr_t s) {
    return TAG_FUNC_SCRIPT | s;
}

static inline val_t val_mk_native(intptr_t n) {
    return TAG_FUNC_NATIVE | n;
}

static inline val_t val_mk_nan(void) {
    return TAG_NAN;
}

static inline val_t val_mk_undefined(void) {
    return TAG_UNDEFINED;
}

static inline val_t val_mk_array(void *ptr) {
    return TAG_ARRAY | (intptr_t) ptr;
}

static inline val_t val_mk_buffer(void *ptr) {
    return TAG_BUFFER | (intptr_t) ptr;
}

static inline val_t val_mk_foreign(intptr_t f) {
    return TAG_FOREIGN | f;
}

static inline void val_set_boolean(val_t *p, int b) {
    *((uint64_t *)p) = TAG_BOOLEAN | !!b;
}

static inline void val_set_number(val_t *p, double d) {
    *((double *)p) = d;
}

static inline void val_set_foreign_string(val_t *p, intptr_t s) {
    *((uint64_t *)p) = TAG_STRING_F | s;
}

static inline void val_set_heap_string(val_t *p, intptr_t s) {
    *((uint64_t *)p) = TAG_STRING_H | s;
}

static inline void val_set_inner_string(val_t *p, char c) {
    *((uint64_t *)p) = TAG_STRING_I;
    *(((char *)p) + 4) = c;
}

static inline void val_set_script(val_t *p, intptr_t s) {
    *((uint64_t *)p) = TAG_FUNC_SCRIPT | s;
}

static inline void val_set_native(val_t *p, intptr_t f) {
    *((uint64_t *)p) = TAG_FUNC_NATIVE | f;
}

static inline void val_set_nan(val_t *p) {
    *((uint64_t *)p) = TAG_NAN;
}

static inline void val_set_undefined(val_t *p) {
    *((uint64_t *)p) = TAG_UNDEFINED;
}

static inline void val_set_array(val_t *p, intptr_t a) {
    *((uint64_t *)p) = TAG_ARRAY | a;
}

static inline void val_set_buffer(val_t *p, void *b) {
    *((uint64_t *)p) = TAG_BUFFER | (intptr_t)b;
}

static inline void val_set_object(val_t *p, intptr_t d) {
    *((uint64_t *)p) = TAG_OBJECT | d;
}

static inline void val_set_foreign(val_t *p, intptr_t f) {
    *((uint64_t *)p) = TAG_FOREIGN | f;
}

static inline void val_set_reference(val_t *p, uint8_t id, uint8_t generation) {
    *((uint64_t *)p) = TAG_REFERENCE | id * 256 | generation;
}

typedef void (*val_op_t)(void *env, val_t *oprand1, val_t *oprand2, val_t *result);
typedef void (*val_op_unary_t)(void *env, val_t *oprand, val_t *result);

int  val_is_true(val_t *v);
int  val_is_equal(val_t *a, val_t *b);
int  val_compare(val_t *a, val_t *b);
int  val_is_ge(val_t *a, val_t *b);
int  val_is_gt(val_t *a, val_t *b);
int  val_is_le(val_t *a, val_t *b);
int  val_is_lt(val_t *a, val_t *b);

void val_op_prop(void *env, val_t *v, val_t *key, val_t *prop);
void val_op_elem(void *env, val_t *v, val_t *key, val_t *elem);
val_t *val_prop_ref(void *env, val_t *v, val_t *name);
val_t *val_elem_ref(void *env, val_t *v, val_t *id);

void val_op_neg(void *env, val_t *oprand, val_t *res);
void val_op_not(void *env, val_t *oprand, val_t *res);

void val_op_inc (void *env, val_t *self, val_t *res);
void val_op_incp(void *env, val_t *self, val_t *res);
void val_op_dec (void *env, val_t *self, val_t *res);
void val_op_decp(void *env, val_t *self, val_t *res);

void val_op_mul(void *env, val_t *a, val_t *b, val_t *r);
void val_op_div(void *env, val_t *a, val_t *b, val_t *r);
void val_op_mod(void *env, val_t *a, val_t *b, val_t *r);
void val_op_add(void *env, val_t *a, val_t *b, val_t *r);
void val_op_sub(void *env, val_t *a, val_t *b, val_t *r);
void val_op_and(void *env, val_t *a, val_t *b, val_t *r);
void val_op_or (void *env, val_t *a, val_t *b, val_t *r);
void val_op_xor(void *env, val_t *a, val_t *b, val_t *r);
void val_op_lshift(void *env, val_t *a, val_t *b, val_t *r);
void val_op_rshift(void *env, val_t *a, val_t *b, val_t *r);
void val_op_set(void *env, val_t *a, val_t *b, val_t *r);

val_t val_create(void *env, const val_foreign_op_t *op, intptr_t data);
int val_foreign_create(void *env, const val_foreign_op_t *op, intptr_t data, val_t *foreign);

static inline
int foreign_mem_space(val_foreign_t *foreign) {
    (void) foreign;
    return SIZE_ALIGN(sizeof(val_foreign_t));
}

#endif /* __LANG_VALUE_INC__ */

