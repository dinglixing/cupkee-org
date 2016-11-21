/*******************************************************************************
 * dbg field
*******************************************************************************/
#include "hardware.h"

#define SCRIPTS_MAX_SIZE    (1 * 1024)

static char dbg_scripts_storage[SCRIPTS_MAX_SIZE];
static int  dbg_scripts_end = 0;


/*******************************************************************************
 * hw field
*******************************************************************************/
#include <bsp.h>

static inline int storage_ptr_valid(const void *addr)
{
    int dis = (intptr_t)addr - (intptr_t)dbg_scripts_storage;

    return dis >= 0 && dis < SCRIPTS_MAX_SIZE && (dis % 4) == 0;
}

static int storage_clear(const void *addr, int size)
{
    if (!storage_ptr_valid(addr)) {
        return -1;
    }

    if (size % 4) {
        size += 4 - size % 4;
    }

    memset((void *)addr, 0, size);

    return 0;
}

static int storage_write(const void *data, int size)
{
    uint8_t *bgn = (uint8_t *)dbg_scripts_storage + dbg_scripts_end;
    uint8_t *end = (uint8_t *)dbg_scripts_storage + SCRIPTS_MAX_SIZE;
    int tail;

    if (!storage_ptr_valid(bgn)) {
        return -1;
    }

    if (size < 1) {
        return 0;
    }

    while(bgn < end && *((uint32_t *)bgn) != 0xffffffff) {
        bgn += 4;
    }
    if (size > end - bgn) {
        return -1;
    }

    memcpy(bgn, data, size);

    tail = size % 4;
    if (tail) {
        memset(bgn + size, 0, 4 - tail);
        size += 4 - tail;
    }

    dbg_scripts_end += size;

    return 0;
}

static const char *hw_scripts_get(int id)
{
    const char *s;

    s = hw_scripts_load(NULL);
    while (s && 0 < id--) {
        s = hw_scripts_load(s);
    }

    return s;
}

/*******************************************************************************
 * bsp interface
*******************************************************************************/
int hw_scripts_erase(void)
{
    memset(dbg_scripts_storage, 0xff, SCRIPTS_MAX_SIZE);
    dbg_scripts_end = 0;
    return 0;
}

int hw_scripts_remove(int id)
{
    const char *pos = hw_scripts_get(id);

    if (pos) {
        return storage_clear(pos, strlen(pos));
    }
    return -1;
}

const char *hw_scripts_load(const char *prev)
{
    char *cur = prev ? (char *)prev : (char *) dbg_scripts_storage;
    char *end = (char *) dbg_scripts_storage +  SCRIPTS_MAX_SIZE;

    if (!storage_ptr_valid(cur)) {
        return NULL;
    }

    // Skip zero
    while (*cur == 0 && cur != end) {
        cur++;
    }
    if (cur == end || *cur == -1) {
        return NULL;
    }

    if (!prev) {
        return cur;
    }

    // Skip current string
    while (*cur != 0 && cur != end) {
        cur++;
    }
    if (cur == end || *cur == -1) {
        return NULL;
    }

    // Skip string terminate and paddings
    while (*cur == 0 && cur != end) {
        cur++;
    }
    if (cur == end || *cur == -1) {
        return NULL;
    }

    return cur;
}

int hw_scripts_save(const char *s)
{
    int len = strlen(s);

    if (!len) {
        return 0;
    }
    len += 1; // Include the terminate char '\000'

    return storage_write(s, len);
}

void hw_led_set(void)
{
}

void hw_led_clear(void)
{
}

void hw_led_toggle(void)
{
}

