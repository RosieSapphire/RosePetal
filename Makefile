DEBUG ?= 1
DEBUG_VERBOSE ?=

# Library
LIB_NAME := memory_allocator
LIB_H    := $(LIB_NAME).h

# Test
PROG_NAME := test
PROG_ELF  := $(PROG_NAME).elf
TEST_C    := $(PROG_NAME).c
TEST_O    := $(TEST_C:%.c=%.o)

# Helper
BUILD_FILES := $(PROG_ELF) $(TEST_O) $(LIB_O) *.json .cache/

WARN_IGNORE := -Wno-format-nonliteral -Wno-reserved-macro-identifier \
	       -Wno-reserved-identifier -Wno-unsafe-buffer-usage \
	       -Wno-variadic-macros -Wno-bad-function-cast

CC     := clang
CFLAGS := -Wall -Wextra -Weverything -Werror -pedantic -ansi $(WARN_IGNORE)

ifdef DEBUG
	CFLAGS += -O0 -ggdb3 -D_DEBUG -DALLOCATOR_DEBUG -DRANDOM_DEBUG
		  # -fsanitize=address,undefined,leak,null
	ifdef DEBUG_VERBOSE
		CFLAGS += -DALLOCATOR_DEBUG_VERBOSE
	endif
else
	CFLAGS += -O3 -g0 -DNDEBUG
endif

all: $(PROG_ELF)

$(PROG_ELF): $(LIB_O) $(TEST_O)
	@echo "    [LD] $@"
	@$(CC) $(CFLAGS) -o $@ $^

$(TEST_O): $(TEST_C)
	@echo "    [CC] $<"
	@$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@echo "Cleaning previous build..."
	@rm -rf $(BUILD_FILES)
