DEBUG ?= 1

# Libraries
RP_RANDOM_H := rp_random.h
RP_MEMORY_H := rp_memory.h

# Test
PROG_NAME := test
PROG_ELF  := $(PROG_NAME).elf
TEST_C    := $(PROG_NAME).c
TEST_O    := $(TEST_C:%.c=%.o)

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

all: $(PROG_ELF)

$(PROG_ELF): $(TEST_O) $(RP_MEMORY_H)
	@echo "    [LD] $@"
	@$(CC) $(CFLAGS) -o $@ $<

$(TEST_O): $(TEST_C)
	@echo "    [CC] $<"
	@$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@echo "Cleaning previous build..."
	@rm -rf *.elf *.o *.json .cache/
