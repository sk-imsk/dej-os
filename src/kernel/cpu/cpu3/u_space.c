/*
 * User space idk ig
 *
 */
#include <dej/ata.h>
#include <dej/stdio.h>
#include <dej/kernel.h>

extern _Noreturn void jmp2user(void * rip, void * rsp);

extern void * u_buf;
void enter_userspace(void){

    void * buf = (void *)(u_buf);
    struct file_fat32 exec = fat_open("dih.bin");
    if (exec.first_cluster < 2){
        printf("opening dih.bin failed ");
        return;

    }

    fat_read(exec, buf);

    jmp2user((void *)0x400000, (void *)0x801000 + KiB(4));

}
