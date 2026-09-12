#include <stdint.h>
static inline int rdrand(uint64_t *value)
{
    unsigned char ok;

    __asm__ volatile (
        "rdrand %0"
        : "=r"(*value), "=@ccc"(ok)
    );

    return ok;
}
