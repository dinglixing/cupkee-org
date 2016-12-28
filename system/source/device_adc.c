#include "device.h"

int device_adc_get(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_adc_t *adc = (hw_config_adc_t *) conf;

    switch (which) {
    case DEVICE_ADC_CONF_CHANNELS: device_get_sequence(env, val, adc->chn_num, adc->chn_seq);   break;
    case DEVICE_ADC_CONF_INTERVAL: val_set_number(val, adc->interval); break;
    default:                       return -CUPKEE_EINVAL;
    }

    return CUPKEE_OK;
}

int device_adc_set(env_t *env, hw_config_t *conf, int which, val_t *val)
{
    hw_config_adc_t *adc = (hw_config_adc_t *) conf;

    (void) env;

    switch (which) {
    case DEVICE_ADC_CONF_CHANNELS: return device_set_sequence(val, HW_CHN_MAX_ADC, &adc->chn_num, adc->chn_seq);
    case DEVICE_ADC_CONF_INTERVAL: return device_set_uint16(val, &adc->interval);
    default:                       return -CUPKEE_EINVAL;
    }
}

