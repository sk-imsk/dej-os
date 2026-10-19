#include <dej/kernel.h>
#include <dej/log.h>
#include "device.h"
#include "../disk/ahci.h"
#include "../hda/hda.h"


driver_t * registered_drivers[MAX_DRIVERS];
int driver_count = 0;


void RegisterDriver(driver_t * driver){
	if (driver_count < MAX_DRIVERS ){
		registered_drivers[driver_count++] = driver;
	} else {
		LogfStr("Driver %s couldent be loaded because all driver spots are full", driver->name);
	}
}


void load_drivers(void){				// loads all my bs drivers ig i will make a better way later
	achi_init();
	hda_init();
}
