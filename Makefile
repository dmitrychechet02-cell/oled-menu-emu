# OLED 128x64 menu emulator — сборка ядра на хосте (тесты) и в WebAssembly (эмулятор).
#   make test            — все unit-тесты
#   make test T=menu     — только тесты, в имени которых есть "menu"
#   make wasm            — web/build/emu.wasm
#   make lint            — clang-format --dry-run + сборка с -Werror (host и wasm)
#   make format          — clang-format -i по всему C-коду
#   make fonts           — перегенерировать шрифт и иконки из tools/gen_font.py

CC        ?= gcc
CLANG     ?= clang
PYTHON    ?= python3
BUILD     := build
WEB_BUILD := web/build

WARN   := -Wall -Wextra -Werror -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wpedantic
CSTD   := -std=c11

CORE_SRC := $(wildcard src/gfx/*.c src/fonts/*.c src/menu/*.c src/app/*.c)
TEST_SRC := $(wildcard tests/unit/*.c) src/hal/ssd1309/ssd1309.c
WASM_SRC := $(CORE_SRC) $(wildcard src/hal/web/*.c)
FW_SRC   := $(CORE_SRC) $(wildcard src/hal/ssd1309/*.c)

C_FILES  := $(shell find src tests/unit -name '*.c' -o -name '*.h')

.PHONY: all test wasm lint format fonts firmware-check clean

all: test wasm

$(BUILD)/unit_tests: $(CORE_SRC) $(TEST_SRC) $(wildcard src/*/*.h src/*/*/*.h tests/unit/*.h)
	@mkdir -p $(BUILD)
	$(CC) $(CSTD) $(WARN) -Wno-missing-prototypes -g -O0 -fsanitize=address,undefined \
		-o $@ $(CORE_SRC) $(TEST_SRC)

test: $(BUILD)/unit_tests
	./$(BUILD)/unit_tests $(T)

# Ядро без libc: ровно так оно будет собираться под микроконтроллер.
$(WEB_BUILD)/emu.wasm: $(WASM_SRC) $(wildcard src/*/*.h src/*/*/*.h)
	@mkdir -p $(WEB_BUILD)
	$(CLANG) --target=wasm32 $(CSTD) $(WARN) -O2 -ffreestanding -nostdlib \
		-Wl,--no-entry -Wl,--strip-debug -o $@ $(WASM_SRC)

wasm: $(WEB_BUILD)/emu.wasm

# Проверка, что код для платы (драйвер SSD1309 + ядро) компилируется freestanding.
firmware-check:
	@mkdir -p $(BUILD)
	$(CLANG) --target=armv7m-none-eabi -mthumb $(CSTD) $(WARN) -O2 -ffreestanding \
		-fsyntax-only $(FW_SRC)

lint: firmware-check wasm
	clang-format --dry-run --Werror $(C_FILES)
	@echo "lint: ok"

format:
	clang-format -i $(C_FILES)

fonts:
	$(PYTHON) tools/gen_font.py
	clang-format -i src/fonts/font5x7_data.c src/fonts/icons_data.c src/fonts/icons.h

clean:
	rm -rf $(BUILD) $(WEB_BUILD)
