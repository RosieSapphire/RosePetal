VERBOSE     := 0
V           ?=
ifeq ($(VERBOSE), 0)
	V := @
else
	V :=
endif

DEBUG       := 0
OPT_AND_DEB ?=
ifeq ($(DEBUG), 0)
	OPT_AND_DEB := -O3
else
	OPT_AND_DEB := -O0 -ggdb3 -DDEBUG_ENABLED
endif

CC          := gcc
CFLAGS      := -Wall -Wextra -Werror -ansi -pedantic $(OPT_AND_DEB)
BUILD_DIR   := build
LIB_NAME    := rosepetal
LIB_STATIC  := lib$(LIB_NAME).a
LIB_LINK    := -l$(LIB_NAME)
TEST_PROG   := $(LIB_NAME)_test

SRC_DIRS    := src
C_FILES     := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
O_FILES     := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

test: $(TEST_PROG)

$(TEST_PROG): $(O_FILES)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR) $(LIB_STATIC) $(TEST_PROG)
