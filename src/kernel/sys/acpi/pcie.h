#include <dej/kernel.h>
#pragma once


 void mcfg_enter(void * mcfg);


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
