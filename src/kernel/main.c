// the kernel ig
#include "x86.h"
#include "lim/limine.h"
#include "stddef.h"
#include "stdbool.h"
#include "keyboard/keyboard.h"
#include "stdio.h"
#include "interrupt/interrupt.h"
#include "memory/memory.h"
#include "string.h"
#include <stdint.h>

// limine stuff (6 is latest revision)
__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};
__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
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

    // Ensure the bootloader actually gets us
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
         serial_puts("Didnt recieve a framebuffer\n");
         hcf();
    }

    // make sure we got a memmap
    if (memmap_request.response == NULL || memmap_request.response->entry_count < 1) {
        serial_puts("Didnt recieve a memmap\n");
        hcf();
    }

    // make sure we got hhdm or something
    if (hhdm_request.response == NULL){
        serial_puts("Didnt recieve a hhdm\n");
        hcf();
    }


    int8_t result;
    serial_init();
    idt_init();
    memory_init(memmap_request.response, hhdm_request.response);
    result = keyboard_init();

    serial_puts("kentry\n");

    switch (result){
        case -1:
            x86_outb(0x3F8, 'x');   // x for very strange error
            hcf();
        case 0:
            break;
        case 1:
            x86_outb(0x3F8, 't'); // t for test failed
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

    void * data = givemeapage();
    memset(data, 0, 1024*4);
    retpage(data);

    // We're done, just hang...
    hcf();
}
