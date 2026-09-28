#include <dej/log.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <dej/cpu.h>
#include <dej/percpu.h>


struct stack_frame{
    struct stack_frame * next;
    void * ret;
};


// prints or something
// uses frame pointer beacuse im not a nerd
void stack_unwind(void){
    LogStr("\nStack trace\n");

    struct stack_frame * f;

    __asm__ volatile ("movq %%rbp, %0": "=r" (f));

    int count = 0;
    while (f != NULL && count < 20) {

        if (f->ret == NULL) break;

        LogfStr("%llu \r", f->ret);

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



    LogStr("Yo panic rn everybody chill yo");

    /*
    if (percpu_read(cpu_state) & 0x1) {
        while (true){
            __asm__ volatile ("hlt");
        }
    }
    */
    // to do yo turn off all cpus


    LogfStr("\nPanic: %s \n", s);



    // stack_unwind();

    triple_fault(); // turn off computer
}


_Noreturn void fi_panic(const char * cooked){ // like panic but were basically cooked instantly so dont bother with anything fancy
    LogStr(cooked); // we cooked gng
    triple_fault();
}
