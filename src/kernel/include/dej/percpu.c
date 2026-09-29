//percpu.c
//
#include <stdint.h>
#include <dej/percpu.h>



uint8_t *cpu_percpu[MAX_CPUS];
extern tss_cpu tssforcpus[32];

void setupbspcpudata(){
    char * n_block = KGetPage();
    memset(n_block, 0, 4096);
    memcpy(n_block, __percpu_start, percpu_size);
    wrmsr(0xC0000101, (uint64_t)n_block);
    percpu_write(cpu_id, 0);
    percpu_write(tss, &tssforcpus[0]);
    cpu_percpu[0] = (uint8_t *)n_block;
    cpu_enable_interrupts();
    percpu_write(sil, 0);
}
bool ispercpuready(void){
    unsigned int low, high;

        // Safely read the 64-bit GS Base MSR without hitting memory or segments
        __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(MSR_GS_BASE));

        // If both low and high dwords are 0, GS base is empty/uninitialized
        return (low != 0) || (high != 0);
}
