.POSIX:
.PHONY: build all clean full

# Define directories
BIN_DIR = bin
SRC_DIR = src
INC_DIR = inc

# Define files
SOURCES := $(shell find $(SRC_DIR) -name "*.c")			# All C source files under SRC_DIR
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)		# Transform .c to .o in BIN_DIR
TARGET ?= $(BUILD_DIR)/tool

# The tools to use
CC = gcc
CFLAGS = -std=c17 -O2 $(INCLUDE_DIR) -I$(INC_DIR)

build: $(TARGET)

full:
	@make -s clean
	@make -s all

all: $(TARGET)

# Build the target executable
$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBRARY)

# Compile all source files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Delete the output files
clean:
	rm -f $(BIN_DIR)/* $(TARGET)
