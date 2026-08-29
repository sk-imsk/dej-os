/*
 * Regression: stbi__load_gif_main_outofmem must clear *delays after free.
 *
 * stbi_load_gif_from_memory hands the caller a heap-allocated int array via
 * its **delays out-parameter. If decoding hits an allocation failure or
 * overflow guard after the first frame succeeded, the cleanup helper
 * stbi__load_gif_main_outofmem must STBI_FREE the array AND null the caller-
 * visible pointer. Otherwise the very common idiom
 *
 *     if (delays) free(delays);
 *
 * (used in tests/fuzz2.c:385 and tests/libfuzz_entry.c:151) double-frees the
 * heap allocation. The convert_format failure and gif_test rejection paths
 * already null *delays; this test pins the OOM helper to the same contract.
 *
 * We force the failure deterministically with a counted allocator and the
 * tiny multi-frame GIF in tests/regression_corpus/gif_delays_uaf_dangling_free.gif.
 * Built under ASan+UBSan (or plain glibc tcache) the regression aborts on
 * the second free; with the fix it passes silently.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_alloc_count = 0;
static int g_fail_after  = -1;

static void *count_malloc(size_t n)
{
    if (n == 0) return NULL;
    ++g_alloc_count;
    if (g_fail_after >= 0 && g_alloc_count > g_fail_after) return NULL;
    return malloc(n);
}

static void *count_realloc(void *p, size_t n)
{
    ++g_alloc_count;
    if (g_fail_after >= 0 && g_alloc_count > g_fail_after) return NULL;
    return realloc(p, n);
}

#define STBI_MALLOC(n)      count_malloc(n)
#define STBI_REALLOC(p,n)   count_realloc((p),(n))
#define STBI_FREE(p)        free(p)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* GIF89a, 1x1 canvas with a 2-entry global palette, then five 1x1 frames.
   Same bytes as tests/regression_corpus/gif_delays_uaf_dangling_free.gif. */
static const unsigned char anim_gif[] = {
    'G','I','F','8','9','a',
    0x01,0x00, 0x01,0x00, 0x80, 0x00, 0x00,
    0xff,0xff,0xff, 0x00,0x00,0x00,
#define FRAME \
    0x21,0xF9,0x04, 0x00, 0x00,0x00, 0x00, 0x00, \
    0x2C, 0x00,0x00, 0x00,0x00, 0x01,0x00, 0x01,0x00, 0x00, \
    0x02, 0x02, 0x44,0x00, 0x00
    FRAME, FRAME, FRAME, FRAME, FRAME,
#undef FRAME
    0x3B
};

static int run_one(int fail_after)
{
    g_alloc_count = 0;
    g_fail_after  = fail_after;

    int *delays = NULL;
    int x = 0, y = 0, z = 0, n = 0;
    unsigned char *p = stbi_load_gif_from_memory(
        anim_gif, (int)sizeof anim_gif, &delays, &x, &y, &z, &n, 0);

    /* Mirror the harness idiom in tests/fuzz2.c and tests/libfuzz_entry.c.
       Pre-fix, when fail_after is large enough that *delays was successfully
       allocated before the failure, this `if (delays) free(delays)` is a
       double-free. Post-fix the helper nulls *delays, so the conditional
       branch is skipped on the failure paths. */
    if (p) stbi_image_free(p);
    if (delays) free(delays);

    return (p != NULL);
}

int main(void)
{
    /* The success path runs ~14 allocations for this 5-frame GIF (1 gif
       struct + 3 first-frame canvas buffers + 2 first-iter big-buffer/delays
       + 2 reallocs * 4 subsequent frames). Sweep the fail threshold across
       and well past that range so every call site that routes through
       stbi__load_gif_main_outofmem with *delays already set is exercised. */
    int passes = 0;
    for (int t = 0; t < 32; ++t) {
        run_one(t);
        ++passes;
    }
    /* Also a clean run with no induced failure, as a sanity check that the
       harness itself isn't lying. */
    g_fail_after = -1;
    int *delays = NULL;
    int x = 0, y = 0, z = 0, n = 0;
    unsigned char *p = stbi_load_gif_from_memory(
        anim_gif, (int)sizeof anim_gif, &delays, &x, &y, &z, &n, 0);
    if (!p) {
        fprintf(stderr, "FAIL: clean run returned NULL (%s)\n",
                stbi_failure_reason() ? stbi_failure_reason() : "(null)");
        if (delays) free(delays);
        return 1;
    }
    if (z != 5) {
        fprintf(stderr, "FAIL: expected 5 frames, got z=%d\n", z);
        stbi_image_free(p);
        if (delays) free(delays);
        return 1;
    }
    stbi_image_free(p);
    free(delays);

    printf("OK regress_gif_delays: %d threshold passes + clean load (z=%d)\n", passes, z);
    return 0;
}
