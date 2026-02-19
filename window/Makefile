RMW_DIR        ?= ../rose_memory_wrapper
RMW_INC_FLAGS  := -I$(RMW_DIR)/include
RMW_STATIC_LIB := librmw.a
RMW_LNK_FLAGS  := -L $(RMW_DIR)/build $(RMW_STATIC_LIB:lib%.a=-l%)

O_FILE := main.o

CC     := clang
CFLAGS := -Weverything -Wno-unsafe-buffer-usage -Werror \
	  -Wno-double-promotion -Wno-float-equal -pedantic \
	  -std=c99 $(RMW_INC_FLAGS)

PROG := testing_program

.PHONY: all clean

all: $(PROG)

$(PROG): $(O_FILE) $(RMW_STATIC_LIB)
	$(CC) $(CFLAGS) -o $@ $< $(RMW_LNK_FLAGS)

$(RMW_STATIC_LIB):
	make -C $(RMW_DIR) -B -j

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(O_FILE) $(PROG) compile_commands.json
