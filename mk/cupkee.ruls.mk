## This file is part of the cupkee project.
#
# cupkee.ruls.mk
#
# Copyright (C) 2016 ding.lixing@gmail.com
#
# public make rules
# user should not modify anything there

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q		:= @
NULL 	:= 2>/dev/null
endif

###############################################################################
# Executables

PREFIX  ?= arm-none-eabi-
AS  	:= ${PREFIX}as
CC 		:= ${PREFIX}gcc
LD  	:= ${PREFIX}gcc
AR  	:= ${PREFIX}ar
CXX 	:= $(PREFIX)g++
OBJCOPY	:= $(PREFIX)objcopy
OBJDUMP	:= $(PREFIX)objdump
GDB		:= $(PREFIX)gdb

RM 		:= rm -rf
MAKE 	:= make

OPT		:= -Os
CSTD    ?= -std=c99


###############################################################################
# Depend files

ifeq ($(strip $(MCU)),)
$(error "Miss target mcu")
endif
LDSCRIPT    = ../../ld/$(MCU).ld

ifeq ($(findstring stm32f1,${MCU}),stm32f1)
LIBNAME		= opencm3_stm32f1
DEFS		+= -DSTM32F1

FP_FLAGS	?= -msoft-float
ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

OPENCM3_DIR = ../../libopencm3
else
	$(error "mcu ${MCU}: not support now!")
endif

DEFS		+= -I$(OPENCM3_DIR)/include
OPENCM3_LIBS = -L$(OPENCM3_DIR)/lib -l$(LIBNAME)
SYSTEM_LIBS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

ifeq ($(V),1)
$(info Using $(OPENCM3_DIR) path to library)
endif


###############################################################################
# C flags

TGT_CFLAGS	+= $(OPT) $(CSTD) -g
TGT_CFLAGS	+= $(ARCH_FLAGS)
TGT_CFLAGS	+= -Wextra -Wshadow -Wimplicit-function-declaration
TGT_CFLAGS	+= -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
TGT_CFLAGS	+= -fno-common -ffunction-sections -fdata-sections

###############################################################################
# C++ flags

TGT_CXXFLAGS	+= $(OPT) $(CXXSTD) -g
TGT_CXXFLAGS	+= $(ARCH_FLAGS)
TGT_CXXFLAGS	+= -Wextra -Wshadow -Wredundant-decls  -Weffc++
TGT_CXXFLAGS	+= -fno-common -ffunction-sections -fdata-sections

###############################################################################
# C & C++ preprocessor common flags

TGT_CPPFLAGS	+= -MD
TGT_CPPFLAGS	+= -Wall -Wundef
TGT_CPPFLAGS	+= $(DEFS)


###############################################################################
# Linker flags

TGT_LDFLAGS		+= --static -nostartfiles
TGT_LDFLAGS		+= -T$(LDSCRIPT)
TGT_LDFLAGS		+= $(ARCH_FLAGS)
TGT_LDFLAGS		+= -Wl,--gc-sections
ifeq ($(V),99)
TGT_LDFLAGS		+= -Wl,--print-gc-sections
endif
TGT_LDFLAGS     += ${SYSTEM_LIBS} ${OPENCM3_LIBS}

###############################################################################
# Build ruls

%.images: %.bin %.hex %.srec %.list %.map
	@printf "*** $* images generated ***\n"

%.bin: %.elf
	@printf "  OBJCOPY $(*).bin\n"
	$(Q)$(OBJCOPY) -Obinary $(*).elf $(*).bin

%.hex: %.elf
	@printf "  OBJCOPY $(*).hex\n"
	$(Q)$(OBJCOPY) -Oihex $(*).elf $(*).hex

%.srec: %.elf
	@printf "  OBJCOPY $(*).srec\n"
	$(Q)$(OBJCOPY) -Osrec $(*).elf $(*).srec

%.list: %.elf
	@printf "  OBJDUMP $(*).list\n"
	$(Q)$(OBJDUMP) -S $(*).elf > $(*).list


# Marco build_obj_rule
# param ${1}: source file
# param ${2}: program or library name
define build_obj_rule
${1:%.c=%.o}: ${1}
	@printf "[CC]\t$$@\n"
	$(Q)${CC} ${TGT_CPPFLAGS} ${${2}_CPPFLAGS} ${TGT_CFLAGS} ${${2}_CFLAGS} -MD -c $$< -o $$@
endef

# Marco build_elf_rule
# param ${1}: target name
define build_elf_rule
${1}_OBJS = ${${1}_SRCS:%.c=%.o}
${1}_DEPS = ${${1}_SRCS:%.c=%.d}
${1}: ${1}.elf

${1}.elf: $${${1}_OBJS}
	@printf "[LD]\t$$@\n"
	$(Q)${LD} -o $$@ $${${1}_OBJS} ${TGT_LDFLAGS} -Wl,-Map=${1}.map $${${1}_LDFLAGS}

${1}.clean:
	$(Q)${RM} $${${1}_OBJS} $${${1}_DEPS} ${1}.elf ${1}.map

$(foreach src,${${1}_SRCS},$(eval $(call build_obj_rule,${src},${1})))

-include $${${1}_DEPS}
endef

# Marco build_lib_rule
# param ${1}: library name
define build_lib_rule
${1}_OBJS = ${${1}_SRCS:%.c=%.o}
${1}_DEPS = ${${1}_SRCS:%.c=%.d}
${1}: lib${1}.a

lib${1}.a: $${${1}_OBJS}
	@printf "[AR]\t$$@\n"
	$(Q)${AR} rcs lib${1}.a $${${1}_OBJS}

${1}.clean:
	$(Q)${RM} $${${1}_OBJS} $${${1}_DEPS} lib${1}.a

$(foreach src,${${1}_SRCS},$(eval $(call build_obj_rule,${src},${1})))

-include $${${1}_DEPS}
endef

###############################################################################
###############################################################################
# Build target

all: elf lib

elf: ${elf_NAMES}

lib: ${lib_NAMES}

bin: ${elf_NAMES} ${elf_NAMES:%=%.bin}

hex: ${elf_NAMES} ${elf_NAMES:%=%.hex}

list: ${elf_NAMES} ${elf_NAMES:%=%.list}

clean: ${elf_NAMES:%=%.clean} ${lib_NAMES:%=%.clean}
	@${RM} ${elf_NAMES:%=%.bin}
	@${RM} ${elf_NAMES:%=%.hex}
	@${RM} ${elf_NAMES:%=%.list}

$(foreach elf,${elf_NAMES},$(eval $(call build_elf_rule,${elf})))

$(foreach lib,${lib_NAMES},$(eval $(call build_lib_rule,${lib})))


.PHONY: clean lib all images elf bin hex srec list

