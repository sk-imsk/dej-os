static inline void cpu_stop(void){
    while (1) __asm__ volatile ("wfi");
}
