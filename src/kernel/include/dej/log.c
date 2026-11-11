#include <dej/kernel.h>
#include <dej/lock.h>
#include "../hal/early.h"

/*
static lock_t logs_lock = {
	.held = false,
	.holding_cpu = -1
};
*/

static void logs(const char *c){
	if (!c) return;

	while (*c) {
		e_logc(*c++);
	}
}

void strlog(const char *restrict mod, const char *restrict msg);
void fstrlog(const char *restrict mod, const char *restrict msg, ...);
void early_strlog(const char *restrict mod, const char *restrict msg){

	e_logc('[');
	logs(mod);
	logs("] ");

	logs(msg);
	e_logc('\n');

}
void early_fstrlog(const char *restrict mod, const char *restrict msg, ...);
