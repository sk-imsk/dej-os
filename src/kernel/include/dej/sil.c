#include <dej/sil.h>
#include <dej/kernel.h>
#include <dej/cpu.h>
#include <dej/panic.h>
#include <dej/percpu.h>

static inline ksil __readcr8(){
    uint64_t cr8;
        __asm__ volatile("mov %%cr8, %0" : "=r"(cr8));
        return (ksil)cr8;
}

static inline void __writecr8(ksil level){
    int64_t cr8 = level;
        __asm__ volatile("mov %0, %%cr8" : : "r"(cr8) : "memory");
}


ksil RaiseSil(ksil new){

    ksil old = (ksil)__readcr8();

    if (old > new) panic("Tried to lower sil using RaiseSil", STATUS_INVALID_SIL);

    __writecr8(new);

    percpu_write(sil, new);
    return old;
}


void LowerSil(ksil new){
    ksil old = (ksil)__readcr8();

    if (old < new) panic("attempted to raise sil using LowerSil", STATUS_INVALID_SIL);

    __writecr8(new);

    percpu_write(sil, new);

    return;
}
