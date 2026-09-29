#include <stdint.h>
void InterruptInit(void);
void RegisterInterruptVector(uint8_t vector, void (*handler)(void), char * name);
