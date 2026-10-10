//
// dog.c
//
// runs the watchdog
//
//
#include <dej/kernel.h>
#include <dej/percpu.h>
#include <dej/watchdog.h>
#include <dej/random.h>
#include <dej/cpu.h>
#include <dej/panic.h>
#include <dej/interrupt.h>

static void delay_bad(uint32_t v){
	while (v--){
		cpu_takebreak();
	}
}


void watchdog_handler(void){
	uint64_t gs = rdmsr(MSR_GS_BASE);
	struct watchdog * w = (void *)(percpu_offsetof(dog) + gs);

	atomic_store(&w->here, true);

	send_eoi();

	return;
}

_Noreturn void jump_to_watchdog(void){
	uint8_t cpus = 0;
	struct watchdog * curr_watchdog = NULL;
	uint32_t offdog = percpu_offsetof(dog);
	uint32_t offid = percpu_offsetof(cpu_id);

	for (uint8_t i = 1; i < 64; i++){

		if (cpu_percpu[i] != NULL){
			cpus++;


		}
	}

	uint8_t i;
	while (true){
		i = 0;
		while (i <= cpus){
			uint32_t * id = (cpu_percpu[i] + offid);
			if (i == 0){
				i++;
				continue;
			}
			curr_watchdog = cpu_percpu[i] + offdog;


			atomic_store(&curr_watchdog->here, false);

			send_ipi(0x30, *id);

			delay_bad(100);

			if (!atomic_load(&curr_watchdog->here))
				curr_watchdog->anger += (curr_watchdog->is_having_hard_time) ? 1 : 10;
			else {
				curr_watchdog->anger = 0 ; 			// reset cuz we gota  response
			}

			if (curr_watchdog->anger >= 200) {
				LogfStr("dog: cpu %u is not responding \n", *id);
				panic("cpu not responding", STATUS_LOCKUP);
			}

			if (curr_watchdog->anger > 100){
				LogfStr("Dog: cpu %i not responding will take action if still uncorrected (anger = %u)\n", *id, curr_watchdog->anger);
			}
			i++;
		}

		delay_bad(1000000); 		// give them a break



	}


}
