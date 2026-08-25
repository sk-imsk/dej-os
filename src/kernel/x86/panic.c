#include "../stdio.h"

// yo rebooting normally is too much work so lowk i have a better way
_Noreturn static void triple_fault(void) {
    // create bs idt to make computer crash out
    volatile uint16_t idt_ptr[3] = {0, 0, 0};

    __asm__ volatile("lidt (%0)" : : "r"(idt_ptr));

    // trigger a interupt and make the le computer die or something
    __asm__ volatile("int $3");

    for (;;)
            __asm__ volatile("cli; hlt");


}


_Noreturn void panic(const char * s){
    __asm__ volatile ("cli");
    serial_puts("\rPanic: ");
    serial_puts(s);


    triple_fault();
}
