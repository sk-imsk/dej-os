#include <dej/limine.h>
#include <dej/log.h>
#include <dej/kernel.h>
#include <dej/panic.h>
#include <dej/string.h>
#include "generic.h"
#include "pcie.h"



// types and stuff
struct RSDP_t {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;
} __attribute__ ((packed));


struct RSDT {
  RSDT_t header;
  uint32_t entries[];
} __attribute__ ((packed));

// rev 2 or something
struct XSDP_t {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;      // deprecated since version 2.0

 uint32_t Length;
 uint64_t XsdtAddress;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} __attribute__ ((packed));



static bool check_rsdpsum(struct RSDP_t * r){
    uint8_t * bytes = (uint8_t *)r;
    uint8_t acc = 0;

    for (uint8_t i = 0; i < 20; i++ ){
        acc += bytes[i];
    }

    return acc == 0;
}

static void *find_x(struct RSDT *rsdt, const char * s)
{
    int entries = (rsdt->header.Length - sizeof(rsdt->header)) / 4;

    for (int i = 0; i < entries; i++)
    {
        struct SDT_header *h = (struct SDT_header *) (rsdt->entries[i] + hhdm_request.response->offset);
        if (!strncmp(h->Signature, s, 4))
            return (void *)h;
    }

    return NULL;
}


static void parse_acpi_earlyboot_rev1(struct RSDP_t * rsdp){

    LogStr("Reading acpi1 tables \n");

    if (!(check_rsdpsum(rsdp))){
        panic("rsdp invalid", STATUS_HARDWARE_FAILURE);
    }
    LogStr("Found acpi rsdp \n");


    struct RSDT * rsdt = (struct RSDT *)(phys2virt32(rsdp->RsdtAddress));

    if (!(check_genericsum(&rsdt->header))) {
    panic("rsdt invalid", STATUS_HARDWARE_FAILURE);
    }

    LogStr("Found valid rsdt\n");


    void * mcfg = find_x(rsdt, "MCFG");
    if (mcfg != NULL){
        mcfg_enter(mcfg);
    } else {
        LogStr("Failed to find MCFG\n");
    }







}

static void parse_acpi_earlyboot_rev2(struct XSDP_t * rsdp __unused){
    LogStr("using revision more than 2");
}


void parse_acpi_earlyboot(void){
    struct RSDP_t * rsdP = rsdp_request.response->address;            // temp before we figure out which one it is

    if (rsdP->Revision > 1) {
        parse_acpi_earlyboot_rev2((struct XSDP_t *)rsdP);
    }
    else {
        parse_acpi_earlyboot_rev1(rsdP);
    }
}
