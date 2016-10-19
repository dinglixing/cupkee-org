#include "sal.h"


int storage_init(void)
{
    return 0;
}

/*
static inline int hal_storage_usr_is_valid_ptr(const char *ptr) {
    int dis = ptr - (const char *)_storage;

    return dis >= 0 && dis < _SIZE && (((intptr_t)ptr & 3) == 0);
}
*/

const char *usr_scripts_next(const char *prev)
{
    char *cur = prev ? (char *)prev : (char *)hal_storage_base_usr();
    char *end = (char *)hal_storage_base_usr() + hal_storage_size_usr();

    if (!hal_storage_valid_usr(cur)) {
        return NULL;
    }

    // Skip zero
    while (*cur == 0 && cur != end) {
        cur++;
    }
    if (cur == end || *cur == 0xff) {
        return NULL;
    }

    if (!prev) {
        return cur;
    }

    // Skip current string
    while (*cur != 0 && cur != end) {
        cur++;
    }
    if (cur == end || *cur == 0xff) {
        return NULL;
    }

    // Skip string terminate and paddings
    while (*cur == 0 && cur != end) {
        cur++;
    }
    if (cur == end || *cur == 0xff) {
        return NULL;
    }

    return cur;
}

const char *usr_scripts_get(int id)
{
    const char *s;

    s = usr_scripts_next(NULL);
    while (s && 0 < id--) {
        s = usr_scripts_next(s);
    }

    return s;
}

int usr_scripts_remove(int id)
{
    const char *pos = usr_scripts_get(id);

    if (pos) {
        return hal_storage_clear_usr(pos, strlen(pos));
    }
    return -1;
}

int usr_scripts_append(const char *s)
{
    int len = strlen(s);

    if (!len) {
        return 0;
    }
    len += 1; // Include the terminate char '\000'

    return hal_storage_write_usr(s, len);
}

int usr_scripts_erase(void)
{
    return hal_storage_erase_usr();
}

