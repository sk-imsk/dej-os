#include <stdint.h>

typedef struct {
    uint64_t width, height;
    uint64_t pitch;
}screen_t;


void putpixel(uint64_t x, uint64_t y, uint32_t colour);
void clearscreen(void);
struct limine_framebuffer * RequestFrameBuffer(uint8_t num);
screen_t GetScreenInfo(uint8_t num);
