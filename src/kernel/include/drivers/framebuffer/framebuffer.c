/*
 *
 * most tuffest driver yet
 * now screen drawing is peak
 */

#include <dej/limine.h>
#include <dej/kernel.h>
#include <dej/string.h>
#include "framebuffer.h"

extern struct limine_framebuffer *framebuffer;
extern volatile uint32_t *fb_ptr;                       // get main to do our work lol

void framebuffer_init(){

}

/*
 * tuffest function ever
 * allowed at all irql lol
 *
 */
void putpixel(uint64_t x, uint64_t y, uint32_t colour){
    fb_ptr[y * (framebuffer->pitch / 4) + x] = colour;
}


void clearscreen(void){
    memset((void *)fb_ptr, 0, framebuffer->pitch * framebuffer->height);
}

struct limine_framebuffer * RequestFrameBuffer(uint8_t num){
    return framebuffer_request.response->framebuffers[num];
}


screen_t GetScreenInfo(uint8_t num){
    screen_t ret;

    ret.height = framebuffer[num].height;
    ret.pitch = framebuffer[num].pitch;
    ret.width = framebuffer[num].width;

    return ret;
}
