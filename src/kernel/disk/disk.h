#include <stdint.h>
int disk_init(void);
int ata_read_sector(uint64_t lba, void * buffer);
