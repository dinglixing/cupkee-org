elf_NAMES = cupkee

cupkee_SRCS = ${notdir ${wildcard ${BASE_DIR}/main/*.c}}

cupkee_CPPFLAGS = -I${BSP_DIR}/include -I${SYS_DIR}/include -I${LANG_DIR}/include
cupkee_CFLAGS   =
cupkee_LDFLAGS  = -L${BSP_BUILD_DIR} -L${SYS_BUILD_DIR} -L${LANG_BUILD_DIR} -lsys -lbsp -llang

include ${MAKE_DIR}/cupkee.ruls.mk

VPATH = ${BASE_DIR}/main
