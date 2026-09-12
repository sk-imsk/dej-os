#include "arch/arm/cpu.h"
#include <stdint.h>
#include <dej/cpu.h>
#include <dej/stdio.h>
#include <dej/panic.h>




extern void exception_vectors(void);


void exception_handler(void){
    for (;;) cpu_stop();
}

void interrupt_init(void){

    __asm__ volatile (
            "msr VBAR_EL1, %0"
            :
            : "r"(exception_vectors)
            : "memory"
        );

        __asm__ volatile("isb");

}
