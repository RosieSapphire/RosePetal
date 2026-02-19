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

# C stuff
CC     := clang
CFLAGS := -Weverything -Wno-unsafe-buffer-usage -Wno-format-nonliteral \
	  -pedantic -std=c99 -I../include \
	  -DRP_MEM_MAX_ALLOC_CNT=$(RP_MEM_MAX_ALLOC_CNT) \
	  -DRP_MEM_DEBUG_PRINTF=$(RP_MEM_DEBUG_PRINTF)

# Configuring debug mode
ifeq ($(DEBUG_MODE), 0)
	CFLAGS += --no-warnings -O3 -g0 -DNDEBUG
else
	CFLAGS += -Werror -O0 -ggdb3 -D_DEBUG
endif


# The library itself
build/librp_memory.a: build/memory.o
	@mkdir -p $(dir $@)
	@ar rcs $@ $^
	@echo "Memory Submodule Library built successfully!"

# Test program
test: build/test.elf

build/test.elf: build/test.o build/librp_memory.a
	@$(CC) $(CFLAGS) -o $@ $< -L build -lrp_memory
	@echo "Memory Submodule test built successfully!"

# Compiling to object files
build/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -o $@ -c $<
	@echo "    [CC] $@ <- $<"

# Clean any previous build (if applicable)
clean:
	@rm -rf build compile_commands.json
	@echo "Cleaning Memory Submodule..."

.PHONY: test clean
