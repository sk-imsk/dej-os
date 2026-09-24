// panic.h

#ifdef __x86_64__
#   include <x86/panic.h>
#endif
#ifdef __aarch64__
#   include <arm/panic.h>
#endif
