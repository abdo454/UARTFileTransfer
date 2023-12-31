# Compiler
CC := gcc

# Application name
APP := master

# Flags
CFLAGS := -Wall -g -MMD  # Add -MMD to generate dependency files
LDFLAGS :=

# Directories
SRC_DIR := .
SLAVE_DIR := ../Slave
OBJ_DIR := ./obj
BIN_DIR := ./bin

# Source and object files
SRCS := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SLAVE_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c)) \
        $(patsubst $(SLAVE_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SLAVE_DIR)/*.c))
DEPS := $(OBJS:.o=.d)  # Add this line for dependencies
# Include paths
INCLUDES := -I$(SRC_DIR) -I$(SLAVE_DIR)

# Default target
all: $(BIN_DIR)/$(APP)

# Link object files into a binary
$(BIN_DIR)/$(APP): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(SLAVE_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

-include $(DEPS)  # Include the dependency files

# Clean up
clean:
	@rm -rf $(BIN_DIR) $(OBJ_DIR) $(DEPS)

# Phony targets
.PHONY: all clean
