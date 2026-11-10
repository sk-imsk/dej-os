#include <stdint.h>


/*
 * status codes for recovery (so we can know what to do )
 */
#define LIMINE_RQ_FAILED 1
#define ENTRY_TWICE 2
_Noreturn void HaRecover(uint8_t status);
