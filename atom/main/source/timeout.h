
#ifndef __TIMEOUT_INC__
#define __TIMEOUT_INC__

#include "panda.h"

void timeout_init(env_t *env);
void timeout_execute(env_t *env);

int timeout_regiseter(uint32_t wait, val_t *handle, int repeat);
int timeout_unregiseter(int tid, int repeat);

val_t native_set_timeout(env_t *env, int ac, val_t *av);
val_t native_set_interval(env_t *env, int ac, val_t *av);
val_t native_clear_timeout(env_t *env, int ac, val_t *av);
val_t native_clear_interval(env_t *env, int ac, val_t *av);

#endif /* __TIMEOUT_INC__ */

