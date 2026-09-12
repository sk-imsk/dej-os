#include <arch/x86/x86.h>

void serial_init(void)
{
    x86_outb(0x3F8 + 1, 0x00); // Disable interrupts
    x86_outb(0x3F8 + 3, 0x80); // Enable DLAB
    x86_outb(0x3F8 + 0, 0x03); // Baud divisor low: 38400
    x86_outb(0x3F8 + 1, 0x00); // Baud divisor high
    x86_outb(0x3F8 + 3, 0x03); // 8 bits, no parity, 1 stop bit
    x86_outb(0x3F8 + 2, 0xC7); // Enable FIFO
    x86_outb(0x3F8 + 4, 0x0B); // IRQs enabled, RTS/DSR
}


void serial_puts(const char *s) {
    while (*s) {
        // 1. Wait for the hardware transmitter buffer to be empty
        // 0x3FD is the Line Status Register. Bit 5 (0x20) means "Ready to Transmit"
        while ((x86_inb(0x3FD) & 0x20) == 0) {
            // wait for slow ahh serial guy
        }


        if (*s == '\n') {
            x86_outb(0x3F8, '\r');

            // Wait again before sending the follow-up '\n'
            while ((x86_inb(0x3FD) & 0x20) == 0);
            x86_outb(0x3F8, '\n');
            s++; // Move to next character
            continue;
        }

        // 3. Send the regular character
        x86_outb(0x3F8, *s++);
    }
}

void putc(const char p){
    x86_outb(0x3F8, p);
}
