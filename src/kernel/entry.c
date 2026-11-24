#include <dej/kernel.h>
#include <limine.h>
#include <dej/cpu.h>
#include <hal/recover.h>
#include <hal/panic.h>
#include <dej/log.h>

static const char mod[] = "boot";

#define limine_request __attribute__((used, section(".limine_requests")))

limine_request
volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

limine_request
volatile struct limine_mp_request mp_request = {
    .id = LIMINE_MP_REQUEST_ID,
    .revision = 1
};

limine_request
volatile struct limine_framebuffer_request fb_rq = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;


static atomic_flag kernel_entered = {};

void check_limine_good(void){
	if (!fb_rq.response){
		HaRecover(LIMINE_RQ_FAILED);
	}
	if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision)){
		HaRecover(LIMINE_RQ_FAILED);
	}
}


void kentry(void) {
	if (atomic_flag_test_and_set_explicit(&kernel_entered, memory_order_seq_cst)){
		HaRecover(ENTRY_TWICE);
	}

	check_limine_good();


	early_strlog(mod, "Kernel entered");
	early_strlog(mod, "I will not fail");
	early_fstrlog( mod, "Hello It is %s  at %i %i %u also %x", "dih",  1, 2, 3, 0xabcd);

    	cpu_stop(); // yo dont forget
}
