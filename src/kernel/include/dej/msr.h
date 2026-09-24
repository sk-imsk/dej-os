#ifdef __x86_64__
#include "x86/msr.h"
#endif
#ifdef __aarch64__
#   include <arm/msr.h>
#endif
