#include "../stdio.h"
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


extern _Atomic bool cpu_running;
struct stack_frame{
    struct stack_frame * next;
    void * ret;
};


// prints or something
// uses frame pointer beacuse im not a nerd
void stack_unwind(void){
    serial_puts("\nStack trace\n");

    struct stack_frame * f;
    char buffer[19];

    __asm__ volatile ("movq %%rbp, %0": "=r" (f));

    int count = 0;
    while (f != NULL && count < 20) {

        if (f->ret == NULL) break;

        uint64_to_hex((uint64_t)f->ret, buffer);
        serial_puts(buffer);
        serial_puts("\r");

        f = f->next;
        count++;
    }
}

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


    if (atomic_exchange(&cpu_running, false)) {
        while (true){
            __asm__ volatile ("hlt");
        }
    }
    atomic_store(&cpu_running, false);          // let everyone know cpus shouldent be running (they will turn off eventually)


    serial_puts("\nPanic: ");
    serial_puts(s);
    serial_puts("\n");


    stack_unwind();

    triple_fault(); // turn off computer
}
