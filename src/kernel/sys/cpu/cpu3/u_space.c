/*
 * User space idk ig
 *
 */
#include <drivers/disk/ata.h>
#include <dej/log.h>
#include <dej/kernel.h>
#include "../interrupt/interrupt.h"


__attribute__((interrupt)) void handle_interrupt(void * frame __unused){
    __asm__ volatile ("nop");
}

extern _Noreturn void jmp2user(void * rip, void * rsp);

extern void * u_buf;
void enter_userspace(void){
    RegisterInterruptVector(0x80, (void *)handle_interrupt, "Enter userspace (cpu3)");

    void * buf = (void *)(u_buf);
    struct file_fat32 exec = fat_open("dih.bin");
    if (exec.first_cluster < 2){
        LogStr("opening dih.bin failed \n");
        return;

    }

    fat_read(exec, buf);

    jmp2user((void *)0x400000, (void *)0x801000 + KiB(4));

}
