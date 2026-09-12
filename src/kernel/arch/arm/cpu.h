// cpu.h
#pragma once

static inline void cpu_takebreak(void){
    __asm__ volatile ("yield" ::: "memory");
}
static inline void cpu_stop_interrupts(void){
    __asm__ volatile ("msr daifset, #2");
}
static inline void cpu_enable_interrupts(void){
    __asm__ volatile ("msr daifclr, #2");
}
static inline void cpu_stop(void){
    cpu_stop_interrupts();
    while (1){
        __asm__ volatile ("wfi");
    }
}




#define CPU_ID processor_id
#define BSP_ID bsp_mpidr


static inline void cpu_clear_watchdog(void){
    return;
}
