#include <drivers/acpi/pcie.h>
#include <dej/driver.h>
#include <dej/log.h>

static const char mod[] = "generic achi";

static bool attach(device_t * dev __unused){
	LogStr(mod,"Attached successfully\n");
	return true;
}

static bool detach(device_t * dev __unused){
	return true;
}

static bool probe(device_t * dev){
	struct pci_device * pci = &dev->device;


	if (pci->class == 0x01 && pci->subclass == 0x06 && pci->prog_if == 0x01){
		return true;
	}

	if (pci->class == 0x01 && pci->subclass == 0x06 && pci->prog_if == 0x00){
		LogStr(mod, "ahci device is not normal (banned)\n");
		return false;
	}

	return false;
}

static driver_t driver = {
	.name = "Generic dej achi driver",
	.attach = attach,
	.detach = detach,
	.probe = probe,
};

void __dih achi_init(void){
	RegisterDriver(&driver);
	LogStr(mod, "Registered driver succesfully\n");
	return;
}
