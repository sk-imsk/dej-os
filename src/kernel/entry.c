#include <stdint.h>
#include <limine.h>
#include <dej/cpu.h>
#define limine_request __attribute__((used, section(".limine_requests")))

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;



void kentry(void) {

    cpu_stop(); // yo dont forget
}
