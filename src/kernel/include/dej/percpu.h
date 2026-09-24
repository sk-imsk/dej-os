#pragma once
#include <dej/cpu.h>
#include <stdint.h>
#include "../../memory/memory.h"
#include <dej/string.h>
#include <dej/msr.h>


/*
 * per cpu things
 * help
 *
 */
#define __percpu __attribute__((section(".percpu")))
#define DEFINE_PERCPU(type, name) \
    __percpu type name


extern char __percpu_start[];
extern char __percpu_end[];

#define percpu_size __percpu_end - __percpu_start



#define percpu_offsetof(var) \
    ((uint64_t)&(var) - (uint64_t)__percpu_start)


// Read a 64-bit per-CPU variable
#define percpu_read(var) ({ \
    uint64_t __val; \
    uint64_t __off = percpu_offsetof(var); \
    __asm__ volatile ( \
        "movq %%gs:(%1), %0" \
        : "=r"(__val) \
        : "r"(__off) \
        : "memory" \
    ); \
    __val; \
})

// Write to a 64-bit per-CPU variable
#define percpu_write(var, val) ({ \
    uint64_t __off = percpu_offsetof(var); \
    uint64_t __val = (uint64_t)(val); \
    __asm__ volatile ( \
        "movq %1, %%gs:(%0)" \
        : \
        : "r"(__off), "r"(__val) \
        : "memory" \
    ); \
})


extern DEFINE_PERCPU(_Atomic uint64_t, cpu_state);
extern DEFINE_PERCPU(uint64_t, irq);
extern DEFINE_PERCPU(uint64_t, sil);
extern DEFINE_PERCPU(uint64_t, cpu_id);



#define MAX_CPUS 64

extern uint8_t *cpu_percpu[MAX_CPUS];       // pointer to all cpus data or something





void setupbspcpudata();
