VERBOSE     := 1
V           ?=
ifeq ($(VERBOSE), 0)
	V := @
else
	V :=
endif

DEBUG       := 1
OPT_AND_DEB ?=
ifeq ($(DEBUG), 0)
	OPT_AND_DEB := -O3
else
	OPT_AND_DEB := -O0 -ggdb3
endif

CC          := gcc
CFLAGS      := -Wall -Wextra -Werror -ansi -pedantic $(OPT_AND_DEB)
BUILD_DIR   := build
LIB_NAME    := rosepetal
LIB_STATIC  := lib$(LIB_NAME).a
LIB_LINK    := -l$(LIB_NAME)
TEST_PROG   := $(LIB_NAME)_test

INC_DIRS    := include
INC_FLAGS   := $(INC_DIRS:%=-I%)
SRC_DIRS    := src
H_FILES     := $(foreach dir, $(INC_DIRS), $(wildcard $(dir)/*.h))
C_FILES     := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
O_FILES     := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

COL_COMPILE := setaf 7
COL_LINKER  := setaf 153
COL_VERBOSE := setaf 8
COL_SUCCESS := blink setaf 2 bold
COL_DEFAULT := sgr0

test: $(TEST_PROG)
	@tput $(COL_SUCCESS)
	@printf "Successfully built %s.\n" $<
	@tput $(COL_DEFAULT)

$(TEST_PROG): $(O_FILES)
	@tput $(COL_LINKER)
	@printf "[LD] %s -> %s\n" $^ $@

	@tput $(COL_VERBOSE)
	$(V)$(CC) $(CFLAGS) -o $@ $^
	@tput $(COL_DEFAULT)

$(BUILD_DIR)/%.o: %.c
	@tput $(COL_VERBOSE)
	$(V)mkdir -p $(dir $@)

	@tput $(COL_COMPILE)
	@printf "[CC] %s -> %s\n" $< $@

	@tput $(COL_VERBOSE)
	$(V)$(CC) $(CFLAGS) -o $@ -c $< $(INC_FLAGS)
	@tput $(COL_DEFAULT)

.PHONY: clean

clean:
	@tput $(COL_VERBOSE)
	$(V)rm -rf $(BUILD_DIR) $(LIB_STATIC) $(TEST_PROG)
	@tput $(COL_DEFAULT)
	@printf "Cleaned the previous build.\n"
