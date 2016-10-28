
#ifndef __UTIL_INC__
#define __UTIL_INC__

#include <cupkee.h>

#include "rbuff.h"
#include "event.h"

void cupkee_do_callback(env_t *env, val_t *cb, uint8_t ac, val_t *av);
val_t cupkee_error(env_t *env, int code);

#endif /* __UTIL_INC__ */

