#ifndef __SHELL_INC__
#define __SHELL_INC__

#include "panda.h"

int  shell_init(void);

void shell_execute(void);
void shell_timeout_execute(void);

int shell_timeout_regiseter(uint32_t wait, val_t *handle, int repeat);
int shell_timeout_unregiseter(int tid, int repeat);

#endif /* __SHELL_INC__ */

