#ifndef __SHELL_INC__
#define __SHELL_INC__

#include "panda.h"

int  shell_init(void);

val_t *shell_reference_create(val_t *v);
void shell_reference_release(val_t *ref);

void shell_input_execute(void);
void shell_timeout_execute(void);


#endif /* __SHELL_INC__ */

