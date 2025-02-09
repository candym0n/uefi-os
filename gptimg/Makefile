.POSIX:
.PHONY: build all clean full

# Define directories
BIN_DIR = bin
SRC_DIR = src
INC_DIR = inc

# Define files
SOURCES := $(shell find $(SRC_DIR) -name "*.c")			# All C source files under SRC_DIR
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)		# Transform .c to .o in BIN_DIR
TARGET = $(BIN_DIR)/gptimg

# The tools to use
CC = @gcc
CFLAGS = -std=c17 -O2 -I inc

build: $(TARGET)

full:
	@make -s clean
	@make -s all

all: $(TARGET)
	$(TARGET) create test.img --size 1G
	$(TARGET) add-partition test.img --type efi --name "EFI System" --size 33M
	$(TARGET) add-partition test.img --type basic-data --name "Operating System" --size 1M
	$(TARGET) format test.img --partition 0 --cluster-size 512

# Build the target executable
$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@mkdir -p $(BIN_DIR)			# Create BIN_DIR if it doesn't exist
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile all source files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(BIN_DIR)			# Create BIN_DIR if it doesn't exist
	$(CC) $(CFLAGS) -c $< -o $@		# Compile it to a .o object file

# Delete the output files
clean:
	rm -f $(BIN_DIR)/*
