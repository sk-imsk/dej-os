#pragma once
#include <stdint.h>
#include "stdbool.h"

void __attribute__((no_caller_saved_registers)) x86_outb(uint16_t port, uint8_t value);
uint8_t __attribute__((no_caller_saved_registers)) x86_inb(uint16_t port);
uint16_t __attribute__((no_caller_saved_registers)) x86_inw(uint16_t port);
void x86_load_idt(void * idtr);
