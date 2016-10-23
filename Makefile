
export MCU = stm32f103rc


export BASE_DIR = ${PWD}
export MAKE_DIR = ${BASE_DIR}/mk

export BSP_DIR = ${BASE_DIR}/boards
export SYS_DIR = ${BASE_DIR}/system
export LANG_DIR  = ${BASE_DIR}/panda

BUILD_DIR = ${BASE_DIR}/build
export BSP_BUILD_DIR = ${BUILD_DIR}/bsp
export SYS_BUILD_DIR = ${BUILD_DIR}/sys
export LANG_BUILD_DIR = ${BUILD_DIR}/lang

all: build main
	@printf "build ok\n"

build:
	@mkdir -p ${LANG_BUILD_DIR} ${BSP_BUILD_DIR} ${SYS_BUILD_DIR}

bsp:
	@make -C ${BSP_BUILD_DIR} -f ${MAKE_DIR}/bsp.mk

sys:
	@make -C ${SYS_BUILD_DIR} -f ${MAKE_DIR}/sys.mk

lang:
	@make -C ${LANG_BUILD_DIR} -f ${MAKE_DIR}/lang.mk

main: bsp sys lang
	@make -C ${BUILD_DIR} -f ${MAKE_DIR}/main.mk

bin: main
	@make -C ${BUILD_DIR} -f ${MAKE_DIR}/main.mk bin

clean:
	@rm -rf ${BUILD_DIR}

do:
	rm -rf main/out.elf

load: do main
	openocd -f../openocd/interface/jlink.cfg -f../openocd/target/stm32f1x.cfg \
		-c "program build/cupkee.elf verify reset exit"

.PHONY: bin hal sal lang main clean load build
