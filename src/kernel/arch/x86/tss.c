#include <dej/kernel.h>
#include <dej/percpu.h>
#include <x86/tss.h>


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


tss_cpu tssforcpus[32];


extern char __tss_descriptor_ptr[];
extern void load_tss(void);

void write_tss_descriptor(void) {


    tss_cpu * percpu_tss = (tss_cpu *)percpu_readptr(tss);


    if (percpu_tss == NULL) {
        LogStr("percpu_tss = null");
        LogfStr("cpuid = %llu", percpu_read(cpu_id));
        cpu_stop();
    }
    tss_entry_t * tss_desc = &percpu_tss->tss;
    uint8_t * kernel_stack = percpu_tss->kernel_stack;
    // 1. Clear TSS
    for (size_t i = 0; i < sizeof(*tss_desc); i++) {
        ((uint8_t *)tss_desc)[i] = 0;
    }

    // 2. Point rsp0 to top of allocated kernel stack
    tss_desc->rsp0 = (uint64_t)&kernel_stack[sizeof(kernel_stack)];
    tss_desc->iomap_base = sizeof(*tss_desc); // Disable I/O permission bitmap

    // 3. Populate the 16-byte TSS descriptor in the GDT
    uint64_t base = (uint64_t)tss_desc;
    uint32_t limit = sizeof(*tss_desc) - 1;

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
