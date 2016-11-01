
#ifndef __TIMEOUT_INC__
#define __TIMEOUT_INC__

#include "util.h"

void timeout_init(void);
void systick_event_proc(env_t *env, int e);

int timeout_regiseter(uint32_t wait, val_t *handle, int repeat);
int timeout_unregiseter(int tid, int repeat);

#endif /* __TIMEOUT_INC__ */

