#include "arm/cpu.h"
#include <dej/stdio.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <dej/cpu.h>
#include <dej/percpu.h>


struct stack_frame{
    struct stack_frame * next;
    void * ret;
};


// prints or something
// uses frame pointer beacuse im not a nerd
void stack_unwind(void){
    serial_puts("No stack unwind (arm)");
}


_Noreturn void panic(const char * s){
    __asm__ volatile ("cli");


    if (percpu_read(cpu_state) & 0x1) {
        while (true){
            __asm__ volatile ("hlt");
        }
    }
    // to do yo turn off all cpus


    printf("\nPanic: %s \n", s);



    // stack_unwind();

    cpu_stop(); // turn off computer
}


_Noreturn void fi_panic(const char * cooked){ // like panic but were basically cooked instantly so dont bother with anything fancy
    serial_puts(cooked); // we cooked gng
    cpu_stop();
}
