

#ifndef __STORAGE_INC__
#define __STORAGE_INC__

#define CON_IN          0
#define CON_OUT         1
#define CON_BUFF_MAX    2
#define CON_BUFF_SIZE 128

int storage_init(void);

const char *usr_scripts_next(const char *prev);
const char *usr_scripts_get(int id);
int usr_scripts_append(const char *s);
int usr_scripts_remove(int id);
int usr_scripts_erase(void);


#endif /* __STORAGE_INC__ */

