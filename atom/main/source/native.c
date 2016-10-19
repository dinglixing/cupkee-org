#include "sal.h"
#include "shell.h"
#include "native.h"

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
        snprintf(buf, 32, "<function:%p>\r\n", (void *)val_2_intptr(v));
        console_puts(buf);
    } else
    if (val_is_dictionary(v)) {
        snprintf(buf, 32, "<object:%p>\r\n", (void *)val_2_intptr(v));
        console_puts(buf);
    } else
    if (val_is_array(v)) {
        snprintf(buf, 32, "<array:%p>\r\n", (void *)val_2_intptr(v));
        console_puts(buf);
    } else {
        console_puts("object\r\n");
    }
}

static val_t native_led(env_t *env, int ac, val_t *av)
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

static val_t native_sysinfos(env_t *env, int ac, val_t *av)
{
    hal_info_t hal;
    char buf[80];

    (void) ac;
    (void) av;

    hal_info_get(&hal);

    snprintf(buf, 80, "FREQ: %luM\r\n", hal.sys_freq / 1000000);
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

static val_t native_systicks(env_t *env, int ac, val_t *av)
{
    (void) env;
    (void) ac;
    (void) av;

    return val_mk_number(system_ticks_count);
}

static val_t print(env_t *env, int ac, val_t *av)
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

static int timeout_register(int ac, val_t *av, int repeat)
{
    val_t   *handle;
    uint32_t wait;

    if (ac > 0) {
        handle = av;
    } else {
        return -1;
    }

    if (ac > 1 && val_is_number(av + 1)) {
        wait = val_2_double(av + 1);
    } else {
        wait = 0;
    }

    return shell_timeout_regiseter(wait, handle, repeat);
}

static int timeout_unregister(int ac, val_t *av, int repeat)
{
    int tid;

    if (ac > 0) {
        if (val_is_number(av)) {
            tid = val_2_double(av);
        } else {
            return -1;
        }
    } else {
        // clear all
        tid = -1;
    }

    return shell_timeout_unregiseter(tid, repeat);
}

static val_t native_set_timeout(env_t *env, int ac, val_t *av)
{
    int tid = timeout_register(ac, av, 0);

    (void) env;

    return tid < 0 ? val_mk_boolean(0) : val_mk_number(tid);
}

static val_t native_set_interval(env_t *env, int ac, val_t *av)
{
    int tid = timeout_register(ac, av, 1);

    (void) env;

    return tid < 0 ? val_mk_boolean(0) : val_mk_number(tid);
}

static val_t native_clear_timeout(env_t *env, int ac, val_t *av)
{
    int n = timeout_unregister(ac, av, 0);

    (void) env;

    return n < 0 ? val_mk_boolean(0) : val_mk_number(n);
}

static val_t native_clear_interval(env_t *env, int ac, val_t *av)
{
    int n = timeout_unregister(ac, av, 1);

    (void) env;

    return n < 0 ? val_mk_boolean(0) : val_mk_number(n);
}

static val_t native_scripts(env_t *env, int ac, val_t *av)
{
    const char *script = NULL;
    int show = -1, del = 0, n = 0;

    (void) env;

    if (ac > 0) {
        if (val_is_number(av)) {
            show = val_2_double(av);
        } else {
            show = -1;
        }
    }

    if (ac > 1) {
        if (val_is_string(av + 1)) {
            if (strcmp("delete", val_2_cstring(av + 1))) {
                del = 1;
            }
        }
    }

    if (del) {
        if (n < 0) {
            return usr_scripts_erase() ?
                val_mk_boolean(0) : val_mk_boolean(1);
        } else {
            return usr_scripts_remove(val_2_double(av)) ?
                val_mk_boolean(0) : val_mk_boolean(1);
        }
    }

    while (NULL != (script = usr_scripts_next(script))) {
        if (show < 0 || show == n) {
            char id[16];
            snprintf(id, 16, "[%.4d]\r\n", n);
            console_puts(id);
            console_puts(script);
        }
        n++;
    }

    return val_mk_number(n);
}

static const native_t native_entry[] = {
    {"sysinfos",        native_sysinfos},
    {"systicks",        native_systicks},

    {"print",           print},
    {"led",             native_led},

    {"setTimeout",      native_set_timeout},
    {"setInterval",     native_set_interval},
    {"clearTimeout",    native_clear_timeout},
    {"clearInterval",   native_clear_interval},

    {"scripts",         native_scripts},
};

int native_init(env_t *env)
{
    return env_native_set(env, native_entry, sizeof(native_entry)/sizeof(native_t));
}

