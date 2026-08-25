#pragma once
#include <stdint.h>


void serial_puts(const char * s);
char* uint64_to_hex(uint64_t value, char *buffer);
