#include <dej/driver.h>
#include <dej/panic.h>
#include <dej/cpu.h>
#include <dej/log.h>
#include <drivers/acpi/pcie.h>
#include <dej/limine.h>
#include "hda.h"

static inline always_inline uint64_t virt2phys(void * addr){
    return (uint64_t)(addr - hhdm_request.response->offset);
}

static inline void delay(uint32_t d){
	while (d--) cpu_takebreak();
}

int sound_dev_reset(struct pci_device * dev, volatile uint32_t * gctl){
	if (dev->vendor_id != 0x8086){
		panic("Intel Hda driver: Device recieved was not intel", STATUS_INVALID);
	}


	LogStr("Reseting hda controller\n");

	*gctl &= ~HDA_GCTL_CRST;

	uint32_t timeout = 10000;

	while ((*gctl & HDA_GCTL_CRST) != 0) {
		if (--timeout == 0){
			LogStr("hda failed to reset (stuck on ) \n");
			return ENXIO;
		}
		delay(10);
	}

	delay(1000);

	*gctl |= HDA_GCTL_CRST;			// turn back on



	timeout = 10000;


	while ((*gctl & HDA_GCTL_CRST) == 0) {
		if (--timeout == 0){
			LogStr("hda failed to turn back on\n");
			return ENXIO;
		}
		delay(10);
	}



	delay(1000);


	return 0;
}

uint16_t hda_getring_size(uint8_t caps){
	if (caps & (1 << 6)) return 256;
	if (caps & (1 << 5)) return 16;
	if (caps & (1 << 4))return 2;

    	return 0; // controller supports nothing usable
}

int sound_init(buffers_t * buf, struct pci_device * dev, volatile uint8_t * mmio){

	if (dev->vendor_id != 0x8086) return EINVAL;

	mmio_write8(mmio + HDA_CORBCTL, 0);
	mmio_write8(mmio + HDA_RIRBCTL, 0);

	uint64_t raw = virt2phys(buf);

	mmio_write32(mmio + HDA_CORBLBASE, raw);
	mmio_write32(mmio + HDA_CORBUBASE, (raw + 1024) >> 32);			// give corb buffer

	mmio_write32(mmio + HDA_RIRBLBASE, raw + 1024);
	mmio_write32(mmio + HDA_RIRBUBASE, raw >> 32);					// give corb buffer

	mmio_write16(mmio + HDA_CORBRP, 0x8000);

	uint32_t timeout = 10000;

	while (!(mmio_read16(mmio + HDA_CORBRP) & 0x8000)) {
		if (--timeout == 0){
			LogStr("corbrp failed to move to the start\n");
			return ENXIO;
		}
		delay(10);
	}



	mmio_write16(mmio + HDA_CORBRP, 0);
	mmio_write16(mmio + HDA_RIRBWP, 0x8000);

	mmio_write8(mmio + HDA_CORBCTL, 0x02);
	mmio_write8(mmio + HDA_RIRBCTL, 0x02);

	LogStr("corbrp reset succesfully\n");

	return 0;
}
