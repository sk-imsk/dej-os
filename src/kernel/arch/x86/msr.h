#pragma once
#include <stdint.h>


//msrs will add more if i feel like idk
#define MSR_GS_BASE                                       0xC0000101
#define MSR_FS_BASE                                       0xC0000100
#define MSR_IA32_ARCH_CAPABILITIES        0x10A
#define MSR_KERNEL_GS_BASE                        0xC0000102
#define MSR_IA32_FEATURE_CONTROL          0x3A
#define MSR_IA32_MCG_STATUS                    0x17F
#define MSR_IA32_TIME_STAMP_COUNTER  0x10
#define MSR_IA32_PERF_STATUS                    0x198
#define MSR_IA32_THERM_STATUS                0x19C
#define MSR_IA32_STAR                                   0xC0000104



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
