// yo x86 panic.h
#include <dej/kernel.h>

typedef uint16_t status;
#define STATUS_UNKNOWN 0
#define STATUS_INVALID_SIL 1    /* sil not right level for operation */
#define STATUS_TAKEN 2          /* Resource already taken */
#define STATUS_HARDWARE_FAILURE 3 /* hardware failed */
#define STATUS_LOCKUP   4               /* processor lockup */
#define STATUS_NOMEM    5       /* System out of memory */
#define STATUS_INVALID 6     /* Invalid arguments to critical function */

_Noreturn void panic(const char *s,  status code);
_Noreturn void fi_panic(const char * cooked);
