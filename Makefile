# Shush Makefile
MAKEFLAGS += --silent

# Directories used for compiling (overridable)
BUILD_DIR ?= $(abspath build)
INCLUDE_DIR ?= $(abspath include)

# Size of the target image (overridable)
SIZE ?= 1G

# Target image
IMAGE := $(BUILD_DIR)/test.img

# Tool directories
GPTIMG_DIR   := $(abspath tools/gptimg)
FSFORMAT_DIR := $(abspath tools/fsformat)

# OS component directories
BOOT_DIR := $(abspath boot)
LIB_DIR  := $(abspath lib)

# Scripts
BUILD_SCRIPT := scripts/build.sh
QEMU_SCRIPT  := scripts/qemu.sh

# Export only what scripts actually need
export SIZE IMAGE BUILD_DIR

# Common flags passed to sub-makes
MAKE_FLAGS := INCLUDE_DIR=-I$(INCLUDE_DIR) LIBRARY="-L$(LIB_DIR)/build/ -llibrary" BUILD_DIR=$(BUILD_DIR)

# Standard variables
RM      ?= rm -rf
MKDIR_P ?= mkdir -p

.PHONY: all clean run image bootloader tools lib

all: image

clean:
	@echo "Cleaning up..."
	@$(MAKE) -C "$(BOOT_DIR)" clean
	@$(MAKE) -C "$(GPTIMG_DIR)" clean
	@$(MAKE) -C "$(FSFORMAT_DIR)" clean
	@$(MAKE) -C "$(LIB_DIR)"  clean
	@$(RM) -rf "$(BUILD_DIR)"

bootloader: lib
	@echo "Building bootloader..."
	@$(MAKE) -C "$(BOOT_DIR)" all $(MAKE_FLAGS)

tools: lib
	@echo "Building tools..."
	@$(MAKE) -C "$(GPTIMG_DIR)"   all $(MAKE_FLAGS)
	@$(MAKE) -C "$(FSFORMAT_DIR)" all $(MAKE_FLAGS)

image: bootloader tools
	@echo "Creating image..."
	@$(MKDIR_P) "$(BUILD_DIR)"
	@bash "$(BUILD_SCRIPT)"

lib:
	@echo "Building library..."
	@$(MAKE) -C "$(LIB_DIR)" all $(MAKE_FLAGS)

run:
	@bash "$(QEMU_SCRIPT)"
