#ifndef __SHELL_NATIVE_INC__
#define __SHELL_NATIVE_INC__


void print_error(int error);
void print_value(val_t *v);
int native_init(env_t *env);

#endif /* __SHELL_NATIVE_INC__ */

