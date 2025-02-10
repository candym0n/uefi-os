.PHONY: all clean run

all:
	@sh scripts/build.sh
	echo "Run make run to test qemu"

run:
	@sh scripts/qemu.sh

clean:
	@cd boot && make clean && cd ..
	rm -f test.img
