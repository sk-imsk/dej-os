BUILD_DIR := $(ROOT_DIR)/build/kernel
KERNEL_DIR := $(ROOT_DIR)/src/kernel

ASM := nasm
CC := gcc
CCFLAGS := -ffreestanding -fno-stack-protector -fno-pie -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-builtin -fno-omit-frame-pointer -mno-red-zone -m64 -mcmodel=kernel -std=gnu11 -g3 -mrdrnd -Wall -Wextra -Werror -O2 -mno-sse -I$(ROOT_DIR)/src/kernel/include -I$(ROOT_DIR)/src/kernel/arch -mgeneral-regs-only  -D__RC__
# bro too many args bro
LD := ld.lld

LIMINE_DIR := $(ROOT_DIR)/limine
LIMINE := $(LIMINE_DIR)/bin/limine

KERNEL := $(BUILD_DIR)/kernel.elf
IMAGE := $(BUILD_DIR)/dej-os.img
MNT := $(BUILD_DIR)/mnt

ARCH ?= x86

C_SOURCES := $(shell find $(KERNEL_DIR) \( \
    -path '$(KERNEL_DIR)/arch/$(ARCH)/*' -o \
    -not -path '$(KERNEL_DIR)/arch/*' \
    \) -name '*.c')

C_OBJECTS := $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))


S_SOURCES := $(shell find $(KERNEL_DIR) \( \
    -path '$(KERNEL_DIR)/arch/$(ARCH)/*' -o \
    -not -path '$(KERNEL_DIR)/arch/*' \
    \) -name '*.S')

S_OBJECTS := $(patsubst $(KERNEL_DIR)/%.S,$(BUILD_DIR)/%.o,$(S_SOURCES))

ifeq ($(MAKECMDGOALS),rc)
CCFLAGS := $(filter-out -D__RC__,$(CCFLAGS))
endif


.PHONY: always all kernel run clean rc

rc: all

all: kernel

kernel: $(KERNEL)


$(KERNEL): always $(C_OBJECTS) $(ASM_OBJECTS) $(S_OBJECTS) $(KERNEL_DIR)/linker.ld
	$(LD) -T $(KERNEL_DIR)/linker.ld -o $@ $(C_OBJECTS) $(ASM_OBJECTS) $(S_OBJECTS)




$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf64 $< -o $@

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	sudo umount $(MNT) 2>/dev/null || true
	rm -rf $(BUILD_DIR)
