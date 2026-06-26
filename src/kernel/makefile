BUILD_DIR?=build
ASM?=nasm



.PHONY: all kernel clean

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: main.asm
	$(ASM) main.asm -f bin -o $(BUILD_DIR)/kernel.bin


clean:
	rm -f $(BUILD_DIR)/kernel.bin
