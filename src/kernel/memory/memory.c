#include <dej/stdio.h>
#include <dej/panic.h>
#include <dej/kernel.h>
#include <dej/string.h>
#include "memory.h"
#include <dej/cpu.h>
#include <dej/sil.h>
#include <x86/tss.h>

#define USER_CODE  0x400000
#define USER_STACK 0x800000

static uint64_t pages = 0;
static bool inited = false;
struct page{
    uint64_t start;
    bool used;
};
typedef struct address_space {
    uint64_t pml4_phys;
    uint64_t *pml4;
} address_space_t;

static struct page page_list[10000];
static struct limine_hhdm_response * hhdm;


extern void load_gdt(void);

static inline always_inline uint64_t get_cr3(void)
{
    uint64_t cr3;

    __asm__ volatile (
        "mov %%cr3, %0"
        : "=r"(cr3)
    );

    return cr3;
}
static inline always_inline void write_cr3(uint64_t val){
    __asm__ volatile (
        "mov %0, %%cr3"
        :
        : "r"(val)
        : "memory"
    );
}


int memory_init(struct limine_memmap_response * memmap, struct limine_hhdm_response * _hhdm){
    hhdm = _hhdm;
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
            case LIMINE_MEMMAP_USABLE: {
                if (pages > 10000) break;
                uint64_t base = memmap->entries[i]->base;
                uint64_t length = memmap->entries[i]->length;


                // Align base UP to 4KiB boundary
                if (base & 0xFFF) {
                    uint64_t align_offset = 0x1000 - (base & 0xFFF);
                    if (length < align_offset) break;
                    base += align_offset;
                    length -= align_offset;
                }

                uint64_t page_count = length / KiB(4);

                for (uint64_t p = 0; p < page_count; p++) {
                    if (pages >= 10000) goto end;

                    uint64_t phys_addr = base + (p * KiB(4));

                    // RULE 1: Filter out low physical memory below 1MB
                    if (phys_addr < 0x100000) {
                        continue;
                    }

                    page_list[pages].start = phys_addr;
                    page_list[pages].used = false;
                    pages++;
                }
                break;
            }



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

static int map_page(address_space_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags)
{
    uint64_t pml4_i = (virt >> 39) & 0x1FF;
    uint64_t pdpt_i = (virt >> 30) & 0x1FF;
    uint64_t pd_i   = (virt >> 21) & 0x1FF;
    uint64_t pt_i   = (virt >> 12) & 0x1FF;

    uint64_t *pdpt;
    uint64_t *pd;
    uint64_t *pt;

    /*
     * PML4 -> PDPT
     */
    if (!(pml4->pml4[pml4_i] & PAGE_PRESENT)) {
        uint64_t page = __giverawpage();

        memset(phys2virt(page), 0, PAGE_SIZE);

        pml4->pml4[pml4_i] = page | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    pdpt = phys2virt(pml4->pml4[pml4_i] & ~0xFFFULL);

    /*
     * PDPT -> PD
     */
    if (!(pdpt[pdpt_i] & PAGE_PRESENT)) {
        uint64_t page = __giverawpage();

        memset(phys2virt(page), 0, PAGE_SIZE);

        pdpt[pdpt_i] = page | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    pd = phys2virt(pdpt[pdpt_i] & ~0xFFFULL);

    /*
     * PD -> PT
     */
    if (!(pd[pd_i] & PAGE_PRESENT)) {
        uint64_t page = __giverawpage();

        memset(phys2virt(page), 0, PAGE_SIZE);

        pd[pd_i] = page | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    pt = phys2virt(pd[pd_i] & ~0xFFFULL);

    /*
     * PT -> actual physical page
     */
    if (pt[pt_i] & PAGE_PRESENT) {
        return -1; // already mapped
    }

    pt[pt_i] = phys | flags;

    return 0;
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


int virtual_memory_init(void) {
    cpu_stop_interrupts();

    uint64_t new_pml4_phys = __giverawpage();

    // Safety check for alignment & low memory
    if (new_pml4_phys < 0x100000 || (new_pml4_phys & 0xFFF) != 0) {
        return -1; // Allocation failed or unaligned
    }

    uint64_t *new_pml4 = (uint64_t *)phys2virt(new_pml4_phys);

    uint64_t current_cr3 = get_cr3();
    uint64_t *boot_pml4 = (uint64_t *)phys2virt(current_cr3 & ~0xFFFULL);

    // 1. Copy ONLY higher-half mappings from boot PML4 (indices 256 to 511)
    memcpy(&new_pml4[256], &boot_pml4[256], 256 * sizeof(uint64_t));

    // 2. Explicitly zero out lower-half mappings (indices 0 to 255)
    memset(&new_pml4[0], 0, 256 * sizeof(uint64_t));


    // 3. Switch to the new page table
    write_cr3(new_pml4_phys);

    cpu_enable_interrupts();

    return 0;
}


void * u_buf;
uint64_t user_cr3;
static address_space_t user_as;
void user_space_init(void){


    user_as.pml4_phys = __giverawpage();
    user_as.pml4 = phys2virt(user_as.pml4_phys);
        // 1. Zero out lower half (user space, indices 0-255)
    memset(&user_as.pml4[0], 0, 256 * sizeof(uint64_t));

        // 2. Copy higher half from current active kernel PML4 (indices 256-511)
    uint64_t *current_pml4 = (uint64_t *)phys2virt(get_cr3() & ~0xFFFULL);
    memcpy(&user_as.pml4[256], &current_pml4[256], 256 * sizeof(uint64_t));

        // 3. Map user code and stack into lower half
    uint64_t page = __giverawpage();
    memset(phys2virt(page), 0, PAGE_SIZE);
    map_page(&user_as, USER_CODE, page, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

    uint64_t stack_page = __giverawpage();
    memset(phys2virt(stack_page), 0, PAGE_SIZE);
    map_page(&user_as, USER_STACK, stack_page, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

    user_cr3 = user_as.pml4_phys;
    u_buf = phys2virt(page);

    load_gdt();
    setupbspcpudata();
    tss_init();


}
