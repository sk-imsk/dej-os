#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../string.h"
#include "../x86.h"
#include "../stdio.h"
#include "../memory/memory.h"


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


struct bpb {
    uint8_t jmp[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors;
    uint8_t media;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t sec_per_fat;
    uint16_t mirror_flags;
    uint16_t version;
    uint32_t root_dir_cluster;
    uint16_t l_fs_info_sector;
    uint16_t l_backup_sector;
    char reserved[12];
    uint8_t drive_number;
    uint8_t flags;
    uint8_t ebsig;
    uint32_t serial_number;
    char label[11];
    char fs_type[8];
    uint8_t padding[422];
}__attribute__((__packed__));
_Static_assert(sizeof(struct bpb) == 512,"bpb struct incorrect size");

typedef union {
    uint8_t raw[512];
    struct bpb bp;
}fat_bpb ;


struct direntry {
    uint8_t name[8]; // sfn
    char ext[3]; // file extenstion like .bin or somethign
    uint8_t flags;
    uint8_t reserved;
    uint8_t creation_time; // 10 ms res
    uint16_t c_time_HMS; // creation time hour/minute/second
    uint16_t c_date_YMD; // year month day
    uint16_t last_access;
    uint16_t high_word_first_cluster;
    uint16_t last_modify_time;
    uint16_t last_modify_date;
    uint16_t low_word_first_cluster;
    uint32_t size;
}__attribute__((__packed__));
_Static_assert(sizeof(struct direntry) == 32, "Dir entry incorrect size");

static bool inited;
static bool _48bitlba;
static uint64_t vol_start_lba;



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

static uint64_t clustertolba48(uint32_t cluster,
                         uint64_t volumeStartLBA,
                         uint16_t reservedSectors,
                         uint8_t numFATs,
                         uint32_t fatSizeSectors,
                         uint8_t sectorsPerCluster){
    uint64_t DataStartLba = volumeStartLBA + reservedSectors + ((uint64_t)numFATs * fatSizeSectors );

    uint64_t lba = DataStartLba + (((uint64_t) cluster -2) * sectorsPerCluster);


    return lba;

}

static uint8_t fat_toupper(uint8_t c)
{
    if (c >= 'a' && c <= 'z')
        return c - ('a' - 'A');

    return c;
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
    if (ata_read_sector(1, buf.raw) != 0) return -1;
    if (strncmp((char *)buf.gpt_header.sig, "EFI PART", 8) != 0) return 0; // cmp efi partition casting sig into a char *


    serial_puts("Found gpt header\n");

    gpt_hp part_buf;
    if (ata_read_sector(buf.gpt_header.part_entry_lba, part_buf.raw) != 0) {
        printf("Failed to read lba %ull ", buf.gpt_header.part_entry_lba);
        return -1;
    }

    vol_start_lba = part_buf.gpt_p.part2.f_lba;         // explicitly use partition 2 for my disk layout
    serial_puts("Found start of fat\n");


    inited = true;
    return 0;


timeout:
    serial_puts("Time out.");
    return 100;
}


int findfat_file(const char * fname){
    if (!inited) return -1;
    if (strnlen(fname, 12) == 12) return -1;
    void * cluster = givemeapage();



    uint8_t fat_name[11];
    memset(fat_name, ' ', 11);

    int x = 0;
    while (fname[x] != '.' && fname[x] != '\0' && x < 8) {
        fat_name[x] = fat_toupper(fname[x]);
        x++;
    }

    /* skip '.' */
    if (fname[x] == '.')
        x++;

    /* copy extension */
    int y = 0;
    while (fname[x] != '\0' && y < 3) {
        fat_name[8 + y] = fat_toupper(fname[x]);
        x++;
        y++;
    }

    fat_bpb bpb;
    if (ata_read_sector(vol_start_lba, bpb.raw) != 0){
        serial_puts("File read failed");
        return -1;
    }

    if (bpb.raw[510] == 0x55 && bpb.raw[511] == 0xAA) printf("Got fat bpb\n");


    uint64_t sector_lba = clustertolba48(bpb.bp.root_dir_cluster,
                                                vol_start_lba,
                                                bpb.bp.reserved_sectors,
                                                bpb.bp.num_fats,
                                                bpb.bp.sec_per_fat,
                                                bpb.bp.sectors_per_cluster);



    for (int i = 0; i < bpb.bp.sectors_per_cluster; i++){
        ata_read_sector(sector_lba + i, (uint8_t *)cluster + (i * 512));
    }

    /*
     * Read directory entries
     */
    uint64_t off = 0;
    struct direntry * dir;


    while (off < (uint64_t)bpb.bp.sectors_per_cluster * bpb.bp.bytes_per_sector){
        dir = (struct direntry *) ((uint8_t *)cluster + off);

        if (dir->name[0] == 0x00) goto NotFound;
        if (dir->name[0] == 0xE5){ off+= sizeof(struct direntry); continue;}
        if (dir->flags == 0x0F) { off+= sizeof(struct direntry); continue;}

        off+= sizeof(struct direntry);


        if (memcmp(dir->name, fat_name, 11)) goto Found; // read past the end of le buffer intentional

        }




Found:

    printf("Found file \n");


    retpage(cluster);
    return 0;


NotFound:
    retpage(cluster);
    return 2;
}
