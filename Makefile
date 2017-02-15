##
## MIT License
##
## This file is part of cupkee project.
##
## Copyright (c) 2016 Lixing Ding <ding.lixing@gmail.com>
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in all
## copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##

ifeq (${BOARD},)
$(info "Target board not specified...")
$(info "test will be build.")
endif

# Define default target board
export BOARD ?= test

export BASE_DIR = ${PWD}
export MAKE_DIR = ${BASE_DIR}/make

export INC_DIR = ${BASE_DIR}/include
export BSP_DIR = ${BASE_DIR}/boards
export SYS_DIR = ${BASE_DIR}/system
export TST_DIR  = ${BASE_DIR}/test

export MOD_DIR = ${BASE_DIR}/module
export APP_DIR = ${BASE_DIR}/app

export LANG_DIR  = ${MOD_DIR}/panda

BUILD_DIR = ${BASE_DIR}/build/${BOARD}
export BSP_BUILD_DIR = ${BUILD_DIR}/bsp
export SYS_BUILD_DIR = ${BUILD_DIR}/sys
export LANG_BUILD_DIR = ${BUILD_DIR}/lang

all: ogin
	@printf "ok\n"

build:
	@mkdir -p ${LANG_BUILD_DIR} ${BSP_BUILD_DIR} ${SYS_BUILD_DIR}

bsp:
	@make -C ${BSP_BUILD_DIR} -f ${MAKE_DIR}/bsp.mk

sys:
	@make -C ${SYS_BUILD_DIR} -f ${MAKE_DIR}/sys.mk

lang:
	@make -C ${LANG_BUILD_DIR} -f ${MAKE_DIR}/lang.mk

ogin: build bsp sys lang
	@mkdir -p ${BUILD_DIR}/ogin
	@make -C ${BUILD_DIR}/ogin -f ${MAKE_DIR}/ogin.mk extend

tiny:  build bsp sys lang
	@mkdir -p ${BUILD_DIR}/tiny
	@make -C ${BUILD_DIR}/tiny -f ${MAKE_DIR}/main.mk extend

atom:  build bsp sys lang
	@mkdir -p ${BUILD_DIR}/atom
	@make -C ${BUILD_DIR}/atom -f ${MAKE_DIR}/atom.mk extend

test: build bsp sys lang
	@rm -rf ${BUILD_DIR}/test.elf
	@make -C ${BUILD_DIR} -f ${MAKE_DIR}/test.mk
	${BUILD_DIR}/test.elf

clean:
	@rm -rf ${BUILD_DIR}

load:
	openocd -fopenocd/interface/jlink.cfg -fopenocd/target/stm32f1x.cfg \
		-c "program ${BUILD_DIR}/cupkee.elf verify reset exit"

.PHONY: clean load build main bsp lang sys ogin tiny atom

