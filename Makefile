.PHONY: all clean

all:
	@sh scripts/build.sh
#	sh scripts/qemu.sh

clean:
	@cd boot && make clean && cd ..
	rm -f test.img
