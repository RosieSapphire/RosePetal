# Config
DEBUG_MODE          := 1
RP_MEM_DEBUG_PRINTF := 1 # TODO: Write this to a log file instead!

# The maximum heap allocations the internal heap
# buffer tracker can handle before crashing.
#
# The "default", I guess, should probably be 4096
# 
# NOTE: THIS MUST BE A POWER OF 2!
RP_MEM_MAX_ALLOC_CNT := 4096

# General
DIR_BUILD      := build
DIR_RP_ROOT    ?= ..
DIR_RP_INCLUDE := $(DIR_RP_ROOT)/include

# Library-specific
LIB_FILES_C := memory.c
LIB_FILES_O := $(LIB_FILES_C:%.c=$(DIR_BUILD)/%.o)
LIB_FILE_A  := $(DIR_BUILD)/librmw.a

# Test-specific
TEST_FILE_ELF := $(DIR_BUILD)/test.elf
TEST_FILE_C   := test.c
TEST_FILE_O   := $(DIR_BUILD)/test.o

# For linking the library to the test program
TEST_FLAGS_LINK := -L $(DIR_BUILD) -l$(LIB_FILE_A:$(DIR_BUILD)/lib%.a=%)

# C stuff
CC     := clang
CFLAGS := -Weverything -Wno-unsafe-buffer-usage -Wno-format-nonliteral \
	  -pedantic -std=c99 -I$(DIR_RP_INCLUDE) \
	  -DRP_MEM_MAX_ALLOC_CNT=$(RP_MEM_MAX_ALLOC_CNT) \
	  -DRP_MEM_DEBUG_PRINTF=$(RP_MEM_DEBUG_PRINTF)

# Configuring debug mode
ifeq ($(DEBUG_MODE), 0)
	CFLAGS += --no-warnings -O3 -g0 -DNDEBUG
else
	CFLAGS += -Werror -O0 -ggdb3 -D_DEBUG
endif


# The library itself
$(LIB_FILE_A): $(LIB_FILES_O)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

# Test program
test: $(TEST_FILE_ELF)

$(TEST_FILE_ELF): $(TEST_FILE_O) $(LIB_FILE_A)
	$(CC) $(CFLAGS) -o $@ $< $(TEST_FLAGS_LINK)

# Compiling to object files
$(DIR_BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Clean any previous build (if applicable)
clean:
	rm -rf $(DIR_BUILD) compile_commands.json

.PHONY: test clean
