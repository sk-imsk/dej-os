#include <x86/x86.h>
#include "cpu.h"
#define COM1 0x3F8



void  putc (char c){
    while ((x86_inb(COM1 + 5) & 0x20 ) == 0) cpu_takebreak();
    x86_outb(COM1, c);
}

void puts(const char * s){
    while (*s) x86_outb(COM1, *s++);
}
