BUILD_DIR := build
KERNEL_DIR := src/kernel

ASM := nasm
CC := gcc
CCFLAGS := -ffreestanding -fno-stack-protector -fno-pie -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-builtin -fno-omit-frame-pointer -mno-red-zone -m64 -mcmodel=kernel -std=gnu11 -g3 -mrdrnd -Wall -Wextra -Werror -O2 -mno-sse -I./src/kernel/include -I./src/kernel/arch
# bro too many args bro
LD := ld.lld

LIMINE_DIR := limine
LIMINE := $(LIMINE_DIR)/bin/limine

KERNEL := $(BUILD_DIR)/kernel.elf
IMAGE := $(BUILD_DIR)/dej-os.img
MNT := $(BUILD_DIR)/mnt

ARCH ?= x86

C_SOURCES := $(shell find $(KERNEL_DIR) -name '*.c' \
    -not -path '$(KERNEL_DIR)/arch/*' \
    -o -path '$(KERNEL_DIR)/arch/$(ARCH)/*.c')

C_OBJECTS := $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))

ASM_SOURCES := $(shell find $(KERNEL_DIR) -name '*.asm' \
    -not -path '$(KERNEL_DIR)/arch/*' \
    -o -path '$(KERNEL_DIR)/arch/$(ARCH)/*.asm')

ASM_OBJECTS := $(patsubst $(KERNEL_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASM_SOURCES))
.PHONY: all kernel image run clean

all: always image

kernel: $(KERNEL)


$(KERNEL): $(C_OBJECTS) $(ASM_OBJECTS) $(KERNEL_DIR)/linker.ld
	$(LD) -T $(KERNEL_DIR)/linker.ld -o $@ $(C_OBJECTS) $(ASM_OBJECTS)

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	$(CC) $(CCFLAGS) -c $< -o $@
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.asm
	$(ASM) -f elf64 $< -o $@

image: $(IMAGE)

$(IMAGE): $(KERNEL) limine.conf

	rm -f $@

	dd if=/dev/zero of=$@ bs=1M count=64

	cd $(LIMINE_DIR)    && make

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
	sudo cp test.txt $(MNT)/test.txt

	sudo umount $(MNT)
	sudo losetup -d $$(cat $(BUILD_DIR)/loopdev)
	rm -f $(BUILD_DIR)/loopdev

	$(LIMINE) bios-install $@
run: image
	qemu-system-x86_64 -drive format=raw,file=$(IMAGE)

always:
	mkdir -p build/arch/x86
	mkdir -p build/drivers/keyboard
	mkdir -p build/interrupt
	mkdir -p build/memory
	mkdir -p build/x86/
	mkdir -p build/cpu/cpu1/
	mkdir -p build/drivers/disk
	mkdir -p build/drivers/framebuffer
	mkdir -p build/include/dej
	mkdir -p build/cpu/cpu2

clean:
	sudo umount $(MNT) 2>/dev/null || true
	rm -rf $(BUILD_DIR)
