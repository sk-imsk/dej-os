#include <dej/driver.h>

#define HDA_GCAP   0x00
#define HDA_GCTL   0x08
#define HDA_STATE  0x0E
#define HDA_WAKEEN 0x0C

#define HDA_CORBLBASE 0x40
#define HDA_CORBUBASE 0x44
#define HDA_CORBWP    0x48
#define HDA_CORBRP    0x4A
#define HDA_CORBCTL  0x4C
#define HDA_CORBSTS  0x4D
#define HDA_CORBSIZE 0x4E

#define HDA_RIRBLBASE 0x50
#define HDA_RIRBUBASE 0x54
#define HDA_RIRBWP    0x58
#define HDA_RINTCNT   0x5A
#define HDA_RIRBCTL   0x5C
#define HDA_RIRBSTS  0x5D
#define HDA_RIRBSIZE 0x5E

#define HDA_GCTL_CRST         (1U << 0)

struct hda_ring {
    void *virt;
    uint64_t phys;
    uint16_t entries;
};

typedef struct {
    uint32_t corb[256];
    uint64_t rirb[256];
	uint8_t padding[1024];
}__attribute__((packed)) buffers_t;



int sound_dev_reset(struct pci_device * dev, volatile uint32_t * gctl);
uint16_t hda_getring_size(uint8_t caps);
int sound_init(buffers_t * buf, struct pci_device * dev,  volatile uint8_t * mmio);
