
#ifndef __UTIL_INC__
#define __UTIL_INC__

#include <cupkee.h>

#include "rbuff.h"
#include "event.h"

void cupkee_do_callback(env_t *env, val_t *cb, uint8_t ac, val_t *av);
void cupkee_do_callback_error(env_t *env, val_t *cb, int code);
val_t cupkee_error(env_t *env, int code);
int cupkee_id(val_t *in, int max, const char **names);

#endif /* __UTIL_INC__ */

