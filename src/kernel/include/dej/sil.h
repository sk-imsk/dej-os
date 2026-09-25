// sil.h
#include <dej/kernel.h>
#include <dej/percpu.h>
#include <dej/panic.h>



typedef uint8_t ksil;

ksil RaiseSil(ksil new);
void LowerSil(ksil new);



#define CHILL_LEVEL 0
#define APC_LEVEL 1
#define DISPATCH_LEVEL 2
#define DEVICE_LEVEL 4                  // between 3 and 15, 4 is just like default
#define NOTCHILL_LEVEL 15           // max i would go higher if le cpu let me




#define Assert_sil_less_than_or_equal(x)         \
    if (x <= percpu_read(sil)) panic("sil not less or equal");

#define Assert_sil_more_than(x) \
    if (x > percpu_read(sil)) panic("sil too small ffor operation");

#define Assert_sil_chill()         \
    if (0 != percpu_read(sil)) panic("sil not less or equal");
