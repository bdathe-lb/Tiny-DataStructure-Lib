# --------------------------
# Automated build script.
# --------------------------

# The name of the final executable
TARGET_EXEC := test

# Directory definition
SRC_DIR := src
INC_DIR := include
TEST_DIR := tests
BUILD_DIR := build

# Compiler and related options
CC := gcc
CFLAGS := -I$(INC_DIR) -Wall -Wextra -g -O0 -MMD -MP
LDFLAGS := 

# Automated inference
SRC_SRCS  := $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c) 
SRCS := $(SRC_SRCS) $(TEST_SRCS)
OBJS := \
	$(SRC_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o) \
	$(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Compilation rules
all: $(BUILD_DIR)/$(TARGET_EXEC)

# Linking
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@echo "Linking target: $@"
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build completed!"

# Compilation
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Create build/
$(BUILD_DIR):
	@mkdir -p $@

# Import dependence file
-include $(DEPS)

# Run
run: all
	@./$(BUILD_DIR)/$(TARGET_EXEC)

# Debug
debug: all
	@gdb -tui ./$(BUILD_DIR)/$(TARGET_EXEC)

# Clean
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean completed!"

.PHONY: all run debug clean
