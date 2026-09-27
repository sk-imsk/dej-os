#include <limine.h>
#include <dej/kernel.h>


typedef uint64_t page_entry_t;
typedef uint64_t raw_page;

#define PAGE_PRESENT  (1ULL << 0)
#define PAGE_WRITE    (1ULL << 1)
#define PAGE_USER     (1ULL << 2)
#define PAGE_NX       (1ULL << 63)

#define PAGE_SIZE KiB(4)



int memory_init(struct limine_memmap_response * memmap, struct limine_hhdm_response * hhdm);
int virtual_memory_init(void);
void * KGetPage(void);
void retpage(void * ptr);
void user_space_init(void);
void load_gdt(void);
