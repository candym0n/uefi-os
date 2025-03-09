.PHONY: all clean run image bootloader tools

# Shush Makefile
MAKEFLAGS += --silent

SIZE := 1G

TARGET := build/test.img

GPTIMG_DIR := tools/gptimg

BOOT_DIR := boot

BUILD_SCRIPT := scripts/build.sh
QEMU_SCRIPT := scripts/qemu.sh

# Export all of the variables for scripts to use
export SIZE TARGET GPTIMG_DIR BOOT_DIR

all: image

clean:
	@echo "Cleaning up..."
	@cd $(BOOT_DIR) && make clean && cd ../..
	@cd $(GPTIMG_DIR) && make olsclean && cd ../..
	rm -rf build

bootloader:
	@echo "Building bootloader..."
	@cd $(BOOT_DIR) && make all && cd ../..

tools:
	@echo "Building tools..."
	@cd $(GPTIMG_DIR) && make all && cd ../..

image: bootloader tools
	@echo "Creating image..."
	@mkdir -p build
	@bash $(BUILD_SCRIPT)

run:
	@bash $(QEMU_SCRIPT)
