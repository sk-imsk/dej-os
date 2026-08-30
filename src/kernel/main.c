// the kernel ig
#include "x86.h"
#include "lim/limine.h"
#include "stdio.h"
#include "interrupt/interrupt.h"
#include "memory/memory.h"
#include "string.h"
#include <stdint.h>
#include "panic.h"
#include "msr.h"
#include "cpu/cpu1/temprature.h"
#include <stdatomic.h>
#include "disk/disk.h"

__attribute__((section(".temperature")))
_Atomic uint64_t temperature;


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
__attribute__((used, section(".limine_requests")))
static volatile struct limine_mp_request mp_request = {
    .id = LIMINE_MP_REQUEST_ID,
    .revision = 0
};


__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;


static _Atomic bool kentry_ran = false;
_Atomic bool cpu_running;

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
    if (atomic_exchange(&kentry_ran, true)) panic("kentry ran twice");
    atomic_store(&cpu_running, true);
    atomic_exchange(&kentry_ran, true);

    // Ensure the bootloader actually gets us
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        panic("Bootloader doesnt support our revision");
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
         panic("Didnt recieve a framebuffer");
    }

    // make sure we got a memmap
    if (memmap_request.response == NULL || memmap_request.response->entry_count < 1) {
        panic("Didnt recieve a memmap");
    }

    // make sure we got hhdm or something
    if (hhdm_request.response == NULL){
        panic("Didnt recieve a hhdm");
    }
    // make sure we got a mp thing
    if (mp_request.response == NULL){
        panic("Didnt recieve a mp");
    }



    serial_init();
    idt_init();
    memory_init(memmap_request.response, hhdm_request.response);


    if (x86_inb(0x92) == 4) {
        serial_puts("Last system failure caused by watchdog");
        __asm__ volatile (
            "in $0x92, %%al\n\t"
            "and $0xfb, %%al\n\t"
            "out %%al, $0x92"
            :
            :
            : "al"
        );


    }

    serial_puts("kentry\n");



    struct limine_mp_response * mp = mp_request.response;



    for (uint64_t i = 0; i < mp->cpu_count; i++) {
        struct limine_mp_info *cpu = mp->cpus[i];

        if (cpu->lapic_id != mp->bsp_lapic_id) {
            __atomic_store_n(
                &cpu->goto_address,
                ap_entry,
                __ATOMIC_RELEASE
            );
            break;
        }
    }

    disk_init();

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    volatile uint32_t *fb_ptr = framebuffer->address;

    for (size_t y = 0; y < framebuffer->height; y++) {
        for (size_t x = 0; x < framebuffer->width; x++) {
            uint32_t nY = y * 255 / framebuffer->height;
            uint32_t nX = x * 255 / framebuffer->width;
            uint32_t red = nX;
            uint32_t blue = 233;
            uint32_t green = nY;

            fb_ptr[y * (framebuffer->pitch / 4) + x] = (red << 16) | (green << 8) | blue;
        }
    }








    for (;;) __asm__ volatile ("hlt"); // yo dont forget
}
