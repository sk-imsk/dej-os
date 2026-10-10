#pragma once
#include <dej/kernel.h>

struct watchdog {
	uint8_t anger;
	bool is_having_hard_time;
	_Atomic bool here;
};


_Noreturn void jump_to_watchdog(void);
void watchdog_handler();
