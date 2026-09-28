#include <x86/x86.h>
#define COM1 0x3F8



void  putc (char c){
    x86_outb(COM1, c);
}

void puts(const char * s){
    while (*s) x86_outb(COM1, *s++);
}
