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
    case ERR_SysError:          console_puts("Error: System error\r\n"); break;

    case ERR_NotEnoughMemory:   console_puts("Error: Not enought memory\r\n"); break;
    case ERR_NotImplemented:    console_puts("Error: Not implemented\r\n"); break;
    case ERR_StaticNumberOverrun: console_puts("Error: ..\r\n"); break;
    case ERR_StackOverflow:     console_puts("Error: Stack overflow\r\n"); break;
    case ERR_ResourceOutLimit:  console_puts("Error: Resource out of limit\r\n"); break;

    case ERR_InvalidToken:      console_puts("Error: Invalid Token\r\n"); break;
    case ERR_InvalidSyntax:     console_puts("Error: Invalid syntax\r\n"); break;
    case ERR_InvalidLeftValue:  console_puts("Error: Invalid left value\r\n"); break;
    case ERR_InvalidSementic:   console_puts("Error: Invalid Sementic\r\n"); break;

    case ERR_InvalidByteCode:   console_puts("Error: Invalid Byte code\r\n"); break;
    case ERR_InvalidInput:      console_puts("Error: Invalid input\r\n"); break;
    case ERR_InvalidCallor:     console_puts("Error: Invalid callor\r\n"); break;
    case ERR_NotDefinedId:      console_puts("Error: Not defined id\r\n"); break;
    case ERR_NotDefinedProp:    console_puts("Error: Not defined proprity\r\n"); break;
    case ERR_HasNoneElement:    console_puts("Error: Not defined element\r\n"); break;
    default: console_puts("Error: unknown error\r\n");
    }
}

void print_value(val_t *v)
{
    char buf[32];

    if (val_is_number(v)) {
        if (*v & 0xffff) {
            snprintf(buf, 32, "%f\r\n", val_2_double(v));
        } else {
            snprintf(buf, 32, "%lld\r\n", (int64_t)val_2_double(v));
        }
        console_puts(buf);
    } else
    if (val_is_boolean(v)) {
        console_puts(val_2_intptr(v) ? "true\r\n" : "false\r\n");
    } else
    if (val_is_string(v)) {
        console_puts("\"");
        console_puts(val_2_cstring(v));
        console_puts("\"\r\n");
    } else
    if (val_is_undefined(v)) {
        console_puts("undefined\r\n");
    } else
    if (val_is_nan(v)) {
        console_puts("NaN\r\n");
    } else
    if (val_is_function(v)) {
        console_puts("<function>\r\n");
    } else
    if (val_is_dictionary(v)) {
        console_puts("<dictionary>\r\n");
    } else
    if (val_is_array(v)) {
        console_puts("<array>\r\n");
    } else {
        console_puts("<object>\r\n");
    }
}

val_t native_led(env_t *env, int ac, val_t *av)
{
    (void) env;

    if (ac > 0 && val_is_string(av)) {
        const char *req = val_2_cstring(av);

        if (!strcmp(req, "on")) {
            hal_led_on();
            return val_mk_undefined();
        } else
        if (!strcmp(req, "off")){
            hal_led_off();
            return val_mk_undefined();
        }
    }
    hal_led_toggle();
    return val_mk_undefined();
}

val_t native_sysinfos(env_t *env, int ac, val_t *av)
{
    hal_info_t hal;
    char buf[80];

    (void) ac;
    (void) av;

    hal_info_get(&hal);

    snprintf(buf, 80, "FREQ: %uM\r\n", hal.sys_freq / 1000000);
    console_puts(buf);
    snprintf(buf, 80, "RAM: %dK\r\n", hal.ram_sz / 1024);
    console_puts(buf);
    snprintf(buf, 80, "ROM: %dK\r\n\r\n", hal.rom_sz / 1024);
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
        if (i > 0) {
            console_puts(" ");
        }
        print_value(av+i);
    }
    console_puts("\n");

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
            err = hal_scripts_erase();
        } else {
            err = hal_scripts_remove(which);
        }
        return val_mk_boolean(err == 0 ? val_mk_boolean(1) : val_mk_boolean(0));
    }

    while (NULL != (script = hal_scripts_load(script))) {
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


