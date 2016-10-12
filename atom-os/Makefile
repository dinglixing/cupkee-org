
MCU   = stm32f103ze

export MCU

all: main
	@printf "build ok\n"

hal:
	@make -C hal

sal:
	@make -C sal

main: hal sal
	@make -C main

bin: main
	@make -C main bin

clean:
	@make -C hal clean
	@make -C sal clean
	@make -C main clean

.PHONY: bin hal sal main clean

