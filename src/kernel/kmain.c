#include <dej/kernel.h>
#include <dej/cpu.h>
#include <dej/limine.h>
#include <dej/framebuffer.h>


struct limine_framebuffer *framebuffer;
volatile uint32_t *fb_ptr;




_Noreturn void kmain(void){


    framebuffer = framebuffer_request.response->framebuffers[0];
    fb_ptr = framebuffer->address;


    uint64_t y = 0;
    while (y < framebuffer->height){
        for (uint64_t i = 0; i < framebuffer->width; i++){
            putpixel(i, y, 0x676767);
        }
        y++;
        for (uint64_t i = 0; i < framebuffer->width; i++){
            putpixel(i, y, 0xFF00FF);
        }
        y++;
    }

    cpu_takebreak();
    y = 0;



    cpu_stop();
}
