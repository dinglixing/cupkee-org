#ifndef __MISC_INC__
#define __MISC_INC__

void print_error(int error);
void print_value(val_t *v);

val_t native_led(env_t *env, int ac, val_t *av);
val_t native_sysinfos(env_t *env, int ac, val_t *av);
val_t native_systicks(env_t *env, int ac, val_t *av);
val_t native_print(env_t *env, int ac, val_t *av);
val_t native_scripts(env_t *env, int ac, val_t *av);

#endif /* __MISC_INC__ */

