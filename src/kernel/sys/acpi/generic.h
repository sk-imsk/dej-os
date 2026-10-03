#pragma  once
#include <dej/kernel.h>
#include <dej/limine.h>


typedef struct SDT_header {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
} __attribute__ ((packed)) RSDT_t ;

inline void * phys2virt32(uint32_t addr){
    return (void *)(addr + hhdm_request.response->offset);
}
inline void * phys2virt64(uint64_t addr){
    return (void *)(addr + hhdm_request.response->offset);
}

inline bool check_genericsum(struct SDT_header * r){
    uint8_t * bytes = (uint8_t *)r;
    uint8_t acc = 0;

    for (uint8_t i = 0; i < r->Length; i++ ){
        acc += bytes[i];
    }

    return acc == 0;
}
