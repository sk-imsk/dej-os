// serial.c
#include <stdint.h>

#define UART_BASE 0x09000000UL

#define UART_DR 0x00
#define UART_FR 0x18

#define UART_FR_TXFF (1 << 5)

static inline void uart_write(uint64_t reg, uint32_t value)
{
    *(volatile uint32_t *)(UART_BASE + reg) = value;
}

static inline uint32_t uart_read(uint64_t reg)
{
    return *(volatile uint32_t *)(UART_BASE + reg);
}

void serial_init(void){
    (void)0;
}

void putc(const char p){
    while (uart_read(UART_FR) & UART_FR_TXFF)
            ;

        uart_write(UART_DR, (uint32_t)p);
}

void serial_puts(const char *s){
    while (*s)
            putc(*s++);
}
