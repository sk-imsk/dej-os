// the kernel ig
#include <x86/x86.h>
#include <dej/stdio.h>
#include "interrupt/interrupt.h"
#include "memory/memory.h"
#include <dej/string.h>
#include <dej/panic.h>
#include <dej/msr.h>
#include <dej/cpu.h>
#include <dej/ata.h>
#include <dej/percpu.h>
#include <dej/kernel.h>
#include <dej/sil.h>

extern _Noreturn void kmain(void);
extern void ap_entry(struct limine_mp_info *cpu);

__attribute__((section(".temperature")))
_Atomic uint64_t temperature;


// limine stuff (6 is latest revision)
__attribute__((used, section(".limine_requests")))
volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};
__attribute__((used, section(".limine_requests")))
volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};
__attribute__((used, section(".limine_requests")))
volatile struct limine_mp_request mp_request = {
    .id = LIMINE_MP_REQUEST_ID,
    .revision = 0
};


__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;


static _Atomic bool kentry_ran = false;

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
    atomic_store(&kentry_ran, true);


    // Ensure the bootloader actually gets us
    if (unlikely(LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)) {
        panic("Bootloader doesnt support our revision");
    }

    // Ensure we got a framebuffer.
    if (unlikely(framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1)) {
         panic("Didnt recieve a framebuffer");
    }

    // make sure we got a memmap
    if (unlikely(memmap_request.response == NULL || memmap_request.response->entry_count < 1)) {
        panic("Didnt recieve a memmap");
    }

    // make sure we got hhdm or something
    if (unlikely(hhdm_request.response == NULL)){
        panic("Didnt recieve a hhdm");
    }
    // make sure we got a mp thing
    if (unlikely(mp_request.response == NULL)){
        panic("Didnt recieve a mp");
    }



    serial_init();
    InterruptInit();
    memory_init(memmap_request.response, hhdm_request.response);
    virtual_memory_init();
    user_space_init();
    ata_init();
    check_watchdog();



    printf("framebuffer %lux%lu pitch=%lu bpp=%u\n",framebuffer_request.response->framebuffers[0]->height,
        framebuffer_request.response->framebuffers[0]->width,
        framebuffer_request.response->framebuffers[0]->pitch,
        framebuffer_request.response->framebuffers[0]->bpp);



    struct limine_mp_response * mp = mp_request.response;



    for (uint64_t i = 0; i < mp->cpu_count; i++) {
        struct limine_mp_info *cpu = mp->cpus[i];

        if (cpu->apic_id != mp->bsp_id) {
            __atomic_store_n(
                &cpu->goto_address,
                ap_entry,
                __ATOMIC_RELEASE
            );
            printf("sending cpu %i to ap entry\n", i);
        }
    }






    kmain();


    cpu_stop(); // yo dont forget
}
