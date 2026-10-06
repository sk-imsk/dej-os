ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
export ROOT_DIR

SUBDIRS := kernel re programs
BUILD_DIR := build
KERNEL_DIR := src/kernel

LIMINE_DIR := limine
LIMINE := $(LIMINE_DIR)/bin/limine

KERNEL := $(BUILD_DIR)/kernel/kernel.elf
IMAGE := $(BUILD_DIR)/dej-os.img
MNT := $(BUILD_DIR)/mnt

ARCH ?= x86


.PHONY: all clean $(SUBDIRS)

all: kernel re programs image

kernel:
	$(MAKE) -C src/$@

re:
	$(MAKE) -C src/$@

programs:
	$(MAKE) -C src/$@


image: $(IMAGE)

$(IMAGE): kernel $(ROOT_DIR)/limine.conf

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
	sudo cp src/programs/out.bin $(MNT)/dih.bin


	sudo mkdir -p $(MNT)/EFI/BOOT
	sudo cp $(LIMINE_DIR)/bin/BOOTX64.EFI $(MNT)/EFI/BOOT/BOOTX64.EFI

	sudo umount $(MNT)
	sudo losetup -d $$(cat $(BUILD_DIR)/loopdev)
	rm -f $(BUILD_DIR)/loopdev

	$(LIMINE) bios-install $@


clean:
	sudo umount -l $(MNT) 2>/dev/null || true
	rm -rf $(BUILD_DIR)
