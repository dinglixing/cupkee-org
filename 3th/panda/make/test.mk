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

lib_NAMES = cunit
bin_NAMES = test

cunit_SRCS = CUnit_Basic.c \
			 CUnit_Error.c \
			 CUnit_Mem.c \
			 CUnit_TestDB.c \
			 CUnit_TestRun.c \
			 CUnit_Util.c
cunit_CPPFLAGS =
cunit_CFLAGS   =
cunit_LDFLAGS  =

test_SRCS = test.c \
	   		test_util.c \
	   		test_hello.c \
			test_lang_lex.c  \
			test_lang_val.c  \
			test_lang_parse.c \
			test_lang_symtbl.c \
			test_lang_exec.c \
			test_lang_image.c \
			test_lang_async.c \
			test_lang_foreign.c \
			test_lang_type_buffer.c
test_CPPFLAGS = -I${BASE}
test_CFLAGS   =
test_LDFLAGS  = -L${BASE}/build/lang -L. -llang -lcunit

VPATH = ${BASE}/cunit:${BASE}/test

include ${BASE}/make/Makefile.pub

