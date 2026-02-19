CC     := clang
CFLAGS := -Weverything -Wno-unsafe-buffer-usage -Werror -Wno-double-promotion \
	  -Wno-float-equal -Wno-format-nonliteral -pedantic \
	  -std=c99 -I../include

.PHONY: test clean

build/librp_window.a: build/window.o
	@ar rcs -o $@ $<
	@echo "    [AR] $@ <- $^"

test: build/test.elf

build/test.elf: build/test.o build/librp_window.a
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -o $@ $< -lglfw -L build -lrp_window
	@echo "    [LD] $@ <- $^"

build/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -o $@ -c $<
	@echo "    [CC] $< -> $@"

clean:
	rm -rf build compile_commands.json
