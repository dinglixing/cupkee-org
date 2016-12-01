#ifndef __MISC_INC__
#define __MISC_INC__

int scripts_save(const char *s);
const char *scripts_load(const char *prev);

void reference_init(env_t *env);
val_t *reference_create(val_t *v);
void reference_release(val_t *ref);

void print_error(int error);
void print_simple_value(val_t *v);
void print_value(val_t *v);

#endif /* __MISC_INC__ */

