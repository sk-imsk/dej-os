#pragma once
#include <stdint.h>

struct file_fat32  {
    uint64_t first_cluster;
    uint64_t size;
    uint8_t attr;
};

int ata_init(void);
struct file_fat32 fat_open(const char * path);
int fat_read(struct file_fat32  fat, void * buffer);
