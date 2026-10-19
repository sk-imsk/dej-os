#include <stdint.h>


void sleep_ms(uint32_t ms){
	for (volatile uint64_t i = 0; i < (uint64_t)ms * 100000; i++)
		__asm__ volatile("pause");		// bs will make better later
}

void sleep_short(uint32_t t){
	for (volatile uint64_t i = 0; i < t; i++)
		__asm__ volatile("pause");		// bs will make better later
}
