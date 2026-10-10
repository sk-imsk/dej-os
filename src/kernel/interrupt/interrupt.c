#include <stdint.h>
#include <dej/cpu.h>
#include <x86/x86.h>
#include <dej/log.h>
#include <dej/panic.h>
#include <dej/percpu.h>
#include <dej/watchdog.h>
#include <dej/msr.h>

lock_t iv_lock = {
    .held = false,
    .holding_cpu = -1
};

static bool vectors[256];       // if the vector is used or sno

typedef struct {
    uint64_t rax, rcx, rdx, rbx, rbp, rsi, rdi;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
} __attribute__((packed)) base_regs_t;

typedef struct {
    uint64_t rax, rcx, rdx, rbx, rbp, rsi, rdi;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;

    uint64_t ec;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) gp_registers_t;

typedef struct {
    // General-purpose registers (pushed manually by assembly)
    uint64_t rax, rcx, rdx, rbx, rbp, rsi, rdi, r8, r9, r10, r11, r12, r13, r14, r15;

    // Hardware frame
    uint64_t error_code; // Pushed automatically by CPU for page faults
    uint64_t rip;        // Where the fault happened
    uint64_t cs;         // Code segment
    uint64_t rflags;     // CPU flags
    uint64_t rsp;        // Stack pointer before the fault
    uint64_t ss;
} __attribute__((packed)) page_fault_regs_t;

typedef struct {
    // General-purpose registers (pushed manually by assembly)
    uint64_t rax, rcx, rdx, rbx, rbp, rsi, rdi, r8, r9, r10, r11, r12, r13, r14, r15;

    uint64_t error_code, rip, cs, rflags,  rsp, ss;
} __attribute__((__packed__)) tss_regs_t;

struct InterruptDescriptor {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attributes;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t zero;
}__attribute__((packed));

_Static_assert(sizeof(struct InterruptDescriptor) == 16,
               "IDT entry must be 16 bytes");

struct IDTR {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));

_Static_assert(sizeof(struct IDTR) == 10, "IDTR must be 10 bytes");

struct InterruptDescriptor idt[256];

static inline always_inline uint64_t get_cr2(void)
{
    uint64_t cr2;

    __asm__ volatile (
        "mov %%cr2, %0"
        : "=r"(cr2)
    );

    return cr2;
}

void idt_set_gate(uint8_t vector, void (*handler), uint8_t flags)
{

    uint64_t addr = (uint64_t)handler;

    idt[vector].offset_1 = addr & 0xFFFF;
    idt[vector].selector = 0x28;
    idt[vector].ist = 0;
    idt[vector].type_attributes = flags;
    idt[vector].offset_2 = (addr >> 16) & 0xFFFF;
    idt[vector].offset_3 = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;

    vectors[vector] = true;

}



extern void int_divide_by_0(void);
void divide_by_0_handler(void){
    LogStr("Division by 0 occured");
    cpu_stop();
}

extern void int_nmi(void);
//in nmi.c

extern void int_tss(void);
void tss_handler(tss_regs_t * frame){
    LogfStr("Tss fault frame: ss : %llu rflags: %llu cs: %llu rip: %llu rsp: %llu error code: %llu", frame->ss, frame->rflags, frame->cs, frame->rip, frame->rsp, frame->error_code);

    cpu_stop();
}

extern void int_general_protection_fault(void);
void general_protection_fault_handler(gp_registers_t * frame){
    LogfStr(
        "GP: ec=%lx rip=%lx cs=%lx rflags=%lx rsp=%lx ss=%lx\n",
        frame->ec,
        frame->rip,
        frame->cs,
        frame->rflags,
        frame->rsp,
        frame->ss
    );
    LogfStr("cpu: %lu", percpu_read(cpu_id));

    cpu_stop();
}


extern void int_page_fault(void);
void page_fault_handler(page_fault_regs_t * frame){
    LogfStr("Page fault frame: cs: %llu ec: %llu rip: %llu rflags: %llu \n", frame->cs, frame->error_code, frame->rip, frame->rflags);
    uint64_t cr2 = get_cr2();
    LogfStr("cr2: %llu \n", cr2);
    if (frame->cs > 0x31) LogfStr("from userspace ss = %llu rsp = %llu", frame->ss, frame->rsp);
    LogStr("page fault");
    cpu_stop();
}

extern void int_watchdog(base_regs_t * x __unused);



void InterruptInit(void){


    idt_set_gate(0x00, int_divide_by_0, 0x8E);
    idt_set_gate(0x02, int_nmi, 0x8E);
    idt_set_gate(0x0A, int_tss, 0x8E);
    idt_set_gate(0x0D, int_general_protection_fault, 0x8E);
    idt_set_gate(0x0E, int_page_fault, 0x8E);// will add more later
    idt_set_gate(0x30, int_watchdog, 0x8E);


    struct IDTR idtr = {
        .limit = sizeof(idt) - 1,
        .base = (uint64_t)idt
    };




    x86_load_idt(&idtr);
}


/*
 * RegisterInterruptVector: attempts to register a interrupt handler
 * Vector: Interrupt vector requested
 * *Handler: function pointer to le interrupt handler
 * Name: name of function not driver
 *
 */
void RegisterInterruptVector(uint8_t vector, void (*handler)(void), char * name){
    aquire_lock(&iv_lock);

    if (vector <= 64) {
        panic("Attempted register reserved interrupt vector", STATUS_UNKNOWN);
    }


    if (vectors[vector] == true){
        LogfStr("vector %u used attemped to be registered by %s", vector, name);
        panic("Vector in use ", STATUS_TAKEN);
    }

    idt_set_gate(vector, handler, 0xEE);

    LogfStr("Interrupt vector %u registered succesfully to \"%s\"\n", vector, name);

    release_lock(&iv_lock);

    return;
}



#define LAPIC_ICR_LOW   0x300
#define LAPIC_ICR_HIGH  0x310

#define ICR_DELIVERY_PENDING (1u << 12)
static void * lapic;
static bool x2;

static inline uint32_t lapic_read(uint32_t r){
	return *(volatile uint32_t *)(lapic + r);
}
static inline void lapic_write(uint32_t r, uint32_t val){
	*(volatile uint32_t *)(lapic + r) = val;

}

int send_ipi(uint8_t vector, uint32_t ap_id){
	uint64_t msr = rdmsr(MSR_IA32_APIC_BASE);

	uint8_t en = (msr >> 11) & 1;
	uint8_t extd = (msr >> 10) & 1;

    	if (en && extd) {
     		//x2apic
     		uint64_t icr =
                   ((uint64_t)ap_id << 32) |
                   vector;

               wrmsr(MSR_X2APIC_ICR, icr);
               x2 = true;
     	} else if (en == 1 && extd == 0){
    		if (!lapic) lapic = map_mmio(rdmsr(MSR_IA32_APIC_BASE) & 0xFFFFF000ULL);

      		while (lapic_read(LAPIC_ICR_LOW) & ICR_DELIVERY_PENDING) __asm__ volatile("pause");


          	lapic_write(LAPIC_ICR_HIGH, (ap_id & 0xFF) << 24);

           	// Interrupt vector goes in ICR LOW, bits 0-7.
		// Fixed delivery mode is 000, so the vector alone is sufficient.
          	lapic_write(LAPIC_ICR_LOW, vector);

           	x2 = false;

      	} else {
       		LogStr("apic disabled\n");
         	return -1;
       }


     return 0;
}
void send_eoi(void){
	if (x2){
		wrmsr(MSR_APIC_EOI, 0);
	} else {
		lapic_write(0x0B0, 0);
	}
}
