#MIT License
#
#Copyright (c) 2016 Lixing Ding <ding.lixing@gmail.com>
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.

lib_NAMES = lang

lang_SRCS = heap.c \
			ast.c \
			env.c\
			gc.c \
			lex.c \
			val.c \
			parse.c \
			bcode.c \
			compile.c\
			executable.c \
			interp.c \
			type_number.c \
			type_function.c \
			type_array.c \
			type_string.c \
			type_buffer.c \
			type_object.c \

lang_CPPFLAGS = -I.. -Wall -Wundef
lang_CFLAGS   = -g -Werror
lang_LDFLAGS  =

VPATH = ${BASE}/lang

include ${BASE}/make/Makefile.pub

