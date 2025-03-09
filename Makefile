# Shush Makefile
# MAKEFLAGS += --silent

# Directories used for compiling
BUILD_DIR := $(abspath build)
INCLUDE_DIR := $(abspath include)

# The size of the target image
SIZE := 1G

# The target image
TARGET := $(BUILD_DIR)/test.img

# Directories of tools
GPTIMG_DIR := $(abspath tools/gptimg)

# Directories of different parts of the OS
BOOT_DIR := $(abspath boot)
LIB_DIR := $(abspath lib)

# Scripts
BUILD_SCRIPT := scripts/build.sh
QEMU_SCRIPT := scripts/qemu.sh

# Export all of the variables for scripts to use
export SIZE TARGET GPTIMG_DIR BOOT_DIR

# Include default include dirs
MAKE_FLAGS := INCLUDE_DIR=-I$(INCLUDE_DIR) LIBRARY="-L$(LIB_DIR)/build/ -llibrary"

.PHONY: all clean run image bootloader tools lib

all: image

clean:
	@echo "Cleaning up..."
	@cd $(BOOT_DIR) && make clean && cd $(CURDIR)
	@cd $(GPTIMG_DIR) && make clean && cd $(CURDIR)
	@cd $(LIB_DIR) && make clean && cd $(CURDIR)
	rm -rf $(BUILD_DIR)

bootloader:
	@echo "Building bootloader..."
	@cd $(BOOT_DIR) && make all $(MAKE_FLAGS) && cd $(CURDIR)

tools: lib
	@echo "Building tools..."
	@cd $(GPTIMG_DIR) && make all $(MAKE_FLAGS) && cd $(CURDIR)

image: bootloader tools
	@mkdir -p $(BUILD_DIR)
	@bash $(BUILD_SCRIPT)

lib:
	@echo "Building library..."
	@cd $(LIB_DIR) && make all $(MAKE_FLAGS) && cd $(CURDIR)

run:
	@bash $(QEMU_SCRIPT)
