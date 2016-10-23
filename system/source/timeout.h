
#ifndef __TIMEOUT_INC__
#define __TIMEOUT_INC__

#include "event.h"

void timeout_init(env_t *env);
void timeout_execute(env_t *env);

int timeout_regiseter(uint32_t wait, val_t *handle, int repeat);
int timeout_unregiseter(int tid, int repeat);

#endif /* __TIMEOUT_INC__ */

