#pragma once
#include <stdint.h>



static inline uint64_t rdmsr(uint32_t msr)
{
    uint32_t lo, hi;

    __asm__ volatile (
        "rdmsr"
        : "=a"(lo), "=d"(hi)
        : "c"(msr)
    );

    return ((uint64_t)hi << 32) | lo;
}

static inline void wrmsr(uint32_t msr_id, uint64_t msr_val) {
    uint32_t edx = msr_val >> 32;
    uint32_t eax = msr_val & 0xFFFFFFFF;
    __asm__ __volatile__ (
        "wrmsr"
        : : "c" (msr_id), "d" (edx), "a" (eax)
    );
}
