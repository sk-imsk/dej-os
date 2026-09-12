#ifdef __x86_64__
    #include <arch/x86/interrupt/interrupt.h>
#endif

#ifdef __arm__
    #error 32 bit not supported

#endif

#ifdef __aarch64__
    #include <arch/arm/interrupt/interrupt.h>
#endif
