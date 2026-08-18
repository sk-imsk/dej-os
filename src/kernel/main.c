// the kernel ig
#include "x86.h"
#include "lim/limine.h"
#include "stddef.h"
#include "stdbool.h"
#include "keyboard/keyboard.h"
#include <stdint.h>
#include "stdio.h"


// limine stuff (6 is latest revision)
__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

void hcf(void){
    for (;;)
        __asm__ volatile ("hlt");

}

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


void kentry(void) {

    int8_t result;
    serial_init();
    x86_outb(0x3F8, 'k');
    result = keyboard_init();

    switch (result){
        case -1:
            x86_outb(0x3F8, 'x');   // x for very strange error
            hcf();
        case 0:
            break;
        case 1:
            x86_outb(0x3F8, 't'); // t for test failed
    }

    // serial "kentry" for debugging or some

    x86_outb(0x3F8, 'e');
    x86_outb(0x3F8, 'n');
    x86_outb(0x3F8, 't');
    x86_outb(0x3F8, 'r');
    x86_outb(0x3F8, 'y');

    // Ensure the bootloader actually understands our base revision.
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Print a nice pattern to screen as an example.
    // Note: we assume the framebuffer model is RGB with 32-bit pixels.



    volatile uint32_t *fb_ptr = framebuffer->address;
    for (size_t y = 0; y < framebuffer->height; y++) {
        for (size_t x = 0; x < framebuffer->width; x++) {
            uint32_t nY = y * 255 / framebuffer->height;

            uint32_t red = 255;
            uint32_t blue = nY;
            uint32_t green = 105;

            fb_ptr[y * (framebuffer->pitch / 4) + x] = (red << 16) | (green << 8) | blue;
        }
    }





    // We're done, just hang...
    hcf();
}
