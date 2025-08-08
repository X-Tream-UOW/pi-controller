# Compiler and flags
CC := gcc
CFLAGS := -Wall -O2 -Iinclude
LDFLAGS := -lgpiod -lm

# Auto-detect sources/objects
SRC_DIR := src
OBJ_DIR := build

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

TARGET := master

# Targets
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# New target
LIB_NAME := libacquisition.so
LIB_SRC := $(filter-out $(SRC_DIR)/main.c, $(SRC))  # excludes main.c
LIB_OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SRC))

.PHONY: lib
lib: $(LIB_NAME)

$(LIB_NAME): $(LIB_OBJ)
	$(CC) -shared -fPIC -o $@ $^ $(LDFLAGS)
