#include <stdint.h>

#ifndef MSR_H
#define MSR_H
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
#endif
