#pragma once
#include <stdint.h>

#ifdef __x86_64__
#   include <arch/x86/serial.h>
#elif defined(__arm__)
#   error 32 bit arm not supported
#elif defined(__aarch64__)
#   include <arch/arm/serial.h>
#endif

char* uint64_to_hex(uint64_t value, char *buffer);
void printf(const char* fmt, ...);
