#include "lock.h"
#include <dej/cpu.h>

void aquire_spinlock(spinlock_t *l){
	int curr_cpu = 0;

	while (atomic_exchange_explicit(&l->held, true, memory_order_acquire)){
		yeild_cpu();
	}

	l->holding_cpu = curr_cpu;
}


void release_spinlock(spinlock_t *l){
	l->holding_cpu = -1;
	atomic_store_explicit(&l->held, false, memory_order_release);
}
