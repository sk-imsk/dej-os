#include <dej/kernel.h>

typedef enum {
    DEV_TYPE_ROOT,
    DEV_TYPE_BUS,       // PCI, USB, ACPI
    DEV_TYPE_CONTROLLER, // AHCI, NVMe, xHCI
    DEV_TYPE_DISK,       // SSD, CD-ROM
    DEV_TYPE_INPUT       // Keyboard, Mouse
} device_type_t;



typedef struct device {
    char name[32];
    device_type_t type;

    struct device * parent;
    struct device * first_child;
    struct device * next_sibling;

    void * bus_context;

   struct driver * driver;

}device_t;



typedef struct driver {
    const char* name;

    bool (*probe)(device_t* dev);

    // Lifecycle functions
    bool (*attach)(device_t* dev);
    bool (*detach)(device_t* dev);


} driver_t;



void RegisterDriver(driver_t * driver);
