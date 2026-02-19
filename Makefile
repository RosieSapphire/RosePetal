# Convinience for building and cleaning everything
RP_DIR_MEMORY           := memory
RP_FILE_MEMORY_A        := $(RP_DIR_MEMORY)/build/librp_memory.a
RP_FILE_MEMORY_TEST_ELF := $(RP_DIR_MEMORY)/build/test.elf

RP_DIR_WINDOW           := window
RP_FILE_WINDOW_A        := $(RP_DIR_WINDOW)/build/librp_window.a
RP_FILE_WINDOW_TEST_ELF := $(RP_DIR_WINDOW)/build/test.elf

.PHONY: all all_test memory memory_test window window_test clean

all: memory window

all_test: memory_test window_test

# Memory
memory:
	@echo "Building Memory Library..."
	@make -j -C $(RP_DIR_MEMORY)

memory_test:
	@echo "Building Memory Test..."
	@make test -j -C $(RP_DIR_MEMORY)

# Window
window:
	@echo "Building Window Library..."
	@make -j -C $(RP_DIR_WINDOW)

window_test:
	@echo "Building Window Test..."
	@make test -j -C $(RP_DIR_WINDOW)

clean:
	@echo "Wiping Memory Submodule"
	@make clean -C $(RP_DIR_MEMORY)
	@echo "Wiping Window Submodule"
	@make clean -C $(RP_DIR_WINDOW)
