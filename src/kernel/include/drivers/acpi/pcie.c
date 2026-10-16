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

static const char mod[] = "acpi";

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


device_type_t getdevtype(uint8_t class, uint8_t sub){
	switch (class){
		case 0x01:
			return DEV_TYPE_CONTROLLER;
		case 0x02:
			return DEV_TYPE_CONTROLLER;
		case 0x03:
			return DEV_TYPE_CONTROLLER;
		case 0x04:
			return 	DEV_TYPE_CONTROLLER;
		case 0x0C: {
			if (sub == 0x03) return DEV_TYPE_CONTROLLER;
			return DEV_TYPE_BUS;
		}
		case 0x06:
			return DEV_TYPE_BUS;
		case 0x09:
			return DEV_TYPE_INPUT;
		default:
			LogfStr(mod, "Couldent find a device type for class = %x subclass = %x \n", class, sub);
			return DEV_TYPE_CONTROLLER;

	}
}

void get_drivers(void){

	device_t probe_device;
	struct pci_device probe_pci;
	for (uint16_t bus = 0; bus < 256; bus++){
		for (uint8_t device = 0; device < 32; device++) {
    			for (uint8_t function = 0; function < 8; function++) {


       				volatile uint8_t * cfg = pci_config(bus, device, function);

           			uint16_t vendor_id = *(volatile uint16_t *)(cfg + PCI_VENDOR_ID);

              			if (vendor_id == 0xFFFF) continue;

                 		probe_pci.class = *(volatile uint16_t *)(cfg + PCI_CLASS);
                   		probe_pci.subclass = *(volatile uint16_t *)(cfg + PCI_SUBCLASS);
                     		probe_pci.bus = bus;
                       		probe_pci.device = device;
                         	probe_pci.function = function;
                          	probe_pci.vendor_id = *(volatile uint16_t *)(cfg + PCI_VENDOR_ID);
                           	probe_pci.device_id = *(volatile uint16_t *)(cfg + PCI_DEVICE_ID);
                            	probe_pci.prog_if = *(volatile uint16_t *)(cfg + PCI_PROGIF);
                             	probe_pci.header_type = *(volatile uint16_t *)(cfg + PCI_HEADER_TYPE);
                              	probe_pci.bar[0] = 0x00;


                 		probe_device.type = getdevtype(probe_pci.class, probe_pci.subclass);
                     		probe_device.device = probe_pci;

                       		bool driver = false;
                      		for (int i = 0; i < driver_count; i++){
                        		if (registered_drivers[i]->probe(&probe_device)){
                          			if (registered_drivers[i]->attach(&probe_device)) {
                             				driver = true;
                             				continue;
                             			}
                          		}
                        	}
                        	if (!driver) LogfStr(mod, "Couldent find a driver for class %x subclass %x\n", probe_pci.class, probe_pci.subclass);


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

        LogfStr(mod, "MCFG entry %u : \n", i);
        LogfStr(mod, "base: %p \n", (void *)entry->base_address);
        LogfStr(mod, "segment:  %u \n", entry->segment_group);
        LogfStr(mod, "Buses : %u-%u \n", entry->start_bus, entry->end_bus);
    }


    get_drivers();


}
