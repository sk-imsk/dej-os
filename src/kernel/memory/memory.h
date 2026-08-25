#include "../lim/limine.h"
int memory_init(struct limine_memmap_response * memmap, struct limine_hhdm_response * hhdm);
void * givemeapage();
void retpage(void * ptr);
