#include <limine.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <dej/log.h>
#include <dej/random.h>
#include <dej/cpu.h>
#include <dej/percpu.h>
extern _Atomic uint64_t temperature;

volatile uint64_t ap_started = 0;

void temperature_entry(void)
{
    uint64_t random;
    LogfStr("cpu %i is ready to read the temperature\n", percpu_read(cpu_id));

    percpu_write(cpu_state, 0x1);

    for (;;){
    	if (rdrand(&random)) {
     		atomic_store(&temperature, random % 131);
     	} else {
      		LogfStr("rdrand failed stopping high tech temperature sensor tech");
        	cpu_stop();
        }

     	if (!(percpu_read(cpu_state) & 0x1)) {
      		LogStr("temp shutting down cuz cpu state says so");
        	cpu_stop_interrupts();
         	cpu_stop();
        }


        cpu_takebreak(); // chill bro
    }

}       // trust temperature is real bro
