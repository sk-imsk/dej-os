#include <dej/kernel.h>
#include "device.h"



#define MAX_DRIVERS 128

driver_t * registered_drivers[MAX_DRIVERS];
int driver_count = 0;


void RegisterDriver(driver_t * driver){
	if (driver_count > MAX_DRIVERS){
		registered_drivers[driver_count++] = driver;
	}
}
