# Compiler and linker
CC := gcc

# Application name
APP := my_app

# Directories
SRC_DIR := .
OBJ_DIR := ./obj
BIN_DIR := ./bin

# Flags
CFLAGS := -Wall -g
LDFLAGS := 

# Source and object files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Default target
all: $(BIN_DIR)/$(APP)

# Link object files into a binary
$(BIN_DIR)/$(APP): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	@rm -rf $(BIN_DIR) $(OBJ_DIR)

# Phony targets
.PHONY: all clean
