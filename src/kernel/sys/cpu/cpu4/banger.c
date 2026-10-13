/*
 *
 * banger.c only plays bangers
 * if found not playing a banger report it to me <oven@oven.ninja> and i will take care of it
 *
 * plays banger music all day
 *
 *
 */
#include <dej/cpu.h>
#include <dej/log.h>
#include <drivers/acpi/pcie.h>
#include <dej/memory.h>
#include <drivers/hda/hda.h>
#include <dej/percpu.h>
#include <dej/memory.h>
#include <dej/sil.h>

static const char mod[] = "banger";
struct pci_device sound_dev = {0};


_Noreturn void sound_entry(){

	LogfStr(mod, "Audio device found vendor id = %lu device id = %lu \n", sound_dev.vendor_id, sound_dev.device_id);

	volatile uint64_t mmio_raw = sound_dev.bar[0];
	volatile uint8_t * mmio = map_mmio(mmio_raw);
	volatile uint32_t * gctl = (volatile uint32_t *)mmio + HDA_GCTL;

	int res = sound_dev_reset(&sound_dev, gctl);

	if (res) {
		LogfStr(mod, "reset device failed with ec %i halting on core %i \n", res, percpu_read(cpu_id));
	}

	uint8_t corbsize = mmio_read8(mmio + HDA_CORBSIZE);
	uint8_t rirbsize = mmio_read8(mmio + HDA_RIRBSIZE);

	size_t corb_size = hda_getring_size(corbsize)* sizeof(uint32_t);
	size_t rirb_size = hda_getring_size(rirbsize) * sizeof(uint64_t);

	if (!corb_size || !rirb_size) {
		LogfStr(mod, "Unsupported corb/rirp configuration halting on core %i", percpu_read(cpu_id));
	}

	buffers_t * buffers = KGetPage();
	memset(buffers, 0, PAGE_SIZE);

	res = sound_init(buffers, &sound_dev, mmio);

	if (res) {
		LogfStr(mod, "sound init returned %i \n", res);
	}


	LogStr(mod, "Time out.\n");

	for (int i = 0; i < 10; i++){
		LogStr("dej", "I will not write bad code\n");
	}

	LogStr("dej", "whoever said i will not write bad code is lying all the code is bad\n");


	cpu_stop();
}
