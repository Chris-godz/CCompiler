# Compiler #
CC := clang
CXX := clang++
FLEX := flex
BISON := bison

# Project path #
PRJ_PATH := $(shell pwd)
BUILD_PATH := $(PRJ_PATH)/build
SRC_PATH := $(PRJ_PATH)/src
TARGET_NAME = compiler

# Source files #
FLEX_SRCS := $(patsubst $(SRC_PATH)/%.l, $(BUILD_PATH)/%.lex.c, $(shell find $(SRC_PATH) -name "*.l"))
BISON_SRCS := $(patsubst $(SRC_PATH)/%.y, $(BUILD_PATH)/%.tab.c, $(shell find $(SRC_PATH) -name "*.y"))
SRCS := $(shell find $(SRC_PATH) -name "*.c") $(FLEX_SRCS) $(BISON_SRCS)
OBJS := $(patsubst $(SRC_PATH)/%.c, $(BUILD_PATH)/%.o, $(SRCS))

# Flags #
INC_PATH = $(shell find $(SRC_PATH) -type d)
INC_PATH += $( INC_PATH : $(SRC_PATH)% = $(BUILD_PATH)% )
INC_FLAGS = $(addprefix -I, $(INC_PATH))
CFLAGS = $(INC_FLAGS) -std=c11

# CCompiler #
$(BUILD_PATH)/$(TARGET_NAME): $(SRCS) $(OBJS)
	$(CC) $(OBJS) -o $@ -I$(SRC_PATH)

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_PATH)/%.o: $(BUILD_PATH)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)


# Flex & Bison #
$(BUILD_PATH)/%.lex.c: $(SRC_PATH)/%.l
	mkdir -p $(dir $@)
	$(FLEX) -o $@ $<

$(BUILD_PATH)/%.tab.c: $(SRC_PATH)/%.y
	mkdir -p $(dir $@)
	$(BISON) -d -o $@ $<

# Clean #
.PHONY: clean
clean:
	rm -rf $(BUILD_PATH)