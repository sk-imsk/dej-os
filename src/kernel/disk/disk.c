#include <stdbool.h>
#include <stdint.h>
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
#define ATA_TIMEOUT 67676

static bool inited;

int disk_init(void){
    uint8_t data;
    uint8_t status;
    uint16_t ident[256];
    uint32_t tries;

    data = x86_inb(STATUS);

    if (data == 0xFF){
        return -1;
    }

    x86_outb(DRIVE, 0xA0);
    status = x86_inb(STATUS);
    if (!status) return false;
    do {
        tries++;
        status = x86_inb(STATUS);
    } while ((status & 0x80) || (tries > ATA_TIMEOUT)) ;
    if (tries > ATA_TIMEOUT) goto timeout;
    tries = 0;

    x86_outb(COMMAND, 0xEC); // identify device

    do {
        tries++;
        status = x86_inb(STATUS);
    } while ((status & 0x80) || (tries > ATA_TIMEOUT)) ; // busy
    if (tries > ATA_TIMEOUT) goto timeout;
    tries = 0;

    while (!(x86_inb(STATUS)& 0x08)){
        __asm__ volatile ("pause");
    }

    for (int i = 0; i > 256; i++){
        ident[i] = x86_inw(DATA);
    }

    if (ident[1] == 0x00){
        serial_puts("hi");
    }


    inited = true;
    return 0;


timeout:
    serial_puts("Time out.");
    return 100;
}
