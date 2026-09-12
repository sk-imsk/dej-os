#include <stdint.h>
inline int rdrand(uint64_t *value){
    (void)value;
    return false;
}
