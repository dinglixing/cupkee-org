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
    case ERR_ResourceOutLimit:  console_puts("Error: Resource out of limited\r\n"); break;

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
    if (val_is_number(v)) {
        char buf[32];
        snprintf(buf, 32, "%f\r\n", val_2_double(v));
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
        char buf[32];
        snprintf(buf, 32, "function:%p\r\n", (void *)val_2_intptr(v));
        console_puts(buf);
    } else {
        console_puts("object\r\n");
    }
}

static val_t native_led_on(env_t *env, int ac, val_t *av)
{
    (void) env;
    (void) ac;
    (void) av;

    hal_led_on(HAL_LED_1);
    return val_mk_undefined();
}

static val_t native_led_off(env_t *env, int ac, val_t *av)
{
    (void) env;
    (void) ac;
    (void) av;

    hal_led_off(HAL_LED_1);
    return val_mk_undefined();
}

static val_t native_led_toggle(env_t *env, int ac, val_t *av)
{
    (void) env;
    (void) ac;
    (void) av;

    hal_led_toggle(HAL_LED_1);
    return val_mk_undefined();
}

static val_t native_systick(env_t *env, int ac, val_t *av)
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

static val_t native_script_show(env_t *env, int ac, val_t *av)
{
    const char *script;
    int ctx = 0, n = 0;

    (void) env;

    if (ac > 0) {
        if (val_is_string(av)) {
            return storage_script_append(val_2_cstring(av)) ?
                val_mk_boolean(0) : val_mk_boolean(1);
        }
        if (val_is_number(av)) {
            return storage_script_del(val_2_double(av)) ? 
                val_mk_boolean(0) : val_mk_boolean(1);
        }
    }

    while (NULL != (script = storage_script_next(&ctx))) {
        char id[16];
        snprintf(id, 16, "[%4d: %d]\r\n", n, ctx);
        console_puts(id);
        console_puts(script);
        console_puts("\r\n");
        n++;
    }

    return val_mk_number(n);
}

static const native_t native_entry[] = {
    {"systick",         native_systick},

    {"setTimeout",      native_set_timeout},
    {"setInterval",     native_set_interval},
    {"clearTimeout",    native_clear_timeout},
    {"clearInterval",   native_clear_interval},

    {"print",       print},

    {"script",          native_script_show},
    {"ledOn",      native_led_on},
    {"ledOff",     native_led_off},
    {"ledToggle",  native_led_toggle},
};

int native_init(env_t *env)
{
    return env_native_set(env, native_entry, sizeof(native_entry)/sizeof(native_t));
}

