#include <dej/kernel.h>


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

static tss_entry_t g_tss;

static uint8_t kernel_stack[16384] __attribute__((aligned(16)));


struct tss_descriptor {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;      // 0x89 = Present, Ring 0, 64-bit Available TSS
    uint8_t  flags_limit; // Limit high + flags
    uint8_t  base_high;
    uint32_t base_upper;  // Upper 32 bits of 64-bit base address
    uint32_t reserved;
} __attribute__((packed));

extern char __tss_descriptor_ptr[];
extern void load_tss(void);

void write_tss_descriptor(void) {
    // 1. Clear TSS
    for (size_t i = 0; i < sizeof(g_tss); i++) {
        ((uint8_t *)&g_tss)[i] = 0;
    }

    // 2. Point rsp0 to top of allocated kernel stack
    g_tss.rsp0 = (uint64_t)&kernel_stack[sizeof(kernel_stack)];
    g_tss.iomap_base = sizeof(g_tss); // Disable I/O permission bitmap

    // 3. Populate the 16-byte TSS descriptor in the GDT
    uint64_t base = (uint64_t)&g_tss;
    uint32_t limit = sizeof(g_tss) - 1;

    // First 8 bytes
    uint64_t desc_low = 0;
    desc_low |= (limit & 0xFFFF);
    desc_low |= (base & 0xFFFF) << 16;
    desc_low |= ((base >> 16) & 0xFF) << 32;
    desc_low |= (0x89ULL) << 40; // Present, Ring 0, 64-bit Available TSS
    desc_low |= (uint64_t)((limit >> 16) & 0x0F) << 48;
    desc_low |= ((base >> 24) & 0xFF) << 56;

    // Upper 8 bytes (holds high 32 bits of 64-bit base address)
    uint64_t desc_high = (base >> 32);

    // Write into GDT space
    ((uint64_t *)__tss_descriptor_ptr)[0] = desc_low;
    ((uint64_t *)__tss_descriptor_ptr)[1] = desc_high;
}

void tss_init(void) {
    write_tss_descriptor();
    load_tss(); // Executes ltr 0x48
}
