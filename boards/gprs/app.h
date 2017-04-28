#ifndef __APP_INC__
#define __APP_INC__

#include "cupkee.h"

enum {
    EVENT_COMMAND = EVENT_USER,
    EVENT_GPRS
};

enum {
    COMMAND_INPUT = 0,
};

enum {
    GPRS_EV_TOUT = 0,
    GPRS_EV_DONE,
    GPRS_EV_MSG,
    GPRS_EV_SREADY,
};

#include "gprs.h"

#endif /* __APP_INC__ */

