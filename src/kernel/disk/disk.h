#include <stdint.h>

typedef struct file_fat32 file;
int disk_init(void);
int ata_read_sector(uint64_t lba, void * buffer);
