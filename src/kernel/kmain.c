#include <dej/kernel.h>
#include <dej/cpu.h>
#include <dej/limine.h>
#include <dej/framebuffer.h>
#include <dej/panic.h>
#include <dej/driver.h>


struct limine_framebuffer *framebuffer;
volatile uint32_t *fb_ptr;


extern _Atomic uint64_t temperature;

void test_probe(struct pci_device * dih __unused){
	LogStr("driver loaded");
}

_Noreturn void kmain(void){
    framebuffer = framebuffer_request.response->framebuffers[0];
    fb_ptr = framebuffer->address;

    uint32_t high, low;

    // Seed our random state with the initial temperature
    uint64_t rnd_state = atomic_load(&temperature);

    for (uint64_t y = 0; y < framebuffer->height; y++) {
        // Step by 2 pixels at a time since we get 2 colors (low and high) per 64-bit scramble
        for (uint64_t i = 0; i < framebuffer->width; i += 2) {

            // 1. Mix in the latest temperature so it changes over time
            rnd_state ^= atomic_load(&temperature);

            // 2. Scramble the state using common PRNG multiplier and increment constants
            rnd_state = rnd_state * 6364136223846793005ULL + 1442695040888963407ULL;

            // 3. Split into two 32-bit color values
            low  = (uint32_t)(rnd_state & 0xFFFFFFFFUL);
            high = (uint32_t)(rnd_state >> 32);

            // 4. Draw the two pixels side-by-side
            putpixel(i, y, low);
            if (i + 1 < framebuffer->width) {
                putpixel(i + 1, y, high);
            }
        }
    }

    cpu_takebreak();

    struct pci_driver sdrv = {
        .class = 0x03,
        .subclass = 0x00,
        .prog_if = 0x00,
        .vendor_id = 0x1234,
        .device_id = 0x1111,

        .probe = test_probe,
    };

    RegisterPcieDriver(&sdrv);
    cpu_stop();
}
