#MIT License
#
# Copyright (c) 2016 Lixing Ding <ding.lixing@gmail.com>
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

export BASE = ${PWD}
export PREFIX

MAKE_DIR  = ${BASE}/make

LANG_BUILD_DIR = ${BASE}/build/lang
TEST_BUILD_DIR = ${BASE}/build/test
EXAMPLE_BUILD_DIR = ${BASE}/build/example


.PHONY: all test cunit lang example build pre_lang pre_test pre_example

all: lang

pre_lang:
	@mkdir -p ${LANG_BUILD_DIR}

pre_test:
	@mkdir -p ${TEST_BUILD_DIR}

pre_example:
	@mkdir -p ${EXAMPLE_BUILD_DIR}

lang: pre_lang
	@printf "[Build] lang\n"
	@${MAKE} -C ${LANG_BUILD_DIR} -f ${MAKE_DIR}/lang.mk

test: lang pre_test
	@printf "[Build] test\n"
	@${RM} ${TEST_BUILD_DIR}/test
	@${MAKE} -C ${TEST_BUILD_DIR} -f ${MAKE_DIR}/test.mk
	@${TEST_BUILD_DIR}/test

example: lang pre_example
	@printf "[Build] example\n"
	@${MAKE} -C ${EXAMPLE_BUILD_DIR} -f ${MAKE_DIR}/example.mk

clean:
	@${RM} -rf build

