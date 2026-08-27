#include "../../lim/limine.h"
#include <stdatomic.h>
#include <stdbool.h>
#include "../../random.h"
extern _Atomic uint64_t temperature;

volatile uint64_t ap_started = 0;

void ap_entry(struct limine_mp_info *cpu)
{
    uint64_t random;

    for (;;){
        if (rdrand(&random)) {
            atomic_store(&temperature, random % 131);
        }

        __asm__ volatile ("pause");
        __asm__ volatile ("pause"); // chill bro
    }

}
