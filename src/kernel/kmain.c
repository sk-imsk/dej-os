#include <dej/kernel.h>
#include <dej/cpu.h>
#include <dej/limine.h>
#include <drivers/framebuffer/framebuffer.h>
#include <dej/panic.h>
#include <dej/driver.h>
#include <dej/watchdog.h>


struct limine_framebuffer *framebuffer;
volatile uint32_t *fb_ptr;

void set_up_framebuffer(void){
	framebuffer = framebuffer_request.response->framebuffers[0];
	fb_ptr = framebuffer_request.response->framebuffers[0]->address;
}

extern _Atomic uint64_t temperature;

void test_probe(struct pci_device * dih __unused){
	LogStr("driver loaded");
}


_Noreturn void kmain(void){

	set_up_framebuffer();

	for (uint32_t x = 0; x < framebuffer->width; x++){
		for (uint32_t y = 0; y < framebuffer->height; y++){
			putpixel(x, y, 0x00FFFF);
		}
	}

	jump_to_watchdog();
}
