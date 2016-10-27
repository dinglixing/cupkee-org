
#ifndef __DEVICE_INC__
#define __DEVICE_INC__

#include <cupkee.h>

#define DEV_FL_ENBALE 1
#define DEV_FL_ERROR  2

struct cupkee_driver_t;
typedef struct cupkee_device_t {
    uint16_t magic;
    uint16_t flags;
    struct cupkee_driver_t *driver;
    struct cupkee_devict_t *next;
} cupkee_device_t;

typedef struct cupkee_driver_t {
    int (*init)     (cupkee_device_t *dev);
    int (*deinit)   (cupkee_device_t *dev);
    int (*config)   (cupkee_device_t *dev, env_t*env, val_t *k, val_t *v);
    int (*read)     (cupkee_device_t *dev);
    int (*write)    (cupkee_device_t *dev);
    int (*read_int)  (cupkee_device_t *dev);
    int (*read_uint) (cupkee_device_t *dev);
    int (*write_int)  (cupkee_device_t *dev);
    int (*write_uint) (cupkee_device_t *dev);
} cupkee_driver_t;

void device_setup(void);

#endif /* __DEVICE_INC__ */

