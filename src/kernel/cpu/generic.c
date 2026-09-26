#include <limine.h>
#include <dej/cpu.h>
#include <stdint.h>
#include "cpu1/temperature.h"
#include "cpu2/health.h"
#include "cpu3/user_space.h"
#include "../memory/memory.h"
#include <dej/panic.h>
#include <dej/string.h>
#include <dej/msr.h>
#include <dej/interrupt.h>
#include <dej/percpu.h>
#include <dej/stdio.h>
#include <stdatomic.h>


#define MSR_GS_BASE 0xC0000101
/*
 *  irq is basically like which interrupt is being handled
 * sil is interrupt level to like mask stuff
 *  id is cpu id (laptic)
 *
 */

DEFINE_PERCPU(_Atomic uint64_t, cpu_state);
DEFINE_PERCPU(uint64_t, irq);
DEFINE_PERCPU(uint64_t, sil);
DEFINE_PERCPU(uint64_t, cpu_id);


static _Atomic uint8_t core = 0;

void ap_entry(struct limine_mp_info *cpu){

    uint64_t my_core = atomic_fetch_add(&core, 1) + 1;

    cpu_stop_interrupts();
    InterruptInit();

    if (percpu_size >= 4096){
        panic("percpu tables too big prob like something wrong or ill fix it later or something\n");
    }

    char * n_block = KGetPage();
    memset(n_block, 0, 4096);           // zero out

    memcpy(n_block, __percpu_start, percpu_size);


    wrmsr(MSR_GS_BASE, (uint64_t)n_block);

    percpu_write(cpu_id, cpu->lapic_id);

    cpu_percpu[my_core] = (uint8_t *)n_block;

    cpu_enable_interrupts();
    percpu_write(sil, 0);       // enable all interrupts

    printf("enabling cpu %i \n", my_core);
    switch (my_core) {
        case 1: {
            temperature_entry();
            break;
        }
#ifdef __HEALTH
        case 2:
            HealthMonitor();
            break;
#endif
        case 3:
            enter_userspace();
            break;
        default: cpu_stop();
    }

    printf("core %ul returned halting on core\n", percpu_read(cpu_id));
    percpu_write(sil, 10);
    percpu_write(cpu_state, 0x1);
    cpu_stop();
}
