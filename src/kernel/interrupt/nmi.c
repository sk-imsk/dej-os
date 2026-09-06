#include "../stdio.h"
#include "../x86.h"
#include "../msr.h"
#include "../panic.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    // General-purpose registers (pushed manually by assembly)
    uint64_t rax, rcx, rdx, rbx, rbp, rsi, rdi, r8, r9, r10, r11, r12, r13, r14, r15;

    // Hardware frame
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) nmi_registers_t;

static struct {
    bool r_ip;
    bool e_ip;
    bool mcip;
    bool lmce;


}state;
#define IA32_MCG_CAP 0x179
bool is_bsp(void) {
    uint64_t apic_base = rdmsr(0x1B);
    // Bit 11 is the Bootstrap Processor flag
    return (apic_base & (1ULL << 11)) ? 1 : 0;
}

static uint8_t check_severity(){
    uint32_t num_banks = rdmsr(IA32_MCG_CAP) & 0xFF;

    for (uint32_t i = 0; i < num_banks; i++){
        uint32_t status_msr = 0x401 + (i * 4); // Calculate MCx_STATUS address
        uint64_t status = rdmsr(status_msr);

        if (!(status & (1ULL << 63))) {
            continue;
        }


        if (status & (1ULL << 56)) {
            fi_panic("Cooked PCC corrupted");       //pcc corrupted so basically the cpu is scitzo
        }

        if (status & (1ULL << 61)) {
            // Bit 57: Action Required (AR)
            if (status & (1ULL << 57)) {
                fi_panic("nmi action required (im not good enough to fix it so kill)");
            } else {
                 return 1; // means can run if we kill the process (probably cooked)

            }
        } else {
            return 0;
        }
    }
    return 0;


}

extern _Atomic uint64_t temperature;
extern void int_nmi(void);
void nmi_handler(nmi_registers_t * regs){
    char msg[19];
    char msg2[19];

    uint64_t crash_addr = regs->rip;
    uint64_t crash_rax = regs->rax;
    uint64_to_hex(crash_addr, msg);
    uint64_to_hex(crash_rax, msg2);

    printf("NMI address = %p RAX = %p \n\n",msg , msg2);

    uint64_t res = check_severity();
    if (res == 0) return; // we good yo
    if (res == 1 && !(is_bsp())){
        serial_puts("\n NMI on temperature thread killing core");
        atomic_exchange(&temperature, 999999);
        res = rdmsr(0x17A);
        res &= ~(1ULL << 2);
        wrmsr(0x17A, res);

        while (true) __asm__ volatile ("cli; hlt");
    }
    // we prob cooked gang
    //
    //
    uint32_t num_banks = rdmsr(0x179) & 0xFF;

    serial_puts("--- MCA BANK LOGS ---\n");
        for (uint32_t i = 0; i < num_banks; i++) {
            uint64_t bank_status = rdmsr(0x401 + (i * 4)); // MCx_STATUS

            // If Bit 63 (Valid) is set, this bank has our crime scene data
            if (bank_status & (1ULL << 63)) {
                serial_puts("Bank ");
                uint64_to_hex(i, msg); serial_puts(msg);
                serial_puts(" STATUS = ");
                uint64_to_hex(bank_status, msg); serial_puts(msg);
                serial_puts("\n");

                // Bit 59: Address Valid (ADDRV) -> If set, fetch the physical address
                if (bank_status & (1ULL << 59)) {
                    uint64_t bank_addr = rdmsr(0x402 + (i * 4)); // MCx_ADDR
                    serial_puts("  PHYS ADDR = ");
                    uint64_to_hex(bank_addr, msg); serial_puts(msg);
                    serial_puts("\n");
                }
            }
        }






    res = rdmsr(0x17A);

    state.mcip = (res & 0x04) ? true : false; // mcip (machine check in progress) first beacuse if its there we gotta get it early
    if (state.mcip == true){
        fi_panic("s.minmi"); // if there is a machine check while a nmi is happening basically were cooked
    }


    state.r_ip = (res & 0x01) ? true : false; // RIPV
    state.e_ip = (res & 0x02) ? true : false; // EIPV
    state.lmce = (res & 0x08) ? true : false; // lmce



    res = x86_inb(0x92);
    if (res == 4){
        __asm__ volatile (
            "in $0x92, %%al\n\t"
            "and $0xfb, %%al\n\t"
            "out %%al, $0x92"
            :
            :
            : "al"
        );

         panic("Watchdog expired");

    }

    res = x86_inb(0x61);
    switch (res){
        case 6:
            panic("Bus Error (from legacy port)");
        case 7:
            panic("Memory failure (from legacy port)");
    }





    return;


}
