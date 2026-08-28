#include "../../lim/limine.h"
#include <stdatomic.h>
#include <stdbool.h>
#include "../../random.h"
extern _Atomic uint64_t temperature;

volatile uint64_t ap_started = 0;
extern _Atomic bool cpu_running;

void ap_entry(struct limine_mp_info *cpu)
{
    uint64_t random;

    for (;;){
        if (rdrand(&random)) {
            atomic_store(&temperature, random % 131);
        }



        if (cpu_running == false){
            __asm__ volatile ("cli");
            while (true){
                __asm__ volatile ("hlt");
            }
        }


        __asm__ volatile ("pause");
        __asm__ volatile ("pause"); // chill bro
    }

}
