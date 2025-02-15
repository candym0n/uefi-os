.POSIX:
.PHONY: all clean full

ARCH            = $(shell uname -m | sed s,i[3456789]86,ia32,)

SO_FILE         = $(BIN_DIR)/so_file.so

SOURCES         = $(shell find $(SRC_DIR) -name "*.c")
OBJECTS         = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SOURCES))

CC              = gcc
LD              = ld
OBJCOPY         = objcopy

EFIINC          = /usr/include/efi
EFIINCS         = -I$(EFIINC) -I$(EFIINC)/$(ARCH) -I$(EFIINC)/protocol
LIBDIR          = /usr/lib
EFI_CRT_OBJS    = $(LIBDIR)/crt0-efi-$(ARCH).o
EFI_LDS         = $(LIBDIR)/elf_$(ARCH)_efi.lds
LIBS            = -lefi -lgnuefi $(shell $(CC) -print-libgcc-file-name)

CFLAGS          = $(EFIINCS) -std=c99 -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I$(INC_DIR)
ifeq ($(ARCH),x86_64)
	CFLAGS += -DEFI_FUNCTION_WRAPPER
endif

LDFLAGS         = -nostdlib -znocombreloc -T $(EFI_LDS) -shared -Bsymbolic -L$(LIBDIR) $(EFI_CRT_OBJS)

all: $(TARGET)

full:
	rm -f $(BIN_DIR)/*
	$(MAKE) $(TARGET)

# Build the target executable
$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@mkdir -p $(BIN_DIR)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(SO_FILE) $(LIBS)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel \
	           -j .rela -j .reloc --target=efi-app-$(ARCH) $(SO_FILE) $@

# Compile all source files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Delete the output files
clean:
	rm -rf $(BIN_DIR)/* $(SO_FILE) $(TARGET)
