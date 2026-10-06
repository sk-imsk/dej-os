#include <dej/kernel.h>
#pragma once


 void mcfg_enter(void * mcfg);
 volatile uint8_t *pci_config( uint8_t bus, uint8_t device, uint8_t function);


 struct pci_device {
     uint8_t bus;
     uint8_t device;
     uint8_t function;

     uint16_t vendor_id;
     uint16_t device_id;

     uint8_t class;
     uint8_t subclass;
     uint8_t prog_if;
     uint8_t header_type;

     uint64_t bar[6];
 };

 struct pci_driver {
     uint16_t vendor_id;     // 0xffff = any
     uint16_t device_id;     // 0xffff = any

     uint8_t class;
     uint8_t subclass;
     uint8_t prog_if;

     char name[10];

     void (*probe)(struct pci_device *dev);
 };


 #define mmio_read32(addr)       (*(volatile uint32_t *)(addr))
 #define mmio_write32(addr, val) (*(volatile uint32_t *)(addr) = (val))
 #define mmio_read16(addr)       (*(volatile uint16_t *)(addr))
 #define mmio_write16(addr, val) (*(volatile uint16_t *)(addr) = (val))
 #define mmio_read8(addr)       (*(volatile uint8_t *)(addr))
 #define mmio_write8(addr, val) (*(volatile uint8_t *)(addr) = (val))
