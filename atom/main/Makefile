elf_NAMES = out

out_SRCS = source/main.c \
		   source/shell.c \
		   source/native.c

out_CPPFLAGS = -I../hal/include -I../sal/include -I../../panda/include
out_CFLAGS   =
out_LDFLAGS  = -L../hal -L../sal -L../lang -lhal -lsal -llang

include ../../mk/cupkee.ruls.mk

