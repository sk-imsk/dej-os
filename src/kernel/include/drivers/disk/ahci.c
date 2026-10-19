#include <drivers/acpi/pcie.h>
#include <dej/driver.h>
#include <dej/log.h>
#include <dej/memory.h>
#include <dej/sleep.h>
#include "achi.h"

static const char mod[] = "generic ahci";
static volatile void * mmio;

#define abar 5

static int turn_off_port(int i){
	uint32_t new  = mmio_read32(mmio + AHCI_PORT_BASE(i) + AHCI_PxCMD);


	if (!(new & (AHCI_PXCMD_ST | AHCI_PXCMD_CR | AHCI_PXCMD_FRE | AHCI_PXCMD_FR))){
		return 0;		// already idle
	}

	new &= ~AHCI_PXCMD_ST;
	mmio_write32(AHCI_PORT_BASE(i) + AHCI_PxCMD + mmio, new);

	uint32_t timeout = 500;

	while (timeout--){
		new = mmio_read32(mmio + AHCI_PORT_BASE(i) + AHCI_PxCMD);

		if (!(new & AHCI_PXCMD_CR)) break;

		sleep_ms(1);
	}
	if (timeout == 0) return -1;

	new &= ~AHCI_PXCMD_FRE;			// fre
	mmio_write32(AHCI_PORT_BASE(i) + AHCI_PxCMD + mmio, new);

	while (timeout--){
		new = mmio_read32(mmio + AHCI_PORT_BASE(i) + AHCI_PxCMD);

		if (!(new & AHCI_PXCMD_FR)) break;

		sleep_ms(1);
	}
	if (timeout == 0) return -1;

	return 0;

}

static int wake_up_ports(uint32_t ports){

	for (size_t i = 0; i < 32; i++){
		if (ports & (1 << i)){
			int res = turn_off_port(i);

			if (res){
				LogfStr(mod, "failed to turn off port %i ", i);
				return res;
			}

		}
	}

	return 0;
}



static bool attach(device_t * dev){
	mmio = map_mmio(dev->device.bar[abar]);

	uint32_t ports = mmio_read32(mmio + AHCI_PI);

	int res = wake_up_ports(ports);

	if (res) return false;			// failed


	LogStr(mod, "Attached to controller succesfully\n");
	return true;
}

static bool detach(device_t * dev __unused){
	mmio = NULL;				// bug if still trying to access mmio also will add unmap mmio but not made yet

	return true;
}

static bool probe(device_t * dev){
	struct pci_device * pci = &dev->device;


	if (pci->class == 0x01 && pci->subclass == 0x06 && pci->prog_if == 0x01){
		return true;
	}

	if (pci->class == 0x01 && pci->subclass == 0x06 && pci->prog_if == 0x00){
		LogStr(mod, "ahci device is not normal (banned)(prog if = 0) \n");
		return false;
	}

	return false;
}

static driver_t driver = {
	.name = "Generic dej ahci controller driver",
	.attach = attach,
	.detach = detach,
	.probe = probe,
};

void __dih achi_init(void){
	RegisterDriver(&driver);
	LogStr(mod, "Registered driver succesfully\n");
	return;
}
