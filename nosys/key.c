#include <bsp.h>
#include "key.h"

const hw_device_t *desc;
const hw_driver_t *driver;
int key = 0;

int key_enable(void)
{
    desc = hw_device_take("key", 0, &driver);

    if (desc) {
        driver->listen(desc->id, 0, DEVICE_EVENT_DATA);
        return driver->enable(desc->id, 0);
    } else {
        return 0;
    }
}

