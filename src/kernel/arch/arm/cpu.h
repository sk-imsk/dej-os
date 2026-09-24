// cpu.h
#pragma once

#define apic_id processor_id
#define bsp_id bsp_mpidr

static inline void cpu_takebreak(void){
    __asm__ volatile ("yeild" ::: "memory");
}
static inline void cpu_stop_interrupts(void){
    __asm__ volatile ("msr DAIFSet, #2");
}
static inline void cpu_enable_interrupts(void){
    __asm__ volatile ("msr DAIFClr, #2");
}
static inline _Noreturn void cpu_stop(void){
    cpu_stop_interrupts();
    while (1){
        __asm__ volatile ("wfi");
    }
}

static inline void check_watchdog() {
    return;
}
