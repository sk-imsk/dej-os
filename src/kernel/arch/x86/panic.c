#include <dej/log.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <dej/cpu.h>
#include <dej/percpu.h>
#include <dej/framebuffer.h>
#include <dej/panic.h>



struct stack_frame{
    struct stack_frame * next;
    void * ret;
};

void draw_screen(void){
    struct limine_framebuffer * f = RequestFrameBuffer(0);
    uint64_t y = 0;
    while (y < f->height){
        for (uint64_t i = 0; i < f->width; i++){
            putpixel(i, y, 0x0000FF);                   // tuff blue
        }
        y++;
    }


}

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

    // if the cpu is retarded and doesnt reject the idt then tell it to handle a interrupt
    __asm__ volatile("int $3");

    for (;;)
            __asm__ volatile("cli; hlt");


}


_Noreturn void panic(const char * s, status code){
    cpu_stop_interrupts();



    LogStr("Yo panic rn everybody chill yo");

    if (ispercpuready()){
        if (percpu_read(cpu_state) &  0x1) {
            while (true){
                __asm__ volatile ("hlt");
            }
        }
    }


    // to do yo turn off all cpus


    LogfStr("\nPanic: %s code %u", s, code);


    draw_screen();


    // stack_unwind();

    triple_fault(); // turn off computer
}


_Noreturn void fi_panic(const char * cooked){ // like panic but were basically cooked instantly so dont bother with anything fancy
    LogStr(cooked); // we cooked gng
    triple_fault();
}
