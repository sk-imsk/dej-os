BUILD_DIR ?= build/
SRC_DIR ?= src
ASM ?= nasm


$(BUILD_DIR)/stage2.bin: main.asm
	$(ASM)  $< -f bin  -o $@



always: $(BUILD_DIR)/stage2.bin

clean:
	rm -f $(BUILD_DIR)/stage2.bin
