static inline __attribute__((noreturn)) void cpu_stop(void){
    while (1) __asm__ volatile ("hlt");
}

static inline void yeild_cpu(void){
    __asm__ volatile ("pause");
}

static inline void cpu_stop_interrupts(void){
    __asm__ volatile ("cli");
}

static inline void cpu_enable_interrupts(void){
    __asm__ volatile ("sti");
}
