/*
MIT License

This file is part of cupkee project.

Copyright (c) 2016 Lixing Ding <ding.lixing@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "hardware.h"

static int  _storage_sec_size;
static int  _storage_usr_size;
static char *_storage_usr_base;

static inline int storage_ptr_valid(const void *addr)
{
    int dis = (intptr_t)addr - (intptr_t)_storage_usr_base;

    return dis >= 0 && dis < _storage_usr_size && (dis % 4) == 0;
}

static int storage_clear(const void *addr, int size)
{
    if (!storage_ptr_valid(addr)) {
        return -1;
    }

    if (size % 4) {
        size += 4 - size % 4;
    }

    uint32_t v = 0;
    int off;

    flash_unlock();
    for (off = 0; off < size; off += 4) {
        flash_program_word((uint32_t)addr + off, v);
    }
    flash_lock();

    for (off = 0; off < size; off += 4) {
        if (*((uint32_t*)(addr + off)) != v) {
            return -1;
        }
    }

    return 0;
}

static int storage_write(const void *data, int size)
{
    static uint8_t *last = 0;
    uint8_t *bgn = last ? last : (uint8_t *)_storage_usr_base;
    uint8_t *end = (uint8_t *)_storage_usr_base + _storage_usr_size;
    int head, tail, off;

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

    tail = size % 4;
    head = size - tail;

    flash_unlock();
    for (off = 0; off < head; off += 4) {
        flash_program_word((uint32_t)(bgn + off), *((uint32_t *)(data + off)));
    }

    if (tail) {
        uint32_t tail_val = 0;

        memcpy(&tail_val, data + off, tail);
        flash_program_word((uint32_t)(bgn + off), tail_val);
        off += 4;
    }
    flash_lock();

    if (!memcmp(bgn, data, size)) {
        last = bgn + off;
        return 0;
    } else {
        return -1;
    }
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

void hw_setup_storage(void)
{
    int rom = desig_get_flash_size();

    if (rom >= 256) {
        _storage_sec_size = 2 * 1024;
        _storage_usr_size = 8 * 1024;
    } else {
        _storage_sec_size = 1 * 1024;
        _storage_usr_size = 8 * 1024;
    }
    rom *= 1024;

    _storage_usr_base = (char *) (0x08000000 + (rom - _storage_usr_size));
}

int hw_scripts_erase(void)
{
    int off;

    flash_unlock();
    for (off = 0; off < _storage_usr_size; off += _storage_sec_size) {
        flash_erase_page((uint32_t)(_storage_usr_base + off));
        if (flash_get_status_flags() != FLASH_SR_EOP) {
            return -1;
        }
    }
    flash_lock();

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
    char *cur = prev ? (char *)prev : (char *) _storage_usr_base;
    char *end = (char *) _storage_usr_base +  _storage_usr_size;

    if (!storage_ptr_valid(cur)) {
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

int hw_scripts_save(const char *s)
{
    int len = strlen(s);

    if (!len) {
        return 0;
    }
    len += 1; // Include the terminate char '\000'

    return storage_write(s, len);
}

