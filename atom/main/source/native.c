#include "sal.h"
#include "panda.h"
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

static const native_t native_entry[] = {
    {"print", print},
    {"on",    native_led_on},
    {"off",   native_led_off},
};

int native_init(env_t *env)
{
    return env_native_add(env, 3, native_entry);
}

