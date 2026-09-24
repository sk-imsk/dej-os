#include <dej/stdio.h>
#include <dej/panic.h>
#include <dej/kernel.h>
#include <dej/string.h>
#include "memory.h"

static uint64_t pages = 0;
static bool inited = false;
struct page{
    uint64_t start;
    bool used;
};
static struct page page_list[10000];
static struct limine_hhdm_response * hhdm;
static raw_page pml4;
static raw_page pdpt;
static raw_page pt;
static raw_page pd;
static uint64_t * Vpml4;
static uint64_t * Vpdpt;
static uint64_t * Vpt;
static uint64_t * Vpd;



int memory_init(struct limine_memmap_response * memmap, struct limine_hhdm_response * _hhdm){
    printf("Page tables or something ");
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
                printf("Bad memory at %llu ", memmap->entries[i]->base);
                printf("Length %llu \r\n", memmap->entries[i]->length);
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

static inline always_inline void * phys2virt(raw_page addr){
    return (void *)(addr + hhdm->offset);
}
// returns pointer to 4 Kib page
void * KGetPage(){

    uint32_t i;
    for (i = 0; i < pages; i++){
        if (page_list[i].used == false) break;
    }
    if (i == pages) return NULL;
    page_list[i].used = true;

    return (void *)(page_list[i].start + hhdm->offset);
}

// tuffer lowk cuz you dont have to return it
// but also like only 1 guy is gonna use it ever so lowk nah
raw_page __giverawpage(){
    uint32_t i;
    for (i = 0; i < pages; i++){
        if (page_list[i].used == false) break;
    }
    if (i == pages) return 0;
    page_list[i].used = true;

    return (uint64_t)(page_list[i].start);
}

// must be from givemepage or else ill take down the system
void retpage(void * ptr){
   uint32_t i;
   ptr = (char *) ptr - hhdm->offset;
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


int virtual_memory_init(void){
    pml4 = __giverawpage();
    pdpt = __giverawpage();
    pt = __giverawpage();
    pd = __giverawpage();
    Vpml4 = phys2virt(pml4);
    Vpdpt = phys2virt(pdpt);
    Vpt = phys2virt(pt);
    Vpd = phys2virt(pd);

    memset(Vpml4, 0, PAGE_SIZE);
    memset(Vpdpt, 0, PAGE_SIZE);
    memset(Vpt, 0, PAGE_SIZE);
    memset(Vpd, 0, PAGE_SIZE);


    Vpml4[0] = pdpt | PAGE_PRESENT | PAGE_WRITE;
    Vpdpt[0] = pd   | PAGE_PRESENT | PAGE_WRITE;
    Vpd[0]   = pt   | PAGE_PRESENT | PAGE_WRITE;

    uint64_t page = __giverawpage();
    Vpt[0] = page | PAGE_PRESENT | PAGE_WRITE;






    return 0;
}

bool vm_map(uint64_t * pm14, uint64_t virt, uint64_t phys, uint64_t flags){
    if (unlikely(!inited)) return ENXIO;
    (void)pm14;
    (void)phys;
    (void)virt;
    (void)flags;

    return false;
}

bool vm_unmap(uint64_t * pm14, uint64_t phys){
    if (unlikely(!inited)) return ENXIO;
    (void)pm14;
    (void)phys;
    return false;
}
