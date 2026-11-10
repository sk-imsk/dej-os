#include "panic.h"
#include <dej/cpu.h>

_Noreturn void HaRecover(uint8_t status){
	(void)status;
	while (1) cpu_stop();
}
