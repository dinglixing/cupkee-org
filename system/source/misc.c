#include "util.h"

#include "console.h"
#include "misc.h"

#define VARIABLE_REF_MAX    (16)

static val_t reference_vals[VARIABLE_REF_MAX];
void reference_init(env_t *env)
{
    int i;

    for (i = 0; i < VARIABLE_REF_MAX; i++) {
        val_set_undefined(&reference_vals[i]);
    }

    env_reference_set(env, reference_vals, VARIABLE_REF_MAX);
}

val_t *reference_create(val_t *v)
{
    int i;

    for (i = 0; i < VARIABLE_REF_MAX; i++) {
        val_t *r = reference_vals + i;

        if (val_is_undefined(r)) {
            *r = *v;
            return r;
        }
    }
    return NULL;
}

void reference_release(val_t *ref)
{
    if (ref) {
        int pos = ref - reference_vals;

        if (pos >= 0 && pos < VARIABLE_REF_MAX) {
            val_set_undefined(ref);
        }
    }
}

void print_error(int error)
{
    switch (error) {
    case ERR_NotEnoughMemory:   console_puts("Error: Not enought memory\r\n");      break;
    case ERR_NotImplemented:    console_puts("Error: Not implemented\r\n");         break;
    case ERR_StackOverflow:     console_puts("Error: Stack overflow\r\n");          break;
    case ERR_ResourceOutLimit:  console_puts("Error: Resource out of limit\r\n");   break;

    case ERR_InvalidToken:      console_puts("Error: Invalid Token\r\n");           break;
    case ERR_InvalidSyntax:     console_puts("Error: Invalid syntax\r\n");          break;
    case ERR_InvalidLeftValue:  console_puts("Error: Invalid left value\r\n");      break;
    case ERR_InvalidSementic:   console_puts("Error: Invalid Sementic\r\n");        break;

    case ERR_InvalidByteCode:   console_puts("Error: Invalid Byte code\r\n");       break;
    case ERR_InvalidInput:      console_puts("Error: Invalid input\r\n");           break;
    case ERR_InvalidCallor:     console_puts("Error: Invalid callor\r\n");          break;

    case ERR_SysError:          console_puts("Error: System error\r\n");            break;

    default: console_puts("Error: unknown error\r\n");
    }
}


static inline void print_number(val_t *v) {
    char buf[32];

    if (*v & 0xffff) {
        snprintf(buf, 32, "%f\r\n", val_2_double(v));
    } else {
        snprintf(buf, 32, "%lld\r\n", (int64_t)val_2_double(v));
    }
    hw_console_sync_puts(buf);
}

static inline void print_boolean(val_t *v) {
    hw_console_sync_puts(val_2_intptr(v) ? "true\r\n" : "false\r\n");
}

static inline void print_string(val_t *v) {
    hw_console_sync_puts("\"");
    hw_console_sync_puts(val_2_cstring(v));
    hw_console_sync_puts("\"\r\n");
}

void print_simple_value(val_t *v)
{
    if (val_is_number(v)) {
        print_number(v);
    } else
    if (val_is_boolean(v)) {
        print_boolean(v);
    } else
    if (val_is_string(v)) {
        print_string(v);
    } else
    if (val_is_undefined(v)) {
        hw_console_sync_puts("undefined\r\n");
    } else
    if (val_is_nan(v)) {
        hw_console_sync_puts("NaN\r\n");
    } else
    if (val_is_function(v)) {
        hw_console_sync_puts("<function>\r\n");
    } else
    if (val_is_object(v)) {
        hw_console_sync_puts("<object>\r\n");
    } else
    if (val_is_buffer(v)) {
        hw_console_sync_puts("<buffer>\r\n");
    } else
    if (val_is_array(v)) {
        hw_console_sync_puts("<array>\r\n");
    } else {
        hw_console_sync_puts("<object>\r\n");
    }
}

static void print_object_value(val_t *o)
{
    object_t *obj = (object_t *) val_2_intptr(o);
    object_iter_t it;
    const char *k;
    val_t *v;

    hw_console_sync_puts("{");

    _object_iter_init(&it, obj);
    if (_object_iter_next(&it, &k, &v)) {
        hw_console_sync_puts("\r\n");
        do {
            hw_console_sync_puts("  ");
            hw_console_sync_puts(k);
            hw_console_sync_puts(": ");
            print_simple_value(v);
        }while(_object_iter_next(&it, &k, &v));
    }
    hw_console_sync_puts("}\r\n");
}

static void print_array_value(val_t *v)
{
    array_t *array = (array_t *) val_2_intptr(v);
    int i, max;

    max = array_len(array);

    if (max == 0) {
        hw_console_sync_puts("[]\r\n");
        return;
    }
    hw_console_sync_puts("[\r\n");
    for (i = 0; i < max; i++) {
        char buf[16];
        snprintf(buf, 16, "  [%2d]:", i);
        hw_console_sync_puts(buf);

        print_simple_value(_array_elem(array, i));
    }
    hw_console_sync_puts("]\r\n");
}

static void print_buffer_value(val_t *v)
{
    int i, len = buffer_size(v);
    uint8_t *ptr = buffer_addr(v);
    char buf[16];

    snprintf(buf, 16, "<Buffer[%d]:", len);
    hw_console_sync_puts(buf);
    for (i = 0; i < len && i < 8; i++) {
        snprintf(buf, 16, " %.2x", ptr[i]);
        hw_console_sync_puts(buf);
    }

    if (i < len) {
        hw_console_sync_puts(" ...>\r\n");
    } else {
        hw_console_sync_puts(">\r\n");
    }
}

void print_value(val_t *v)
{
    if (val_is_array(v)) {
        print_array_value(v);
    } else
    if (val_is_object(v)) {
        print_object_value(v);
    } else
    if (val_is_buffer(v)) {
        print_buffer_value(v);
    } else {
        print_simple_value(v);
    }
}

val_t native_sysinfos(env_t *env, int ac, val_t *av)
{
    hw_info_t hw;
    char buf[80];

    (void) ac;
    (void) av;

    hw_info_get(&hw);

    snprintf(buf, 80, "FREQ: %uM\r\n", hw.sys_freq / 1000000);
    console_puts(buf);
    snprintf(buf, 80, "RAM: %dK\r\n", hw.ram_sz / 1024);
    console_puts(buf);
    snprintf(buf, 80, "ROM: %dK\r\n\r\n", hw.rom_sz / 1024);
    console_puts(buf);
    snprintf(buf, 80, "symbal: %d/%d, ", env->symbal_tbl_hold, env->symbal_tbl_size);
    console_puts(buf);
    snprintf(buf, 80, "string: %d/%d, ", env->exe.string_num, env->exe.string_max);
    console_puts(buf);
    snprintf(buf, 80, "number: %d/%d, ", env->exe.number_num, env->exe.number_max);
    console_puts(buf);
    snprintf(buf, 80, "function: %d/%d, ", env->exe.func_num, env->exe.func_max);
    console_puts(buf);
    snprintf(buf, 80, "variable: %d\r\n", env->main_var_num);
    console_puts(buf);

    return val_mk_undefined();
}

val_t native_systicks(env_t *env, int ac, val_t *av)
{
    (void) env;
    (void) ac;
    (void) av;

    return val_mk_number(system_ticks_count);
}

val_t native_print(env_t *env, int ac, val_t *av)
{
    int i;
    (void) env;

    for (i = 0; i < ac; i++) {
        print_value(av+i);
    }

    return val_mk_undefined();
}

val_t native_scripts(env_t *env, int ac, val_t *av)
{
    const char *script = NULL;
    int which = -1, del = 0, n = 0;

    (void) env;

    if (ac > 0 && val_is_number(av)) {
        which = val_2_double(av);
        ac--;
        av++;
    }

    if (ac && val_is_string(av)) {
        if (strcmp("delete", val_2_cstring(av)) == 0) {
            del = 1;
        }
    }

    if (del) {
        int err;
        if (which < 0) {
            err = hw_scripts_erase();
        } else {
            err = hw_scripts_remove(which);
        }
        return val_mk_boolean(err == 0 ? val_mk_boolean(1) : val_mk_boolean(0));
    }

    while (NULL != (script = hw_scripts_load(script))) {
        if (which < 0 || which == n) {
            char id[16];
            snprintf(id, 16, "[%.4d] ", n);
            console_puts(id);
            console_puts(script);
        }
        n++;
    }

    return val_mk_number(n);
}


