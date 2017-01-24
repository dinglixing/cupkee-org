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

#ifndef __CUPKEE_CONFIG__
#define __CUPKEE_CONFIG__

#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NULL
# define NULL ((void *)0)
#endif

#define LE                          1
#define BE                          2

#define ADDRSIZE_32                 1
#define ADDRSIZE_64                 2

#define SYS_BYTE_ORDER              (BYTE_ORDER == LITTLE_ENDIAN ? LE : BE)
#define SYS_ADDR_SIZE               ADDRSIZE_64

#define ADDR_ALIGN_4(a)             ((void *)((((intptr_t)(a)) +  3) & ~0x03L))
#define ADDR_ALIGN_8(a)             ((void *)((((intptr_t)(a)) +  7) & ~0x07L))
#define ADDR_ALIGN_16(a)            ((void *)((((intptr_t)(a)) + 15) & ~0x0FL))
#define ADDR_ALIGN_32(a)            ((void *)((((intptr_t)(a)) + 31) & ~0x1FL))

#define SIZE_ALIGN_4(x)             (((x) +  3) & ~0x03)
#define SIZE_ALIGN_8(x)             (((x) +  7) & ~0x07)
#define SIZE_ALIGN_16(x)            (((x) + 15) & ~0x0F)
#define SIZE_ALIGN_32(x)            (((x) + 31) & ~0x1F)
#define SIZE_ALIGN_64(x)            (((x) + 63) & ~0x3F)
#define SIZE_ALIGN_64(x)            (((x) + 63) & ~0x3F)

#define SIZE_ALIGN                  SIZE_ALIGN_8
#define ADDR_ALING                  ADDR_ALIGN_8

// lang profile
#define MAGIC_BASE                  (0xE0)
#define TOKEN_MAX_SIZE              (32)

# define INTERACTIVE_VAR_MAX        (32)

# define DEF_PROP_SIZE              (4)
# define DEF_ELEM_SIZE              (8)
# define DEF_FUNC_SIZE              (4)
# define DEF_VMAP_SIZE              (4)
# define DEF_FUNC_CODE_SIZE         (32)

# define LIMIT_VMAP_SIZE            (32)    // max variable number in function
# define LIMIT_FUNC_SIZE            (32767) // max function number in  module
# define LIMIT_FUNC_CODE_SIZE       (32767) // max code of each function

# define DEF_STRING_SIZE            (8)

// lang compile resource default and limit

#endif /* __CUPKEE_CONFIG__ */

