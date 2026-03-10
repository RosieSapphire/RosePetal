DEBUG ?= 1

RANDOM_LOG ?=

MEM_LOG          ?= 1
MEM_LOG_END_ONLY ?=
MEM_LOG_VERBOSE  ?=

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
	       -Wno-reserved-identifier -Wno-variadic-macros \
	       -Wno-bad-function-cast -Wno-unsafe-buffer-usage

CC     := clang-20
CFLAGS := -Wall -Wextra -Weverything -Werror -pedantic -ansi $(WARN_IGNORE)

ifdef DEBUG
	CFLAGS += -O0 -ggdb3 -D_DEBUG
else
	CFLAGS += -O3 -g0 -DNDEBUG
endif

ifdef RANDOM_LOG
	CFLAGS += -DRANDOM_DEBUG
endif

ifdef MEM_LOG
	CFLAGS += -DALLOCATOR_LOG
	ifdef MEM_LOG_VERBOSE
		CFLAGS += -DALLOCATOR_LOG_VERBOSE
	endif
else
	ifdef MEM_LOG_END_ONLY
		CFLAGS += -DALLOCATOR_LOG_END_ONLY
	endif
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
