#ifndef __CUPKEE_DEVICE_INC__
#define __CUPKEE_DEVICE_INC__

typedef void (*handle_t)(void);
typedef struct device_t {
    uint8_t id;
    uint8_t instance;
    uint8_t state;
    hw_config_t config;
    handle_t handles[DEVICE_EVENT_MAX];
    const hw_driver_t *driver;
} device_t;

int cupkee_device_init(void);
void cupkee_device_poll(void);
void cupkee_device_sync(uint32_t systicks);
void cupkee_device_event_handle(uint8_t which, uint16_t code);

device_t *cupkee_device_alloc(int type, int instance);

int cupkee_device_enable(device_t *dev);
int cupkee_device_send(device_t *dev, int n, const void *data);
int cupkee_device_recv(device_t *dev, int n, void *buf);
int cupkee_device_set(device_t *dev, int n, uint32_t data);
int cupkee_device_get(device_t *dev, int n, uint32_t *data);

#endif /* __CUPKEE_DEVICE_INC__ */

