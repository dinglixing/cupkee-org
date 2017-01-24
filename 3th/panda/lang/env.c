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

#include "env.h"
#include "gc.h"
#include "type_object.h"
#include "type_string.h"
#include "type_array.h"
#include "type_function.h"

#define VACATED     (-1)
#define FRAME_SIZE  (sizeof(frame_t) / sizeof(val_t))

typedef struct frame_t {
    int fp;
    int sp;
    intptr_t pc;
    intptr_t scope;
} frame_t;

static inline
int env_is_valid_ptr(env_t *env, void *p) {
    return heap_is_owned(env->heap, p);
}

static inline
void env_heap_setup(env_t *env, void *heap_ptr, int heap_size) {
    int half_size = heap_size / 2;
    heap_init(&env->heap_top, heap_ptr, half_size);
    heap_init(&env->heap_bot, heap_ptr + half_size, half_size);
    env->heap = &env->heap_top;
}

static void env_heap_gc_init(env_t *env)
{
    heap_t  *heap = env_heap_get_free(env);
    val_t   *sb;
    int fp, sp, ss;

    heap_reset(heap);

    if (env->ref_num && env->ref_ent) {
        gc_copy_vals(heap, env->ref_num, env->ref_ent);
    }

    env->scope = gc_copy_scope(heap, env->scope);

    fp = env->fp, sp = env->sp, ss = env->ss;
    sb = env->sb;
    while (1) {
        if (fp == ss) {
            gc_copy_vals(heap, fp - sp, sb + sp);
            break;
        } else {
            frame_t *frame = (frame_t *)(sb + fp);

            gc_copy_vals(heap, fp - sp, sb + sp);
            frame->scope = (intptr_t)gc_copy_scope(heap, (scope_t *)frame->scope);

            fp = frame->fp;
            sp = frame->sp;
        }
    }
}

static uint32_t hash_pjw(const void *key)
{
    const char *ptr = key;
    uint32_t val = 0;

    while (*ptr) {
        uint32_t tmp;

        val = (val << 4) + *ptr;
        tmp = val & 0xf0000000;
        if (tmp) {
            val = (val ^ (tmp >> 24)) ^ tmp;
        }

        ptr++;
    }

    return val;
}

static inline uint32_t htbl_key(uint32_t size, uint32_t hash, int i) {
    return (hash + i * (hash * 2 + 1)) % size;
}

static inline char *env_symbal_buf_alloc(env_t *env, int size)
{
    if (env->symbal_buf_used + size < env->symbal_buf_end) {
        char *p = env->symbal_buf + env->symbal_buf_used;
        env->symbal_buf_used += size;
        return p;
    }
    return NULL;
}

static char *env_symbal_put(env_t *env, const char *str)
{
    int size = strlen(str) + 1;
    char *sym = env_symbal_buf_alloc(env, size);

    if (sym) {
        memcpy(sym, str, size);
    }
    return sym;
}

static int env_symbal_lookup(env_t *env, const char *symbal, char **res)
{
    uint32_t size, pos, i, hash;
    intptr_t *tbl = env->symbal_tbl;


    if (env->symbal_tbl_hold == 0) {
        return 0;
    }

    size = env->symbal_tbl_size;
    hash = hash_pjw(symbal);

    for (i = 0; i < size; i++) {
        pos = htbl_key(size, hash, i);

        if (tbl[pos] == 0) {
            break;
        }
        if ((intptr_t)symbal == tbl[pos] || !strcmp(symbal, (char*)tbl[pos])) {
            if (res) *res = (char *)tbl[pos];
            return 1;
        }
    }

    return 0;
}

intptr_t env_symbal_insert(env_t *env, const char *symbal, int alloc)
{
    uint32_t size, pos, i, hash;
    intptr_t *tbl = env->symbal_tbl;
    char *p;

    if (env_symbal_lookup(env, symbal, &p) == 1) {
        return (intptr_t)p;
    }

    size = env->symbal_tbl_size;
    if (env->symbal_tbl_hold >= size) {
        env_set_error(env, ERR_ResourceOutLimit);
        return 0;
    }

    hash = hash_pjw(symbal);
    for (i = 0; i < size; i++) {
        pos = htbl_key(size, hash, i);

        if (tbl[pos] == 0 || tbl[pos] == VACATED) {
            p = alloc ? env_symbal_put(env, symbal) : (char *)symbal;
            tbl[pos] = (intptr_t)p;
            env->symbal_tbl_hold++;
            return (intptr_t)p;
        }
    }

    return 0;
}

intptr_t env_symbal_get(env_t *env, const char *name) {
    char *p;
    if (1 == env_symbal_lookup(env, name, &p)) {
        return (intptr_t)p;
    }
    return 0;
}

int env_exe_memery_distribute(int size, int *num_max, int *str_max, int *fn_max, int *code_max)
{
    int code_space;
    int fent_space;
    int num_space;
    int str_space;

    if (code_max) {
        // half of memory as code space
        code_space = SIZE_ALIGN_8(size / 2);
        *code_max = code_space;
        size -= code_space;
    } else {
        code_space = 0;
    }

    if (str_max) {
        // 7/16 of memory as function entry space
        str_space = size * 7 / 8;
        *str_max = str_space / (sizeof(intptr_t) * 2 + DEF_STRING_SIZE);
        size -= str_space;
    } else {
        str_space = 0;
    }

    if (num_max) {
        // 1/32 of memory as number
        num_space = SIZE_ALIGN_8(size / 2);
        *num_max = num_space / sizeof(double);
        size -= num_space;
    } else {
        num_space = 0;
    }

    if (fn_max) {
        // 1/32 of memory as function entry space
        fent_space = SIZE_ALIGN_8(size);
        *fn_max = fent_space / sizeof(intptr_t);
    } else {
        fent_space = 0;
    }

#if 0
    printf("num space: %d\n", num_space);
    printf("str space: %d\n", str_space);
    printf("code space: %d\n", code_space);
    printf("fent space: %d\n", fent_space);
#endif

    return 0;
}

int env_init(env_t *env, void *mem_ptr, int mem_size,
             void *heap_ptr, int heap_size, val_t *stack_ptr, int stack_size,
             int number_max, int string_max, int func_max,
             int code_max, int interactive)
{
    int mem_offset;
    int exe_size, symbal_tbl_size;

    env->error = 0;

    // stack init
    if (!stack_ptr) {
        // alloc memory for stack
        stack_ptr = ADDR_ALIGN_8(mem_ptr);
        mem_offset = (intptr_t)stack_ptr - (intptr_t)mem_ptr;
        mem_offset += sizeof(val_t) * stack_size;
        // TODO: create a barrier here
    } else {
        mem_offset = 0;
    }
    env->sb = stack_ptr;
    env->ss = stack_size;
    env->sp = stack_size;
    env->fp = stack_size;

    // heap init
    if (!heap_ptr) {
        // alloc memory for heap
        heap_size = SIZE_ALIGN_16(heap_size);
        heap_ptr = ADDR_ALIGN_16(mem_ptr + mem_offset);
        mem_offset += (intptr_t)heap_ptr - (intptr_t)(mem_ptr + mem_offset);
        mem_offset += heap_size;
    } else {
        if (heap_size % 16 || ((intptr_t)heap_ptr & 0xf)) {
            // heap should be align 16!
            return -1;
        }
    }
    env_heap_setup(env, heap_ptr, heap_size);

    // main_var_map init
    if (interactive) {
        env->main_var_map = (intptr_t *)(mem_ptr + mem_offset);
        mem_offset += sizeof(intptr_t) * INTERACTIVE_VAR_MAX;
    } else {
        env->main_var_map = NULL;
    }
    env->main_var_num = 0;

    // native init
    env->native_num = 0;
    env->native_ent = NULL;

    // reference init
    env->ref_num = 0;
    env->ref_ent = NULL;

    // static memory init
    exe_size = executable_init(&env->exe, mem_ptr + mem_offset, mem_size - mem_offset,
                    number_max, string_max, func_max, code_max);
    if (exe_size < 0) {
        return -1;
    }
    mem_offset += exe_size;

    symbal_tbl_size = string_max * sizeof(intptr_t);
    if (mem_offset + symbal_tbl_size > mem_size) {
        return -1;
    }
    env->symbal_tbl = (intptr_t *) (mem_ptr + mem_offset);
    env->symbal_tbl_size = string_max;
    env->symbal_tbl_hold = 0;
    memset(mem_ptr + mem_offset, 0, symbal_tbl_size);
    mem_offset += symbal_tbl_size;

    env->symbal_buf = mem_ptr + mem_offset;
    env->symbal_buf_end = mem_size - mem_offset;
    env->symbal_buf_used = 0;

#if 0
    printf("memory total: %d\n", mem_size);
    printf("stack: %d\n", mem_offset - exe_size - heap_size);
    printf("heap:  %d\n", heap_size);
    printf("exe:   %d\n", exe_size);
    printf("left:  %d\n", mem_size - mem_offset);
#endif

    if (env_is_interactive(env)) {
        env->scope = env_scope_create(env, NULL, NULL, 0, NULL);
    } else {
        env->scope = NULL;
    }

    // Initialise callbacks
    env->gc_callback = NULL;

    if (0 != objects_env_init(env)) {
        return -1;
    } else {
        return 0;
    }
}

int env_deinit(env_t *env)
{
    (void) env;
    return 0;
}

scope_t *env_scope_create(env_t *env, scope_t *super, uint8_t *entry, int ac, val_t *av)
{
    scope_t *scope;
    val_t   *buf;
    int vn, an, vc;
    int i, d;

    if (entry) {
        vn = executable_func_get_var_cnt(entry);
        an = executable_func_get_arg_cnt(entry);
    } else {
        vn = INTERACTIVE_VAR_MAX;
        an = 0;
    }

    d = ac - an;
    vc = vn + (d > 0 ? d : 0);
    scope = (scope_t *) env_heap_alloc(env, sizeof(scope_t) + sizeof(val_t) * vc);
    if (!scope) {
        env_set_error(env, ERR_NotEnoughMemory);
        return NULL;
    }
    buf = (val_t *) (scope + 1);

    if (d < 0) {
        for (i = 0; i < ac; i++) {
            buf[i] = av[i];
        }
    } else {
        for (i = 0; i < an; i++) {
            buf[i] = av[i];
        }
    }
    for (; i < vn; i++) {
        val_set_undefined(buf + i);
    }

    for (i = 0; i < d; i++) {
        buf[vn + i] = av[an + i];
    }

    scope->magic = MAGIC_SCOPE;
    scope->age = 0;
    scope->num = vc;
    scope->nao = vn;
    scope->super = super;
    scope->var_buf = buf;

    return scope;
}

const uint8_t *env_frame_setup(env_t *env, const uint8_t *pc, val_t *fv, int ac, val_t *av)
{
    function_t *fn = (function_t *)val_2_intptr(fv);
    scope_t *scope;
    frame_t *frame;
    int fp;

    // empty function
    if (function_size(fn) == 0) {
        env->sp += ac + 1; // release arguments & fobj in stack
        *env_stack_push(env) = val_mk_undefined();
        return pc;
    }

    if (NULL == (scope = env_scope_create(env, fn->super, fn->entry, ac, av))) {
        // error had be set in
        return NULL;
    }

    if (!env_is_valid_ptr(env, fn)) {
        // GC happend? super should be update
        fn = (function_t *)val_2_intptr(fv); // fv had update, by gc
        scope->super = fn->super;
    }

    //skip arguments & function
    env->sp += ac + 1;
    fp = env->sp - FRAME_SIZE;
    if (fp < 0 || fp < function_stack_high(fn)) {
        env->error = ERR_StackOverflow;
        return NULL;
    }

    //printf ("############  resave sp: %d, fp: %d\n", env->sp, env->fp);
    frame = (frame_t *)(env->sb + fp);
    frame->fp = env->fp;
    frame->sp = env->sp;
    frame->pc = (intptr_t) pc;
    frame->scope = (intptr_t) env->scope;

    env->fp = fp;
    env->sp = fp;
    env->scope = scope;

    return function_code(fn);
}

void env_frame_restore(env_t *env, const uint8_t **pc, scope_t **scope)
{
    if (env->fp != env->ss) {
        frame_t *frame = (frame_t *)(env->sb + env->fp);

        env->sp = frame->sp;
        env->fp = frame->fp;
        *pc = (uint8_t *) frame->pc;
        *scope = (scope_t *) frame->scope;
    } else {
        *pc = NULL;
        *scope = NULL;
    }
}

void env_native_call(env_t *env, val_t *fv, int ac, val_t *av)
{
    function_native_t fn = (function_native_t) val_2_intptr(fv);
    int sp;

    sp = env->sp + ac; // skip arguments & keep return value in stack

    *(env->sb + sp) = fn(env, ac, av);

    env->sp = sp;
}

const uint8_t *env_func_entry_setup(env_t *env, uint8_t *entry, int ac, val_t *av)
{
    // main scope already created, in interactive mode
    if (!env_is_interactive(env)) {
        env->scope = env_scope_create(env, NULL, entry, ac, av);
    }

    return executable_func_get_code(entry);
}

const uint8_t *env_main_entry_setup(env_t *env, int ac, val_t *av)
{
    uint8_t *entry = env_get_main_entry(env);

    // main scope already created, in interactive mode
    if (!env_is_interactive(env)) {
        env->scope = env_scope_create(env, NULL, entry, ac, av);
    }

    return executable_func_get_code(entry);
}

void *env_heap_alloc(env_t *env, int size)
{
    void *ptr = heap_alloc(env->heap, size);

    if (!ptr) {
        env_heap_gc(env, size);
        return heap_alloc(env->heap, size);
    }

    return ptr;
}

void env_heap_gc(env_t *env, int flags)
{
    heap_t *free_heap = env_heap_get_free(env);

    (void) flags;

    env_heap_gc_init(env);
    gc_scan(free_heap);

    if (env->gc_callback) {
        env->gc_callback();
    }

    env->heap = free_heap;
}

int env_number_find_add(env_t *env, double n)
{
    return executable_number_find_add(&env->exe, n);
}

int env_string_find_add(env_t *env, intptr_t s)
{
    return executable_string_find_add(&env->exe, s);
}

int env_native_find(env_t *env, intptr_t sym_id)
{
    int i;

    for (i = 0; i < env->native_num; i++) {
        if (sym_id == (intptr_t) env->native_ent[i].name) {
            return i;
        }
    }

    return -1;
}

int env_native_set(env_t *env, const native_t *ent, int num)
{
    int i;

    for (i = 0; i < num; i++) {
        if (0 == env_symbal_add_static(env, ent[i].name)) {
            return -1;
        }
    }

    env->native_num = num;
    env->native_ent = ent;
    return 0;
}

int env_reference_set(env_t *env, val_t *ref, int num)
{
    if (env->ref_ent == NULL) {
        env->ref_ent = ref;
        env->ref_num = num;
        return 0;
    }

    return -1;
}

int env_callback_set(env_t *env, void (*cb)(void))
{
    env->gc_callback = cb;
    return 0;
}

void env_symbal_foreach(env_t *env, int (*cb)(const char *, void *), void *param)
{
    int i;

    if (!cb) {
        return;
    }

    // scan main variable symbal
    for (i = 0; i < env->main_var_num; i++) {
        const char *sym = (const char *)(env->main_var_map[i]);
        if (cb(sym, param)) {
            return;
        }
    }

    // scan native symbal
    for (i = 0; i < env->native_num; i++) {
        const char *sym = env->native_ent[i].name;
        if (cb(sym, param)) {
            return;
        }
    }
}

