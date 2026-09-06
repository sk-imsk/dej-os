#include <stdint.h>

typedef struct file_fat32 file;
int disk_init(void);
int findfat_file(const char * fname);
