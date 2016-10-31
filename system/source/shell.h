#ifndef __SHELL_INC__
#define __SHELL_INC__

#include "util.h"

int  shell_init(env_t *env, void *mem, int size);
int  shell_start(const char *init_script);

void shell_event_proc(env_t *env, int event);

#endif /* __SHELL_INC__ */

