BUILD_DIR := build
KERNEL_DIR := src/kernel

ASM := nasm
CC := gcc
CCFLAGS := -ffreestanding -fno-stack-protector -fno-pie -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-builtin -mno-red-zone -m64 -mcmodel=kernel -std=c11 -Wall -Werror -g
# bro too many args bro
LD := ld.lld

LIMINE_DIR := limine
LIMINE := $(LIMINE_DIR)/bin/limine

KERNEL := $(BUILD_DIR)/kernel.elf
IMAGE := $(BUILD_DIR)/dej-os.img
MNT := $(BUILD_DIR)/mnt

C_SOURCES := $(shell find $(KERNEL_DIR) -name '*.c')
C_OBJECTS := $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))

ASM_SOURCES := $(shell find $(KERNEL_DIR) -name '*.asm')
ASM_OBJECTS := $(patsubst $(KERNEL_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASM_SOURCES))
.PHONY: all kernel image run clean

all: always image

kernel: $(KERNEL)


$(KERNEL): $(C_OBJECTS) $(ASM_OBJECTS) $(KERNEL_DIR)/main.c $(KERNEL_DIR)/linker.ld
	$(LD) -T $(KERNEL_DIR)/linker.ld -o $@ $(C_OBJECTS) $(ASM_OBJECTS)

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	$(CC) $(CCFLAGS) -c $< -o $@
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.asm
	$(ASM) -f elf64 $< -o $@

image: $(IMAGE)

$(IMAGE): $(KERNEL) limine.conf

	rm -f $@

	dd if=/dev/zero of=$@ bs=1M count=64

	sgdisk -Z $@
	sgdisk -n 1:2048:4095 -t 1:ef02 $@
	sgdisk -n 2:4096:0 -t 2:ef00 $@

	sudo losetup --find --partscan --show $@ > $(BUILD_DIR)/loopdev
	sudo partprobe $$(cat $(BUILD_DIR)/loopdev)

	sudo mkfs.fat -F 32 $$(cat $(BUILD_DIR)/loopdev)p2

	mkdir -p $(MNT)
	sudo mount $$(cat $(BUILD_DIR)/loopdev)p2 $(MNT)

	sudo mkdir -p $(MNT)/boot/limine
	sudo cp $(KERNEL) $(MNT)/boot/kernel.elf
	sudo cp limine.conf $(MNT)/limine.conf
	sudo cp $(LIMINE_DIR)/bin/limine-bios.sys $(MNT)/boot/limine/limine-bios.sys

	sudo umount $(MNT)
	sudo losetup -d $$(cat $(BUILD_DIR)/loopdev)
	rm -f $(BUILD_DIR)/loopdev

	$(LIMINE) bios-install $@
run: image
	qemu-system-x86_64 -drive format=raw,file=$(IMAGE)

always:
	mkdir -p build/
	mkdir -p build/keyboard
	mkdir -p build/interrupt
	mkdir -p build/memory
	mkdir -p build/x86/

clean:
	sudo umount $(MNT) 2>/dev/null || true
	rm -rf $(BUILD_DIR)
