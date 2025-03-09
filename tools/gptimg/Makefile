.POSIX:
.PHONY: build all clean full

# Define directories
BIN_DIR = bin
SRC_DIR = src

# Define files
SOURCES := $(shell find $(SRC_DIR) -name "*.c")			# All C source files under SRC_DIR
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)		# Transform .c to .o in BIN_DIR
TARGET = build/gptimg

# The tools to use
CC = @gcc
CFLAGS = -std=c17 -O2

build: $(TARGET)

full:
	@make -s clean
	@make -s all

all: $(TARGET)

# Build the target executable
$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@mkdir -p $(dir $@)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile all source files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(BIN_DIR)			# Create BIN_DIR if it doesn't exist
	$(CC) $(CFLAGS) -c $< -o $@		# Compile it to a .o object file

# Delete the output files
clean:
	rm -f $(BIN_DIR)/*
