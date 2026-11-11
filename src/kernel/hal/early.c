#include <dej/kernel.h>

#ifdef __x86_64__
void e_logc(char c){
	__asm__ volatile (
        "outb %0, %%dx"
        :
        : "a"((uint8_t)c), "d"((uint16_t)0x3F8)
    );
}
#endif
