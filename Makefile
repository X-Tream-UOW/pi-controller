# Compiler and flags
CC       := gcc
CFLAGS   := -Wall -O2 -Iinclude -fPIC
LDFLAGS  := -lgpiod -lm

# Directories
SRC_DIR  := c-src
OBJ_DIR  := build

# Discover sources
SRC_ALL      := $(wildcard $(SRC_DIR)/*.c)
MAIN_C       := $(SRC_DIR)/main.c
BIAS_MAIN_C  := $(SRC_DIR)/bias_main.c

# Common sources (everything except the two mains)
COMMON_SRC   := $(filter-out $(MAIN_C) $(BIAS_MAIN_C), $(SRC_ALL))
COMMON_OBJ   := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(COMMON_SRC))

# acq target (main.c + common, but NOT bias_main.c)
ACQ_NAME     := acq
ACQ_SRC      := $(COMMON_SRC) $(MAIN_C)
ACQ_OBJ      := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(ACQ_SRC))

# bias target (bias_main.c + common, but NOT main.c)
BIAS_NAME    := bias
BIAS_SRC     := $(COMMON_SRC) $(BIAS_MAIN_C)
BIAS_OBJ     := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(BIAS_SRC))

# Shared library (only common, ignore both mains)
LIB_NAME     := xtreamlib.so
LIB_OBJ      := $(COMMON_OBJ)

.PHONY: all acq bias lib cllean

# Build everything by default
all: acq bias lib

# Pattern rule for compiling .c to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# acq executable
acq: $(ACQ_OBJ)
	$(CC) $(CFLAGS) -o $(ACQ_NAME) $(ACQ_OBJ) $(LDFLAGS)

# bias executable
bias: $(BIAS_OBJ)
	$(CC) $(CFLAGS) -o $(BIAS_NAME) $(BIAS_OBJ) $(LDFLAGS)

# Shared library (xtream.so)
lib: $(LIB_NAME)
$(LIB_NAME): $(LIB_OBJ)
	$(CC) -shared -o $(LIB_NAME) $(LIB_OBJ) $(LDFLAGS)

# Clean
clean:
	rm -rf $(OBJ_DIR) $(ACQ_NAME) $(BIAS_NAME) $(LIB_NAME)
