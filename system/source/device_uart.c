#include "device.h"

void device_uart_def(hw_config_t *conf)
{
    hw_config_uart_t *uart = (hw_config_uart_t *) conf;

    uart->baudrate = 9600;
    uart->data_bits = 8;
    uart->stop_bits = DEVICE_OPT_STOPBITS_1;
    uart->parity   = DEVICE_OPT_PARITY_NONE;
}

int device_uart_get(hw_config_t *conf, int which, val_t *val)
{
    hw_config_uart_t *uart = (hw_config_uart_t *) conf;

    switch (which) {
    case DEVICE_UART_CONF_BAUDRATE: val_set_number(val, uart->baudrate);  break;
    case DEVICE_UART_CONF_DATABITS: val_set_number(val, uart->data_bits); break;
    case DEVICE_UART_CONF_STOPBITS:
        device_get_option(val, uart->stop_bits, DEVICE_OPT_STOPBITS_MAX, device_opt_stopbits);
        break;
    case DEVICE_UART_CONF_PARITY:
        device_get_option(val, uart->parity, DEVICE_OPT_PARITY_MAX, device_opt_parity);
        break;
    default: break;
    }

    return CUPKEE_OK;
}

int device_uart_set(hw_config_t *conf, int which, val_t *val)
{
    hw_config_uart_t *uart = (hw_config_uart_t *) conf;

    switch (which) {
    case DEVICE_UART_CONF_BAUDRATE: return device_set_uint32(val, &uart->baudrate);  break;
    case DEVICE_UART_CONF_DATABITS: return device_set_uint8(val, &uart->data_bits); break;
    case DEVICE_UART_CONF_STOPBITS:
        return device_set_option(val, &uart->stop_bits, DEVICE_OPT_STOPBITS_MAX, device_opt_stopbits);
    case DEVICE_UART_CONF_PARITY:
        return device_set_option(val, &uart->parity, DEVICE_OPT_PARITY_MAX, device_opt_parity);
    default: break;
    }

    return CUPKEE_EINVAL;
}

