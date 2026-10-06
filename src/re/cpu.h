static inline void cpu_stop() {
    while (1) __asm__ volatile ("hlt");
}
static inline void disable_interrupts(void){
    __asm__ volatile ("cli");
}
static inline void cpu_enable_interrupts(void){
    __asm__ volatile ("sti");
}
static inline void cpu_takebreak() {
    __asm__ volatile ("pause");
}
