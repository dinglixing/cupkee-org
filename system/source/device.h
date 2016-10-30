
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

typedef struct cupkee_driver_t {
    int (*init)     (cupkee_device_t *dev);
    int (*deinit)   (cupkee_device_t *dev);

    int (*enable)   (cupkee_device_t *dev);
    int (*disable)  (cupkee_device_t *dev);

    int (*listen)   (cupkee_device_t *dev, val_t *event, val_t *callback);
    int (*ignore)   (cupkee_device_t *dev, val_t *event);

    val_t (*config) (cupkee_device_t *dev, env_t *env, const char *name, val_t *v);
    val_t (*read)   (cupkee_device_t *dev);
    val_t (*write)  (cupkee_device_t *dev, val_t *data);
} cupkee_driver_t;

void device_setup(void);

#endif /* __DEVICE_INC__ */

