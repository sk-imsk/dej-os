#include "../../lim/limine.h"
#include <stdatomic.h>
#include <stdbool.h>
#include "../../stdio.h"
#include "../../random.h"
#include "../../cpu.h"
extern _Atomic uint64_t temperature;

volatile uint64_t ap_started = 0;
extern _Atomic bool cpu_running;

void ap_entry(struct limine_mp_info *cpu)
{
    uint64_t random;
    printf("cpu %i is ready to read the temperature\n", cpu->lapic_id);

    for (;;){
        if (rdrand(&random)) {
            atomic_store(&temperature, random % 131);
        } else {
            printf("rdrand failed stopping high tech temperature sensor tech");
            cpu_stop();
        }

        if (atomic_load(&cpu_running) == true) {
          cpu_stop_interrupts();
          cpu_stop();
        }


        cpu_takebreak(); // chill bro
    }

}       // inside my kernel that i made lol
