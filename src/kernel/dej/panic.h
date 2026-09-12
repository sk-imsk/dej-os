// panic.h

#ifdef __x86_64__
#include <arch/x86/panic.h>
#endif
#ifdef __aarch64__
#   include <arch/arm/panic.h>
#endif
