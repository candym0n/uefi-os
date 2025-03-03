.PHONY: all clean run image bootloader tools

all: image

run:
	@sh scripts/qemu.sh

clean:
	@echo "Cleaning up..."
	@cd src/boot && make clean && cd ../..
	@cd tools/gptimg && make clean && cd ../..
	rm -f test.img

bootloader:
	@echo "Building bootloader..."
	@cd src/boot && make all && cd ../..

tools:
	@echo "Building tools..."
	@cd tools/gptimg && make all && cd ../..

image: bootloader tools
	@echo "Creating image..."
	@sh scripts/build.sh
