#include "sal.h"
#include "panda.h"
#include "native.h"

void print_error(int error)
{
    switch (error) {
    case ERR_SysError:          sal_console_output("Error: System error\r\n"); break;

    case ERR_NotEnoughMemory:   sal_console_output("Error: Not enought memory\r\n"); break;
    case ERR_NotImplemented:    sal_console_output("Error: Not implemented\r\n"); break;
    case ERR_StaticNumberOverrun: sal_console_output("Error: ..\r\n"); break;
    case ERR_StackOverflow:     sal_console_output("Error: Stack overflow\r\n"); break;
    case ERR_ResourceOutLimit:  sal_console_output("Error: Resource out of limited\r\n"); break;

    case ERR_InvalidToken:      sal_console_output("Error: Invalid Token\r\n"); break;
    case ERR_InvalidSyntax:     sal_console_output("Error: Invalid syntax\r\n"); break;
    case ERR_InvalidLeftValue:  sal_console_output("Error: Invalid left value\r\n"); break;
    case ERR_InvalidSementic:   sal_console_output("Error: Invalid Sementic\r\n"); break;

    case ERR_InvalidByteCode:   sal_console_output("Error: Invalid Byte code\r\n"); break;
    case ERR_InvalidInput:      sal_console_output("Error: Invalid input\r\n"); break;
    case ERR_InvalidCallor:     sal_console_output("Error: Invalid callor\r\n"); break;
    case ERR_NotDefinedId:      sal_console_output("Error: Not defined id\r\n"); break;
    case ERR_NotDefinedProp:    sal_console_output("Error: Not defined proprity\r\n"); break;
    case ERR_HasNoneElement:    sal_console_output("Error: Not defined element\r\n"); break;
    default: sal_console_output("Error: unknown error\r\n");
    }
}

void print_value(val_t *v)
{
    if (val_is_number(v)) {
        char buf[32];
        snprintf(buf, 32, "%f\r\n", val_2_double(v));
        sal_console_output(buf);
    } else
    if (val_is_boolean(v)) {
        sal_console_output(val_2_intptr(v) ? "true\r\n" : "false\r\n");
    } else
    if (val_is_string(v)) {
        sal_console_output("\"");
        sal_console_output(val_2_cstring(v));
        sal_console_output("\"\r\n");
    } else
    if (val_is_undefined(v)) {
        sal_console_output("undefined\r\n");
    } else
    if (val_is_nan(v)) {
        sal_console_output("NaN\r\n");
    } else
    if (val_is_function(v)) {
        char buf[32];
        snprintf(buf, 32, "function:%p\r\n", (void *)val_2_intptr(v));
        sal_console_output(buf);
    } else {
        sal_console_output("object\r\n");
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

static val_t print(env_t *env, int ac, val_t *av)
{
    int i;
    (void) env;

    for (i = 0; i < ac; i++) {
        if (i > 0) {
            sal_console_output(" ");
        }
        print_value(av+i);
    }
    sal_console_output("\n");

    return val_mk_undefined();
}

static native_t native_entry[] = {
    {"print", print},
    {"on",    native_led_on},
    {"off",   native_led_off},
};

int native_init(env_t *env)
{
    return env_native_add(env, 3, native_entry);
}

