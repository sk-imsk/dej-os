#include <dej/kernel.h>
#include <dej/log.h>
#include "device.h"


driver_t * registered_drivers[MAX_DRIVERS];
int driver_count = 0;


void RegisterDriver(driver_t * driver){
	if (driver_count < MAX_DRIVERS ){
		registered_drivers[driver_count++] = driver;
	} else {
		LogfStr("Driver %s couldent be loaded because all driver spots are full", driver->name);
	}
}
