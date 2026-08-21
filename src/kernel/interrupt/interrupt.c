


#include <stdint.h>
#include "../x86.h"
#include "../stdio.h"

struct InterruptDescriptor {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attributes;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t zero;
}__attribute__((packed));

_Static_assert(sizeof(struct InterruptDescriptor) == 16,
               "IDT entry must be 16 bytes");

struct IDTR {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));

_Static_assert(sizeof(struct IDTR) == 10, "IDTR must be 10 bytes");

struct InterruptDescriptor idt[256];

void idt_set_gate(uint8_t vector, void (*handler)(void))
{
    uint64_t addr = (uint64_t)handler;

    idt[vector].offset_1 = addr & 0xFFFF;
    idt[vector].selector = 0x28;
    idt[vector].ist = 0;
    idt[vector].type_attributes = 0x8E;
    idt[vector].offset_2 = (addr >> 16) & 0xFFFF;
    idt[vector].offset_3 = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;
}





extern void int_divide_by_0(void);
void err_divide_by_0(void){
    serial_puts("Division by 0 occured");
    for (;;) __asm__ volatile ("cli; hlt");
}





void idt_init(void){


    idt_set_gate(0, int_divide_by_0);


    struct IDTR idtr = {
        .limit = sizeof(idt) - 1,
        .base = (uint64_t)idt
    };




    x86_load_idt(&idtr);
}
