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

#include "err.h"
#include "executable.h"
#include "type_function.h"

int executable_init(executable_t *exe, void *mem_ptr, int mem_size,
                    int number_max, int string_max, int func_max, int code_max)
{
    int mem_offset = 0;

    exe->code = (uint8_t*) (mem_ptr + mem_offset);
    exe->main_code_end = 0;
    exe->func_code_end = code_max;
    mem_offset += code_max;
    mem_offset = SIZE_ALIGN_8(mem_offset);

    // static number buffer init
    exe->number_max = number_max;
    exe->number_num = 0;
    exe->number_map = (double*) (mem_ptr + mem_offset);
    mem_offset += sizeof(double) * number_max;

    // static string buffer init
    exe->string_max = string_max;
    exe->string_num = 0;
    exe->string_map = (intptr_t *) (mem_ptr + mem_offset);
    mem_offset += sizeof(intptr_t) * string_max;

    // script function buffer init
    exe->func_max = func_max;
    exe->func_num = 0;
    exe->func_map = (uint8_t **) (mem_ptr + mem_offset);
    mem_offset += sizeof(uint8_t **) * func_max;

    if (mem_offset > mem_size) {
        return -1;
    } else {
        return mem_offset;
    }
}

int executable_number_find_add(executable_t *exe, double n)
{
    int i;

    for (i = 0; i < exe->number_num; i++) {
        if (exe->number_map[i] == n) {
            return i;
        }
    }

    if (exe->number_num < exe->number_max) {
        exe->number_map[exe->number_num++] = n;
        return i;
    } else {
        return -1;
    }
}

int executable_string_find_add(executable_t *exe, intptr_t s)
{
    int i;

    if (s == 0) {
        return -1;
    }

    for (i = 0; i < exe->string_num; i++) {
        if (exe->string_map[i] == s) {
            return i;
        }
    }

    if (exe->string_num < exe->string_max) {
        exe->string_map[exe->string_num++] = s;
        return i;
    } else {
        return -1;
    }
}

int executable_func_set_head(void *buf, uint8_t vc, uint8_t ac, uint32_t code_size, uint16_t stack_size, int closure) {
    uint8_t *head = (uint8_t *)buf;
    int mark = 0;

    if (stack_size & 0x8000) {
        // stack overflow: not more than 32768
        return -1;
    }

    if (closure) {
        mark = 0x80;
    }

    head[0] = vc;
    head[1] = ac;

    head[2] = (stack_size >> 8) | mark; // High bit of stack_size, used as closure flag
    head[3] = stack_size;

    head[4] = code_size >> 24;
    head[5] = code_size >> 16;
    head[6] = code_size >> 8;
    head[7] = code_size;

    return 0;
}

int executable_func_get_head(void *buf, uint8_t *vc, uint8_t *ac, uint32_t *code_size, uint16_t *stack_size, int *closure) {
    uint8_t *head = (uint8_t *)buf;
    uint32_t size;
    int mark = 0;

    *vc = head[0];
    *ac = head[1];

    size = (head[2] * 0x100) + head[3];
    mark = size & 0x8000 ? 1 : 0;
    size = size & 0x7FFF;
    *stack_size = size;
    *closure = mark;

    size = (head[4] * 0x1000000 + head[5] * 0x10000 + head[6] * 0x100 + head[7]);
    *code_size = size;

    return 0;
}


int executable_main_add(executable_t *exe, void *code, uint16_t size, uint8_t vc, uint8_t ac, uint16_t stack_need, int closure)
{
    uint8_t *entry;

    if (exe->main_code_end + size + FUNC_HEAD_SIZE >= exe->func_code_end) {
        return ERR_NotEnoughMemory;
    }

    entry = exe->code + exe->main_code_end;
    if (exe->func_num == 0) {
        exe->func_map[0] = entry;
        exe->func_num = 1;
    }

    executable_func_set_head(entry, vc, ac, size, stack_need, closure);
    memcpy(entry + FUNC_HEAD_SIZE, code, size);

    exe->main_code_end += FUNC_HEAD_SIZE + size;

    return 0;
}

int executable_func_add(executable_t *exe, void *code, uint16_t size, uint8_t vc, uint8_t ac, uint16_t stack_need, int closure)
{
    uint8_t *entry;

    if (exe->main_code_end + size + FUNC_HEAD_SIZE >= exe->func_code_end) {
        return ERR_NotEnoughMemory;
    }

    exe->func_code_end -= FUNC_HEAD_SIZE + size;
    entry = exe->code + exe->func_code_end;

    exe->func_map[exe->func_num++] = entry;
    executable_func_set_head(entry, vc, ac, size, stack_need, closure);
    memcpy(entry + FUNC_HEAD_SIZE, code, size);

    return 0;
}



static inline
void image_write(image_info_t *ef, int offset, void *buf, int size) {
    memcpy(ef->base + offset, buf, size);
}

static inline
void image_write_zero(image_info_t *img, int offset, int size) {
    bzero(img->base + offset, size);
}

static inline
void image_write_byte(image_info_t *img, int offset, uint8_t d) {
    img->base[offset] = d;
}

static inline
void image_write_uint32(image_info_t *img, int offset, uint32_t d) {
    memcpy(img->base + offset, &d, sizeof(d));
}

static inline
void image_write_double(image_info_t *img, int offset, double *d) {
    memcpy(img->base + offset, d, sizeof(*d));
}

static inline
void image_read_byte(image_info_t *img, int offset, uint8_t *b) {
    *b = img->base[offset];
}

static inline
void image_read_uint32(image_info_t *img, int offset, uint32_t *u) {
    memcpy(u, img->base + offset, 4);
}

int image_load(image_info_t *img, uint8_t *input, int size)
{
    if (!img || !input || size < 64) {
        return -ERR_InvalidInput;
    }

    if (memcmp(input, "\177ELF", 4)) {
        return -ERR_InvalidInput;
    }

    img->base = input;
    img->size = size;
    img->end = size;

    image_read_byte(img, 4, &img->addr_size);
    image_read_byte(img, 5, &img->byte_order);
    image_read_byte(img, 6, &img->version);

    image_read_uint32(img, 16, &img->num_cnt);
    image_read_uint32(img, 20, &img->num_ent);
    image_read_uint32(img, 24, &img->str_cnt);
    image_read_uint32(img, 28, &img->str_ent);
    image_read_uint32(img, 32, &img->fn_cnt);
    image_read_uint32(img, 36, &img->fn_ent);

    return 0;
}

int image_init(image_info_t *img, void *mem_ptr, int mem_size, int byte_order, int num_cnt, int str_cnt, int fn_cnt)
{
    (void) byte_order;

    if (!img || !mem_ptr || mem_size < 64) {
        return -1;
    }

    img->base = mem_ptr;
    img->size = mem_size;
    img->byte_order = SYS_BYTE_ORDER;

    img->num_cnt = num_cnt;
    img->str_cnt = str_cnt;
    img->fn_cnt = fn_cnt;

    img->num_ent = 64;
    img->str_ent = SIZE_ALIGN_8(img->num_ent + 8 * num_cnt);
    img->fn_ent  = SIZE_ALIGN_8(img->str_ent + 4 * str_cnt);

    img->end = SIZE_ALIGN_16(img->fn_ent + 4 * fn_cnt + 16);

    image_write(img, 0, "\177ELF", 4);          // magic:
    image_write_byte(img, 4, 1);                // addr size:   1:32, 2:64
    image_write_byte(img, 5, SYS_BYTE_ORDER);   // byte order: LE:BE
    image_write_byte(img, 6, 0);                // version:     0
    image_write_zero(img, 7, 9);                // padding
    image_write_uint32(img, 16, img->num_cnt);
    image_write_uint32(img, 20, img->num_ent);
    image_write_uint32(img, 24, img->str_cnt);
    image_write_uint32(img, 28, img->str_ent);
    image_write_uint32(img, 32, img->fn_cnt);
    image_write_uint32(img, 36, img->fn_ent);
    image_write_zero(img, 40, 24);

    return 0;
}

int image_fill_data(image_info_t *img, unsigned int nc, double *nv, unsigned int sc, intptr_t *sv)
{
    unsigned int i, offset;

    if (nc != img->num_cnt || sc != img->str_cnt) {
        return -1;
    }

    for (i = 0; i < nc; i++) {
        image_write_double(img, img->num_ent + i * 8, nv + i);
    }

    offset = img->end;
    for (i = 0; i < sc; i++) {
        char *str = (char *)sv[i];
        int len = strlen(str) + 1;

        if (offset + len > img->size) {
            return -1;
        }

        image_write_uint32(img, img->str_ent + i * 4, offset);
        image_write(img, offset, str, len);
        offset += len;
    }

    img->end = SIZE_ALIGN_16(offset);

    // padding fill with zero
    image_write_zero(img, offset, img->end - offset);
    return 0;
}

int image_fill_code(image_info_t *img, unsigned int entry, uint8_t vc, uint8_t ac, uint16_t stack_need, int closure, uint8_t *code, unsigned int size)
{
    unsigned int offset, end;

    if (!img || entry >= img->fn_cnt) {
        return -1;
    }

    offset = img->end;
    end = SIZE_ALIGN_8(offset + FUNC_HEAD_SIZE + size);
    if (end > img->size) {
        return -1;
    }

    image_write_uint32(img, img->fn_ent + entry * 4, offset);

    executable_func_set_head(img->base + offset, vc, ac, size, stack_need, closure);
    offset += FUNC_HEAD_SIZE;

    image_write(img, offset, code, size);
    offset += size;

    // padding fill with zero
    image_write_zero(img, offset, end - offset);
    img->end = end;

    return 0;
}

double *image_number_entry(image_info_t *img)
{
    if (!img) {
        return NULL;
    }

    return (double *)(img->base + img->num_ent);
}

const char *image_get_string(image_info_t *img, int index)
{
    unsigned int offset;
    uint32_t entry;

    if (!img) {
        return NULL;
    }

    offset = img->str_ent + index * 4;
    if (offset >= img->end) {
        return NULL;
    }

    image_read_uint32(img, offset, &entry);

    return (const char*)(img->base + entry);
}

const uint8_t *image_get_function(image_info_t *img, int index)
{
    unsigned int offset;
    uint32_t entry;

    if (!img) {
        return NULL;
    }

    offset = img->fn_ent + index * 4;
    if (offset >= img->end) {
        return NULL;
    }

    image_read_uint32(img, img->fn_ent + index * 4, &entry);

    return (const uint8_t*)(img->base + entry);
}

