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

lib_NAMES = example
bin_NAMES = compile dump repl panda

example_SRCS = sal.c native.c
example_CPPFLAGS = -I${BASE} -Wall -Werror
example_CFLAGS   = -g

compile_SRCS = compile.c
compile_CPPFLAGS = -I${BASE}
compile_CFLAGS   = -g
compile_LDFLAGS  = -L. -L${BASE}/build/lang -lexample -llang

dump_SRCS = dump.c
dump_CPPFLAGS = -I${BASE}
dump_CFLAGS   = -g
dump_LDFLAGS  = -L. -L${BASE}/build/lang -lexample -llang

repl_SRCS = interactive.c
repl_CPPFLAGS = -I${BASE}
repl_CFLAGS   = -g
repl_LDFLAGS  = -L. -L${BASE}/build/lang -lexample -llang -lreadline

panda_SRCS = interpreter.c
panda_CPPFLAGS = -I${BASE}
panda_CFLAGS   = -g
panda_LDFLAGS  = -L. -L${BASE}/build/lang -lexample -llang

VPATH = ${BASE}/example

include ${BASE}/make/Makefile.pub
