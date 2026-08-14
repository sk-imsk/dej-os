BUILD_DIR := build
KERNEL_DIR := src/kernel

ASM := nasm
CC := gcc
CCFLAGS := -ffreestanding -fno-stack-protector -fno-pie -fno-asynchronous-unwind-tables -fno-unwind-tables -mno-red-zone -m64
LD := ld.lld

LIMINE_DIR := limine
LIMINE := $(LIMINE_DIR)/bin/limine

KERNEL := $(BUILD_DIR)/kernel.elf
IMAGE := $(BUILD_DIR)/dej-os.img
MNT := $(BUILD_DIR)/mnt

.PHONY: all kernel image run clean

all: image

kernel: $(KERNEL)

$(KERNEL): $(BUILD_DIR)/entry.o $(BUILD_DIR)/main.o $(KERNEL_DIR)/main.c $(KERNEL_DIR)/linker.ld
	$(LD) -T $(KERNEL_DIR)/linker.ld -o $@ $(BUILD_DIR)/entry.o $(BUILD_DIR)/main.o

$(BUILD_DIR)/entry.o:  $(KERNEL_DIR)/entry.asm
	mkdir -p $(BUILD_DIR)
	$(ASM) -f elf64 $< -o $@

$(BUILD_DIR)/main.o: $(KERNEL_DIR)/main.c
	$(CC) $(CCFLAGS) -c $< -o $@

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

clean:
	sudo umount $(MNT) 2>/dev/null || true
	rm -rf $(BUILD_DIR)
