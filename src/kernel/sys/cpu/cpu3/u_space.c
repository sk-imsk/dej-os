/*
 * User space idk ig
 *
 */
#include <dej/log.h>
#include <dej/kernel.h>
#include "../interrupt/interrupt.h"


__attribute__((interrupt)) void handle_interrupt(void * frame __unused){
    __asm__ volatile ("nop");
}

extern _Noreturn void jmp2user(void * rip, void * rsp);

void enter_userspace(void){
    RegisterInterruptVector(0x80, (void *)handle_interrupt, "Enter userspace (cpu3)");



    jmp2user((void *)0x400000, (void *)0x801000 + KiB(4));

}
