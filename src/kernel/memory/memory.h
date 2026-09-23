#include <limine.h>
#include <dej/kernel.h>
int memory_init(struct limine_memmap_response * memmap, struct limine_hhdm_response * hhdm);
void * givemeapage();
void retpage(void * ptr);
bool vm_map(uint64_t * pm14, uint64_t virt, uint64_t phys, uint64_t flags);
bool vm_unmap(uint64_t * pm14, uint64_t phys);
