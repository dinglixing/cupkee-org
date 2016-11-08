
elf_NAMES = test
test_SRCS = ${notdir ${wildcard ${BASE_DIR}/test/*.c}}
test_SRCS +=	CUnit_Basic.c \
				CUnit_Error.c \
				CUnit_Mem.c \
				CUnit_TestDB.c \
				CUnit_TestRun.c \
				CUnit_Util.c \

test_CPPFLAGS = -I${BSP_DIR}/include -I${SYS_DIR}/include -I${LANG_DIR}/include
test_CPPFLAGS += -I${TEST_DIR}/cunit -I${BSP_DIR}/test

test_CFLAGS   =
test_LDFLAGS  = -L${BSP_BUILD_DIR} -L${SYS_BUILD_DIR} -L${LANG_BUILD_DIR} -lsys -lbsp -llang

include ${MAKE_DIR}/cupkee.ruls.mk

VPATH = ${BASE_DIR}/test:${BASE_DIR}/test/cunit
