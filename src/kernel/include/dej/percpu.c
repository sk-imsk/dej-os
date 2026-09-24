//percpu.c
//
#include <stdint.h>
#include <dej/percpu.h>


uint8_t *cpu_percpu[MAX_CPUS];

void setupbspcpudata(){
    char * n_block = KGetPage();
    memset(n_block, 0, 4096);
    memcpy(n_block, __percpu_start, percpu_size);
    wrmsr(0xC0000101, (uint64_t)n_block);
    percpu_write(cpu_id, 0);
    cpu_percpu[0] = (uint8_t *)n_block;
    cpu_enable_interrupts();
    percpu_write(sil, 0);
}
