#include "../stdio.h"


_Noreturn void panic(const char * s){
    serial_puts("Panic: ");
    serial_puts(s);



    for (;;) __asm__ volatile ("cli; hlt");
}
