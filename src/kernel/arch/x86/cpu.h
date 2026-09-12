// cpu.h
#pragma once
#include <dej/stdio.h>
#include <arch/x86/x86.h>

static inline void cpu_takebreak(void){
    __asm__ volatile ("pause" ::: "memory");
}
static inline void cpu_stop_interrupts(void){
    __asm__ volatile ("cli");
}
static inline void cpu_enable_interrupts(void){
    __asm__ volatile ("sti");
}
static inline void cpu_stop(void){
    cpu_stop_interrupts();
    while (1){
        __asm__ volatile ("hlt");
    }
}

#define CPU_ID laptic_id
#define BSP_ID bsp_laptic_id

static inline void cpu_clear_watchdog(void){
    if (x86_inb(0x92) == 4) {
        serial_puts("Last system failure caused by watchdog");
        __asm__ volatile (
            "in $0x92, %%al\n\t"
            "and $0xfb, %%al\n\t"
            "out %%al, $0x92"
            :
            :
            : "al"
    );


    }
}
