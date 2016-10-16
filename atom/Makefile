
export MCU = stm32f103ze

all: hal sal lang main
	@printf "build ok\n"

hal:
	@make -C hal

sal:
	@make -C sal

lang:
	@make -C lang

main: hal sal lang
	@make -C main

bin: main
	@make -C main bin

clean:
	@make -C hal clean
	@make -C sal clean
	@make -C lang clean
	@make -C main clean

do:
	rm -rf main/out.elf

load: do main
	openocd -f../openocd/interface/jlink.cfg -f../openocd/target/stm32f1x.cfg \
		-c "program main/out.elf verify reset exit"

.PHONY: bin hal sal lang main clean load
