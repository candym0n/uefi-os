.POSIX:
.PHONY: all clean full

ifndef INCLUDE_DIR
INCLUDE_DIR = -I ../include
endif

ARCH            = $(shell uname -m | sed s,i[3456789]86,ia32,)

CC              = gcc
LD              = ld
OBJCOPY         = objcopy
AR              = ar

BIN_DIR         = bin
SRC_DIR         = src

SOURCES         = $(shell find $(SRC_DIR) -name "*.c")
OBJECTS         = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SOURCES))

TARGET          = build/liblibrary.a

LIBS            = -lefi -lgnuefi

CFLAGS          = $(INCLUDE_DIR) -fno-stack-protector -fpic -fshort-wchar -mno-red-zone

LDFLAGS         = -nostdlib -znocombreloc -shared -Bsymbolic
ARFLAGS         = rcs

all: $(TARGET)

full:
	rm -f $(BIN_DIR)/*
	$(MAKE) $(TARGET)

# Build the target archive library
$(TARGET): $(OBJECTS)
	@echo "Archiving..."
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^

# Compile all source files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Delete the output files
clean:
	rm -rf $(BIN_DIR)/* $(TARGET)
