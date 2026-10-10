#include <drivers/acpi/pcie.h>
#include <dej/driver.h>
#include <dej/log.h>



static bool attach(device_t * dev){
	return true;
}

static bool detach(device_t * dev){
	return true;
}

static bool probe(device_t * dev){
	struct pci_device * pci = dev->bus_context;


	if (pci->class == 0x01 && pci->subclass == 0x06 && pci->prog_if == 0x01){
		return true;
	}

	if (pci->class == 0x01 && pci->subclass == 0x06 && pci->prog_if == 0x00){
		LogStr("Sata driver is propietary (banned)\n");
		return false;
	}

	return false;
}

driver_t driver = {
	.name = "Generic dej sata driver",
	.attach = attach,
	.detach = detach,
	.probe = probe,
};
