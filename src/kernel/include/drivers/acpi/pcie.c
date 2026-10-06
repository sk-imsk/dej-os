#include <dej/kernel.h>
#include <dej/log.h>
#include <dej/driver.h>
#include "generic.h"
#include "pcie.h"
#include <dej/memory.h>

#define PCI_VENDOR_ID   0x00
#define PCI_DEVICE_ID   0x02
#define PCI_BAR 0x10
#define PCI_CLASS       0x0B
#define PCI_SUBCLASS    0x0A
#define PCI_PROGIF      0x09
#define PCI_HEADER_TYPE 0x0E


extern struct pci_device sound_dev;

typedef struct SDT_header mcfg_header_t;

typedef struct MCFG_entry {
    uint64_t base_address;
    uint16_t segment_group;
    uint8_t start_bus;
    uint8_t end_bus;
    uint32_t reserved;
} __attribute__((packed)) MCFG_entry_t;

struct mcfg {
    mcfg_header_t header;
    uint64_t reserved;
    MCFG_entry_t entries[];
}__attribute__((packed));



volatile uint8_t *pci_config(
    uint8_t bus,
    uint8_t device,
    uint8_t function)
{
    uint64_t offset =
        ((uint64_t)bus << 20) |
        ((uint64_t)device << 15) |
        ((uint64_t)function << 12);

    return (volatile uint8_t *)(PCI_ECAM_BASE + offset);
}

uint64_t pci_read_bar(
    uint8_t bus,
    uint8_t device,
    uint8_t function,
    uint8_t bar_num)
{
    volatile uint8_t *config = pci_config(bus, device, function);

    uint32_t low =
        *(volatile uint32_t *)(config + 0x10 + bar_num * 4);

    // I/O BAR
    if (low & 1) {
        return (uint64_t)(low & ~0x3U);
    }

    // Memory BAR
    uint8_t type = (low >> 1) & 3;

    // 64-bit memory BAR
    if (type == 2) {
        uint32_t high =
            *(volatile uint32_t *)(config + 0x10 + (bar_num + 1) * 4);

        return ((uint64_t)high << 32) |
               (uint64_t)(low & ~0xFULL);
    }

    // 32-bit memory BAR
    return (uint64_t)(low & ~0xFULL);
}



void get_driver(void){
	struct pci_device ret;
	for (uint16_t bus = 0; bus < 256; bus++){
		for (uint8_t device = 0; device < 32; device++) {
    			for (uint8_t function = 0; function < 8; function++) {


       				volatile uint8_t * cfg = pci_config(bus, device, function);



         			ret.vendor_id = *(volatile uint16_t *)(cfg + PCI_VENDOR_ID);


          			if (ret.vendor_id == 0xffff)
              			continue;

            			ret.device_id = *(volatile uint16_t *)(cfg + PCI_DEVICE_ID);

              			ret.class = *(volatile uint8_t *)(cfg + PCI_CLASS);

                		ret.subclass = *(volatile uint8_t *)(cfg + PCI_SUBCLASS);

                 		ret.prog_if = *(volatile uint8_t *)(cfg + PCI_PROGIF);

                  		ret.header_type = *(volatile uint8_t *)(cfg + PCI_HEADER_TYPE);


                      		for (int i = 0; i < 6; i++) {
                        		ret.bar[i] = pci_read_bar(bus, device,function, i);
                        	}


                    		// find driver or something
                      		if (ret.class == 4 && ret.subclass == 3){
                       			sound_dev = ret;
                         		LogfStr("Sound device found\n");
                        	}


            		}
		}
	}

}



void mcfg_enter(void * mcfg ){
    struct mcfg * mh = (struct mcfg *)mcfg;


    check_genericsum(&mh->header);

    size_t entry_count =
        (mh->header.Length
         - sizeof(mh->header)
         - sizeof(mh->reserved))
        / sizeof(MCFG_entry_t);


    for (size_t i = 0; i < entry_count; i++){
        MCFG_entry_t * entry = &mh->entries[i];

        LogfStr("MCFG entry %u : \n", i);
        LogfStr("base: %p \n", (void *)entry->base_address);
        LogfStr("segment:  %u \n", entry->segment_group);
        LogfStr("Buses : %u-%u \n", entry->start_bus, entry->end_bus);
    }


    get_driver();


}
