#include "arch/arm/cpu.h"
#include <dej/stdio.h>
#include <stdbool.h>

_Noreturn void panic(const char *s){
    printf("Panic system shutting down");
    printf(s);
    while (true)    cpu_stop();
}
_Noreturn void fi_panic(const char * cooked){
    printf(cooked);

    while (true) cpu_stop();
}
