#include <dej/limine.h>
#include <dej/log.h>
#include <dej/kernel.h>
#include <dej/panic.h>



// types and stuff
struct RSDP_t {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;
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

static void parse_acpi_earlyboot_rev1(struct RSDP_t * rsdp){
    uint8_t accumulator = 0;
    uint8_t * bytes = (uint8_t *)rsdp;

    for (int i = 0; i < 20; i++){
        accumulator += bytes[i];
    }

    if (accumulator != 0){
        panic("Rsdp is invalid");
    }

    LogStr("Found acpi rsdp \n");





}

static void parse_acpi_earlyboot_rev2(struct XSDP_t * rsdp __unused){
    LogStr("using revision more than 2");
}


void parse_acpi_earlyboot(void){
    struct RSDP_t * __rsdP = rsdp_request.response->address;            // temp before we figure out which one it is

    if (__rsdP->Revision > 1) {
        parse_acpi_earlyboot_rev2((struct XSDP_t *)__rsdP);
    }
    else {
        parse_acpi_earlyboot_rev1(__rsdP);
    }
}
