#include "../lim/limine.h"
#include "../stdio.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "../panic.h"

#define KiB(x) x * 1024
#define KB(x) x * 1000
#define MB(x) KB(x) * 1000

static char buffer[100];
static uint64_t pages = 0;
static bool inited = false;
struct page{
    uint64_t start;
    bool used;
};
static struct page page_list[10000];
static struct limine_hhdm_response * hhdm;

int memory_init(struct limine_memmap_response * memmap, struct limine_hhdm_response * _hhdm){
    hhdm = _hhdm;
    uint64_t amount;
    uint64_t add = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++){

        switch (memmap->entries[i]->type){
            case LIMINE_MEMMAP_RESERVED_MAPPED:
                break;
            case LIMINE_MEMMAP_FRAMEBUFFER:
                break;
            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                // basically i aint gonna use yet but later sure
                break;
            case LIMINE_MEMMAP_BAD_MEMORY:
                serial_puts("Bad memory at");
                uint64_to_hex(memmap->entries[i]->base, buffer);
                serial_puts(buffer);
                serial_puts("\n");
                break;
            case LIMINE_MEMMAP_ACPI_NVS:
                break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                break;
            case LIMINE_MEMMAP_RESERVED:
                break;
            case LIMINE_MEMMAP_USABLE:
                if (pages >= 10000) break;

                amount = (uint64_t)memmap->entries[i]->length / KiB(4);
                while (amount != 0){
                   if (pages >= 10000) goto end;
                   page_list[pages].start = memmap->entries[i]->base + add;
                   pages++;
                   add += KiB(4);
                   amount--;
                }

                add = 0;
                break;



        }
    }
end:
    inited = true;
    return 0;
}

// returns pointer to 4 Kib page
void * givemeapage(){

    uint32_t i;
    for (i = 0; i < pages; i++){
        if (page_list[i].used == false) break;
    }
    if (i == pages) return NULL;
    page_list[i].used = true;

    return (void *)(page_list[i].start + hhdm->offset);
}


// must be from givemepage or else ill take down the system
void retpage(void * ptr){
   uint32_t i;
   ptr = ptr - hhdm->offset;
   for (i = 0; i < pages; i++){
       if (page_list[i].start == (uint64_t)ptr){
            if (page_list[i].used == true){
                page_list[i].used = false;
                return;
            }
            else {
                panic("Attemped deallocation of unused page");
            }
       }


   }
   panic("Attempted deallocation of nonexistent page");
}
