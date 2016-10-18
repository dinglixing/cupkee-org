

#ifndef __STORAGE_INC__
#define __STORAGE_INC__

#define CON_IN          0
#define CON_OUT         1
#define CON_BUFF_MAX    2
#define CON_BUFF_SIZE 128

int storage_init(void);

const char *storage_script_next(int *ctx);
const char *storage_script_prev(int *ctx);
const char *storage_script_get(int i);
int storage_script_append(const char *s);
int storage_script_clear(void);
int storage_script_del(int i);


#endif /* __STORAGE_INC__ */

