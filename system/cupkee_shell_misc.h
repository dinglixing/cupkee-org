
#ifndef __CUPKEE_SHELL_MISC_INC__
#define __CUPKEE_SHELL_MISC_INC__

#include <cupkee.h>

void shell_reference_init(env_t *env);
val_t *shell_reference_create(val_t *v);
void shell_reference_release(val_t *ref);

void print_simple_value(val_t *v);
void shell_print_value(val_t *v);
void shell_print_error(int error);
void shell_do_callback(env_t *env, val_t *cb, uint8_t ac, val_t *av);
void shell_do_callback_error(env_t *env, val_t *cb, int code);
void shell_do_callback_buffer(env_t *env, val_t *cb, type_buffer_t *buffer);

val_t shell_error(env_t *env, int code);
int shell_string_id(val_t *in, int max, const char **names);

#endif /* __CUPKEE_SHELL_MISC_INC__ */

