#include <dej/kernel.h>
#include <dej/cpu.h>
#include <dej/percpu.h>
#include <dej/stdio.h>
#include <dej/panic.h>

extern _Atomic uint64_t temperature;
/*
 * checks system health
 *
 */


static int16_t health;
_Noreturn void HealthMonitor(void){
    health = 0;


    LogStr("Health monitor waiting for the nerds\n");
    while (!((uint64_t)(*cpu_percpu[1]) & 0x01)) cpu_takebreak();        // we need temperature CPU to be on to check system health
    LogStr("Starting health monitoring\n");

    while (true){
        if (atomic_load(&temperature) > 90) {
            LogfStr("High temperature temperature is at %i\n", atomic_load(&temperature) );
            health -= 10;
        }
        else if (atomic_load(&temperature) < 20) {
            LogfStr("bit chilly eh temperature is at %llu\n", atomic_load(&temperature));
            health -= 10;
        }

        else {
            health += 5;
        }

        cpu_takebreak();

        if (health <= -50) {
            panic("Doctor needed computer health dropped to extremly low levels \n");
        }

        for (uint32_t i = 0; i < 1000000; i++) cpu_takebreak();

        LogfStr("health score is at %d \n", health);


    }

}
