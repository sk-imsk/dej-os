#pragma once
#include <dej/kernel.h>

void tss_init(void);
void load_tss(void);


typedef struct {
    uint32_t reserved0;
    uint64_t rsp0;       // Kernel stack pointer for Ring 3 -> Ring 0 transitions
    uint64_t rsp1;       // Unused in standard 64-bit OS
    uint64_t rsp2;       // Unused in standard 64-bit OS
    uint64_t reserved1;
    uint64_t ist[7];     // IST1 to IST7 (Dedicated interrupt stacks)
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base; // Offset to I/O permission bitmap (sizeof(tss_entry) if unused)
} __attribute__((packed)) tss_entry_t;



typedef  struct {
    tss_entry_t tss;
    uint8_t kernel_stack[16384] __attribute__((aligned(16)));
} tss_cpu;
