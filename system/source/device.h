
#ifndef __DEVICE_INC__
#define __DEVICE_INC__

#include <cupkee.h>

#define DEV_FL_ENBALE 1
#define DEV_FL_ERROR  2

struct cupkee_driver_t;
typedef struct cupkee_device_t {
    uint16_t magic;
    uint16_t flags;
    void     *data;
    const struct cupkee_driver_t *driver;
    struct cupkee_device_t *next;
} cupkee_device_t;

typedef struct device_config_handle_t {
    int (*setter)(cupkee_device_t *, env_t *, val_t *);
    val_t (*getter)(cupkee_device_t *, env_t *);
} device_config_handle_t;

typedef struct cupkee_driver_t {
    int (*init)     (cupkee_device_t *dev);
    int (*deinit)   (cupkee_device_t *dev);

    int (*enable)   (cupkee_device_t *dev);
    int (*disable)  (cupkee_device_t *dev);

    val_t (*read)   (cupkee_device_t *dev, int off);
    val_t (*write)  (cupkee_device_t *dev, val_t *data);

    int (*listen)   (cupkee_device_t *dev, val_t *event, val_t *callback);
    int (*ignore)   (cupkee_device_t *dev, val_t *event);

    void (*event_handle) (env_t *env, uint8_t which, uint8_t event);
    device_config_handle_t *(*config) (cupkee_device_t *dev, val_t *name);
} cupkee_driver_t;

void devices_setup(void);
void devices_event_proc(env_t *env, int event);

#endif /* __DEVICE_INC__ */

