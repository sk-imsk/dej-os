#include <stdint.h>
void idt_set_gate(uint8_t vector, void (*handler)(void));
void idt_init(void);
