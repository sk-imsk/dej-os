static inline void cpu_stop(void){
    while (1) __asm__ volatile ("wfi");
}


static inline void yeild_cpu(void){
    __asm__ volatile ("yeild");
}
