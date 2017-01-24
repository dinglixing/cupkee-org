/*
MIT License

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


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int output(const char *s)
{
    return printf("%s", s);
}

void *file_load(const char *name, int *size)
{

    char *addr;
    int fd;
    struct stat sb;
    size_t length;

    fd = open(name, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }

    if (fstat(fd, &sb) == -1) {
        close(fd);
        return NULL;
    }
    length = sb.st_size + 1;

    addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
    *size = length;

    return addr;
}

int file_release(void *addr, int size)
{
    return munmap(addr, size);
}

int file_store(const char *name, void *buf, int sz)
{
    int fd = open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    int off, n;

    if (fd < 0) {
        return -1;
    }

    off = 0;
    while(off < sz) {
        n = write(fd, buf + off, sz - off);
        if (n < 0) {
            unlink(name);
            close(fd);
            return -1;
        }
        off += n;
    }

    close(fd);
    return 0;
}

int file_base_name(const char *name, void *buf, int sz)
{
    const char *base = rindex(name, '/');
    const char *suffix = rindex(name, '.');
    int len;

    base = base ? base : name;
    if (!suffix || suffix < base) {
        len = strlen(base);
    } else {
        len = suffix - base;
    }

    if (len < sz) {
        ((char *)buf)[len] = 0;
        memcpy(buf, base, len);
        return len;
    }

    return -1;
}

