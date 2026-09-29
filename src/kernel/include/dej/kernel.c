#include <dej/kernel.h>
#include <dej/percpu.h>

void aquire_lock(lock_t * lock){
    bool percpu_ready = ispercpuready();
    int current_cpu;

    if (percpu_ready){
        current_cpu = percpu_read(cpu_id);
    } else {
        current_cpu = 0;
    }

    if (lock->holding_cpu == current_cpu) return;               // dont waste time yo

    while (atomic_exchange_explicit(&lock->held, true, memory_order_acquire)){
        cpu_takebreak();
    }

    lock->holding_cpu = current_cpu;
    lock->held = true;

}

void release_lock(lock_t * lock) {


    // Clear the holding tag before releasing the memory boundary
    lock->holding_cpu = -1;
    atomic_store_explicit(&lock->held, false, memory_order_release);
}
