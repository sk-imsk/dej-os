#include <stdint.h>

static inline int rdrand(uint64_t *value){
    int  ok;

    __asm__ volatile (
            "mrs %0, s3_3_c2_c4_0\n\t" // s3_3_c2_c4_0 is the system register encoding for RNDR
            "cset %w1, ne\n\t"        // Check if the read was successful
            : "=r"(*value), "=r"(ok)
            :
            : "cc"
        );

    return ok;

}
