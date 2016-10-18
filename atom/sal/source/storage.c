#include "sal.h"

#define _SIZE (1024 * 4)
static uint8_t _storage[_SIZE];

int storage_init(void)
{
    int i;

    for (i = 0; i < _SIZE; i++) {
        _storage[i] = 0xff;
    }
    return 0;
}

static char xx[128];
const char *storage_script_next(int *ctx)
{
    int pos = ctx ? *ctx : 0;
    int len = 0;
    const char *s;

    /*
    snprintf(xx, 128, "pos: %d\r\n", pos);
    console_puts(xx);

    if (_storage[0] != 0xff)
        console_puts(_storage);
    return NULL;
    */

    // Skip zero
    while (pos < _SIZE) {
        if (_storage[pos])
            break;
        pos++;
    }

    if (_storage[pos] == 0xff) {
        return NULL;
    }

    s = (const char *)_storage + pos;
    while (pos < _SIZE) {
        uint8_t byte = _storage[pos + len];

        if (!byte)
            break;
        len++;
    }

    if (ctx)
        *ctx = pos + len;

    return len ? s : NULL;
}

const char *storage_script_prev(int *ctx)
{
    int pos = ctx ? *ctx : 0;

    /* skip Paddings(0) at tail of string */
    while (pos > 0) {
        uint8_t byte = _storage[pos - 1];

        if (byte) {
            break;
        }
        pos--;
    }

    /* locate string head */
    while (pos > 0) {
        uint8_t byte = _storage[pos - 1];
        if (!byte) {
            if (ctx)
                *ctx = pos;
            return (const char *)_storage + pos;
        }
        pos--;
    }

    return NULL;
}

const char *storage_script_get(int i)
{
    const char *s;
    int ctx = 0;

    s = storage_script_next(&ctx);
    while (s && 0 < i--) {
        s = storage_script_next(&ctx);
    }

    return s;
}

static int storage_script_space_locate(int size)
{
    uint32_t *p = (uint32_t *) _storage;
    int pos = 0;

    while(pos < _SIZE) {
        if (p[pos >> 2] == 0xffffffff) {
            break;
        }
        pos += 4;
    }

    if (pos + size >= _SIZE) {
        return -1;
    } else {
        return pos;
    }
}

static int storage_script_bzero(int start, int len)
{
    uint32_t zero = 0;
    uint32_t *dst = (uint32_t *) (_storage + start);
    int pos = 0;

    while (pos + 4 <= len) {
        *dst++ = zero;
        pos += 4;
    }

    if (pos < len) {
        *dst = zero;
    }

    for (pos = 0; pos < len; pos++) {
        if (_storage[start + pos]) {
            return -1;
        }
    }
    return 0;
}

static int storage_script_write(int start, const void *data, int len)
{
    uint32_t *dst = (uint32_t *) (_storage + start);
    uint32_t *src = (uint32_t *) data;
    int pos = 0;

    while (pos + 4 <= len) {
        *dst++ = *src++;
        pos += 4;
    }

    if (pos < len) {
        uint32_t v = 0;
        memcpy(&v, data + pos, len - pos);
        *dst = v;
    }

    return memcmp(_storage + start, data, len) == 0 ? 0 : -1;
}

int storage_script_append(const char *s)
{
    int len = strlen(s) + 1; // Include the terminate char '\000'
    int start = storage_script_space_locate(len);

    if (start < 0) {
        return -1;
    }

    return storage_script_write(start, s, len);
}

int storage_script_clear(void)
{
    return 0;
}

int storage_script_del(int n)
{
    int pos = 0, i;
    const char *s;

    if (!(s = storage_script_next(&pos))) {
        return -1;
    }

    for (i = 0; i < n; i++) {
        if (!(s = storage_script_next(&pos)))
            return -1;
    }

    return storage_script_bzero(pos, strlen(s));
}

