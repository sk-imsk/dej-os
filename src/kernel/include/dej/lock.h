#include <dej/kernel.h>

typedef struct {
    atomic_bool held;
    int holding_cpu;
}lock_t;

typedef lock_t spinlock_t;


void aquire_spinlock(spinlock_t *l);
