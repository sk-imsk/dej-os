#include <stdint.h>
void InterruptInit(void);
void RegisterInterruptVector(uint8_t vector, void (*handler)(void), char * name);
int send_ipi(uint8_t vector, uint32_t ap_id);
void send_eoi(void);
