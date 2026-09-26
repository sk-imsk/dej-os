/*
 * User space idk ig
 *
 */
#include <dej/ata.h>
#include <dej/stdio.h>


void enter_userspace(void){

    void * buffer = (void *)(0x400000);

    struct file_fat32 exec = fat_open("dih.bin");
    if (exec.first_cluster < 2){
        printf("opening dih.bin failed ");
        return;

    }

    fat_read(exec, buffer);


}
