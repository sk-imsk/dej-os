#include <dej/stdio.h>
#include <x86/x86.h>
#include <dej/cpu.h>
#include <dej/sil.h>
#include <stdarg.h>
#include <stdatomic.h>


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
