#include "stdio.h"
#include "x86.h"

#include <stdarg.h>


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
#define HEX_BUFFER_SIZE 19
/**
 * Converts a 64-bit unsigned integer base address to a hexadecimal string.
 * @param value  The memory address to convert.
 * @param buffer A pointer to a char array of at least HEX_BUFFER_SIZE bytes.
 * @return A pointer to the start of the formatted string inside the buffer.
 */
char* uint64_to_hex(uint64_t value, char *buffer) {
    // Start formatting from the end of the buffer (null terminator)
    int i = HEX_BUFFER_SIZE - 1;
    buffer[i] = '\0';

    // Handle the edge case where the address is exactly 0
    if (value == 0) {
        buffer[--i] = '0';
    } else {
        // Extract 4-bit chunks (nibbles) and convert to hex characters
        while (value > 0 && i > 2) {
            uint8_t nibble = value & 0xF;
            buffer[--i] = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
            value >>= 4;
        }
    }

    // Add standard hexadecimal prefix
    buffer[--i] = 'x';
    buffer[--i] = '0';

    // Return pointer to where the string actually begins in the buffer
    return &buffer[i];
}
