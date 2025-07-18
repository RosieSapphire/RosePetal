VERBOSE      := 0
ifeq ($(VERBOSE), 0)
	V := @
else
	V :=
endif

DEBUG        := 0
OPT_AND_DEB  ?=
ifeq ($(DEBUG), 0)
	OPT_AND_DEB := -O3
else
	OPT_AND_DEB := -O0 -ggdb3
endif

CC           := gcc
CFLAGS       := -Wall -Wextra -Werror -ansi -pedantic $(OPT_AND_DEB)
AR           := ar
AR_FLAGS     := -rcs

BUILD_DIR    := build
LIB_NAME     := rosepetal
LIB_STATIC   := lib$(LIB_NAME).a
LIB_LINK     := -L. -l$(LIB_NAME)
TEST_PROG    := $(LIB_NAME)_test

INC_DIR      := include
INC_FLAGS    := $(INC_DIR:%=-I%)
SRC_DIR_RP   := src/rosepetal
HEADER       := $(INC_DIR)/rosepetal.h
C_FILES_RP   := $(foreach dir, $(SRC_DIR_RP), $(wildcard $(dir)/*.c))
O_FILES_RP   := $(C_FILES_RP:%.c=$(BUILD_DIR)/%.o)
TEST_C       := src/test.c
TEST_O       := $(BUILD_DIR)/src/test.o

COL_COMPILE  := setaf 3
COL_LINKER   := setaf 153
COL_LIB_MAKE := setaf 12
COL_VERBOSE  := setaf 8
COL_SUCCESS  := blink setaf 2 bold
COL_DEFAULT  := sgr0

lib: $(LIB_STATIC)

$(LIB_STATIC): $(O_FILES_RP)
	@tput $(COL_VERBOSE)
	$(V)$(AR) $(AR_FLAGS) $@ $^

	@tput $(COL_LIB_MAKE)
	@printf "[AR] %s -> %s\n" "$^" "$<"
	@tput $(COL_DEFAULT)

test: $(TEST_PROG)
	@tput $(COL_SUCCESS)
	@printf "Successfully built '%s'.\n" $<
	@tput $(COL_DEFAULT)

$(TEST_PROG): $(LIB_STATIC) $(TEST_O)
	@tput $(COL_LINKER)
	@printf "[LD] %s -> %s\n" "$(filter-out $<, $^)" $@

	@tput $(COL_VERBOSE)
	$(V)$(CC) $(CFLAGS) -o $@ $^ $(LIB_LINK)
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
	$(V)rm -rf $(BUILD_DIR) $(LIB_STATIC) $(TEST_PROG) compile_commands.json
	@tput $(COL_DEFAULT)
	@printf "Cleaned the previous build.\n"
