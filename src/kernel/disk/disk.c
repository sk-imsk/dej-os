#include <stdbool.h>
#include <stdint.h>
#include "../string.h"
#include "../x86.h"
#include "../stdio.h"


// io ports
#define DATA 0x1F0
#define ERROR  0x1F1
#define SECTOR_COUNT 0x1F2
#define LBA_LOW 0x1F3
#define LBA_MID 0x1F4
#define LBA_HIGH 0x1F5
#define DRIVE 0x1F6
#define HEAD 0x1F6
#define STATUS 0x1F7
#define COMMAND 0x1F7
#define ALT_STATUS 0x3F6
//timeout
#define ATA_TIMEOUT 67676           // haha so funny bro haha

#define VOLUME_START 2 // for gpt if your fat lowk change it or something


typedef union {
    uint16_t raw_buffer[256];
    // probab gonna add like a struct or something
}ata_identify_t;

struct file_fat32  {
    uint64_t lba;
    uint64_t length;
    uint16_t sector_count;
};

struct gpt {
  uint8_t sig[8];
  uint32_t rev;
  uint32_t header_size;
  uint32_t crc; // idk bro
  uint32_t : 32;    // reserve
  uint64_t cur_lba;
  uint64_t back_lba;
  uint64_t first_lba; // first usable lba
  uint64_t last_lba; // last usable
  uint64_t guid[2];
  uint64_t part_entry_lba;
  uint32_t part_entries;
  uint32_t entry_size;
  uint32_t crc_part;
  uint8_t reserved[420];
}__attribute__((__packed__));
_Static_assert(sizeof(struct gpt) == 512, "gpt struct incorrect size");

typedef union {
    uint8_t raw[512];
    struct gpt gpt_header;
}gpt_u;

struct gpt_part {
    uint64_t part_type[2];
    uint64_t uniq_part_gpt[2];
    uint64_t f_lba;
    uint64_t l_lba;
    uint32_t flags[2];
    uint8_t name[72];
}__attribute__((__packed__));
struct gpt_part_header {
    struct gpt_part part1;
    struct gpt_part part2;
    struct gpt_part part3;
    struct gpt_part part4;

}__attribute__((__packed__));
_Static_assert(sizeof(struct gpt_part_header) == 512, "gpt partition header wrong size");

typedef union {
    uint8_t raw[512];
    struct gpt_part_header gpt_p;
}gpt_hp;

static bool inited;
static bool _48bitlba;

// helpers
static inline int ata_wait_bsy(void)
{
    uint64_t tries = 0;
    uint8_t status;

    x86_inb(ALT_STATUS);
    x86_inb(ALT_STATUS);
    x86_inb(ALT_STATUS);
    x86_inb(ALT_STATUS); // wait like 400 ns or something


    do {
        status = x86_inb(STATUS);

        if (!(status & 0x80))
            return 0;

        if (++tries >= ATA_TIMEOUT)
            return -1;

        __asm__ volatile ("pause");
    } while (1);
}

static inline int ata_wait_drq(void)
{
    uint64_t tries = 0;
    uint8_t status;

    x86_inb(ALT_STATUS);
    x86_inb(ALT_STATUS);
    x86_inb(ALT_STATUS);
    x86_inb(ALT_STATUS); // wait 400 ns or something

    do {
        status = x86_inb(STATUS);

        if (status & 0x01)
            return -2; // ERR

        if (status & 0x08)
            return 0;  // DRQ

        if (++tries >= ATA_TIMEOUT)
            return -1;

        __asm__ volatile ("pause");
    } while (1);
}




static int ata_read_sector(uint64_t lba, void * buffer){



    uint16_t * buf = buffer;

   if (lba > 0x0000FFFFFFFFFFFFULL)  return -1;

   ata_wait_bsy();

   x86_outb(DRIVE, 0x40);

   // lba 48

   x86_outb(SECTOR_COUNT, 0);
   x86_outb(LBA_LOW,  (uint8_t)(lba >> 24));
   x86_outb(LBA_MID,  (uint8_t)(lba >> 32));
   x86_outb(LBA_HIGH, (uint8_t)(lba >> 40));


   /*
     * low bytes
    */

   x86_outb(SECTOR_COUNT, 1);
   x86_outb(LBA_LOW,  (uint8_t)lba);
   x86_outb(LBA_MID,  (uint8_t)(lba >> 8));
   x86_outb(LBA_HIGH, (uint8_t)(lba >> 16));


   x86_outb(COMMAND, 0x24); // read sectors lol or something


   ata_wait_bsy();

   ata_wait_drq();


   for (int i = 0; i < 256; i++) {
       buf[i] = x86_inw(DATA);
   }


   return 0;
}


int disk_init(void){
    uint8_t data;
    uint8_t status;
    uint32_t tries = 0;
    ata_identify_t ident;

    data = x86_inb(STATUS);

    if (data == 0xFF){
        return -1;
    }

    x86_outb(DRIVE, 0xA0); // master drive
    status = x86_inb(STATUS);
    if (!status) return false;
    do {
        tries++;
        status = x86_inb(STATUS);
    } while ((status & 0x80) && (tries > ATA_TIMEOUT)) ;
    if (tries > ATA_TIMEOUT) goto timeout;
    tries = 0;

    x86_outb(COMMAND, 0xEC); // identify device

    do {
        tries++;
        status = x86_inb(STATUS);
    } while ((status & 0x80) && (tries > ATA_TIMEOUT)) ; // busy
    if (tries > ATA_TIMEOUT) goto timeout;
    tries = 0;

    while (!(x86_inb(STATUS)& 0x08)){
        __asm__ volatile ("pause");
        tries++;
        if (tries > ATA_TIMEOUT) break;
    }
    tries = 0;

    for (int i = 0; i < 256; i++){
        ident.raw_buffer[i] = x86_inw(DATA);
    }

    if (!(ident.raw_buffer[49] & 0x0200))    return -2; // Too old

    if (!((ident.raw_buffer[83] & 0xC000) == 0x4000) ) return -1; // broken

    if (ident.raw_buffer[83] & 0x0400) _48bitlba = true;



    // find the start of the useful stuff

    gpt_u buf;
    uint64_t n_lba;
    if (ata_read_sector(VOLUME_START, buf.raw) != 0) return -1;
    if (!(strncmp((char *)buf.gpt_header.sig, "EFI PART", 8))) return 0; // cmp efi partition casting sig into a char *

    n_lba = buf.gpt_header.first_lba;

    serial_puts("Found gpt header");

    gpt_hp part_buf;
    if (ata_read_sector(n_lba, part_buf.raw) != 0) {
        printf("Failed to read %ull ", n_lba);
        return -1;
    }















    inited = true;
    return 0;


timeout:
    serial_puts("Time out.");
    return 100;
}
