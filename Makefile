# Compiler and flags
CC       := gcc
CFLAGS   := -Wall -O2 -Iinclude -fPIC
LDFLAGS  := -lgpiod -lm

# Directories
SRC_DIR  := c-src
OBJ_DIR  := build

# Sources and objects
SRC      := $(wildcard $(SRC_DIR)/*.c)
OBJ      := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Main target
TARGET   := master

# Library target (exclude main.c and bias_main.c)
LIB_NAME := libacquisition.so
LIB_SRC  := $(filter-out $(SRC_DIR)/main.c $(SRC_DIR)/bias_main.c, $(SRC))
LIB_OBJ  := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SRC))

# Bias target
BIAS_NAME := bias
BIAS_SRC  := $(SRC_DIR)/bias_main.c $(SRC_DIR)/bias_api.c $(SRC_DIR)/bias_control.c
BIAS_OBJ  := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(BIAS_SRC))

.PHONY: all clean lib bias

# Default build
all: $(TARGET)

# Main program
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Pattern rule for compiling .c to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Shared library
lib: $(LIB_NAME)
$(LIB_NAME): $(LIB_OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

# Bias program
bias: $(BIAS_NAME)
$(BIAS_NAME): $(BIAS_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(LIB_NAME) $(BIAS_NAME)
