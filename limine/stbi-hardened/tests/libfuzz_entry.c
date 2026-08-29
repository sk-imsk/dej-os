/*
 * libFuzzer entry point. Feeds each input through:
 *   - 8-bit, 16-bit, float memory loaders (each with req_comp in 0..4)
 *   - animated GIF loader
 *   - zlib decoder
 *   - info / is_16 / is_hdr probes
 *   - a "trickling" callback-based loader to exercise refill paths
 *
 * The harness must not exit abnormally under any input. Any crash or
 * sanitizer error is a real finding.
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const unsigned char *buf;
    int len;
    int pos;
} trickle_state;

static int trickle_read(void *user, char *data, int size)
{
    trickle_state *t = user;
    int n = t->len - t->pos;
    if (n > 13) n = 13;           /* deliberately awkward chunk size */
    if (n > size) n = size;
    if (n > 0) memcpy(data, t->buf + t->pos, (size_t)n);
    t->pos += n;
    return n;
}
static void trickle_skip(void *user, int n)
{
    trickle_state *t = user;
    if (n < 0) n = 0;
    if (t->pos + n > t->len) t->pos = t->len;
    else t->pos += n;
}
static int trickle_eof(void *user)
{
    trickle_state *t = user;
    return t->pos >= t->len;
}

static void touch8(unsigned char *p, int x, int y, int c)
{
    if (!p || x <= 0 || y <= 0 || c <= 0) return;
    volatile unsigned long long acc = 0;
    size_t sz = (size_t)x * (size_t)y * (size_t)c;
    for (size_t i = 0; i < sz; ++i) acc += p[i];
    (void)acc;
}
static void touch16(unsigned short *p, int x, int y, int c)
{
    if (!p || x <= 0 || y <= 0 || c <= 0) return;
    volatile unsigned long long acc = 0;
    size_t sz = (size_t)x * (size_t)y * (size_t)c;
    for (size_t i = 0; i < sz; ++i) acc += p[i];
    (void)acc;
}
static void touchf(float *p, int x, int y, int c)
{
    if (!p || x <= 0 || y <= 0 || c <= 0) return;
    volatile double acc = 0;
    size_t sz = (size_t)x * (size_t)y * (size_t)c;
    for (size_t i = 0; i < sz; ++i) acc += (double)p[i];
    (void)acc;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size > (size_t)INT_MAX) return 0;
    int len = (int)size;

    /* Derive a deterministic req_comp from the first byte so libFuzzer
       can explore the req_comp axis as part of the input. */
    int rc = (len > 0 ? data[0] : 0) & 0x7;
    if (rc > 4) rc -= 4;

    /* --- 8-bit memory ---------------------------------------------- */
    {
        int x=0, y=0, n=0;
        unsigned char *p = stbi_load_from_memory(data, len, &x, &y, &n, rc);
        if (p) {
            touch8(p, x, y, rc ? rc : n);
            stbi_image_free(p);
        }
    }

    /* --- 8-bit memory with NULL out-params ------------------------- */
    {
        unsigned char *p = stbi_load_from_memory(data, len, NULL, NULL, NULL, rc);
        if (p) stbi_image_free(p);
    }

    /* --- 8-bit memory with vertical flip --------------------------- */
    stbi_set_flip_vertically_on_load(1);
    {
        int x=0, y=0, n=0;
        unsigned char *p = stbi_load_from_memory(data, len, &x, &y, &n, rc);
        if (p) {
            touch8(p, x, y, rc ? rc : n);
            stbi_image_free(p);
        }
    }
    stbi_set_flip_vertically_on_load(0);

    /* --- 16-bit memory --------------------------------------------- */
    {
        int x=0, y=0, n=0;
        unsigned short *p = stbi_load_16_from_memory(data, len, &x, &y, &n, rc);
        if (p) {
            touch16(p, x, y, rc ? rc : n);
            stbi_image_free(p);
        }
    }

#ifndef STBI_NO_LINEAR
    /* --- float memory ---------------------------------------------- */
    {
        int x=0, y=0, n=0;
        float *p = stbi_loadf_from_memory(data, len, &x, &y, &n, rc);
        if (p) {
            touchf(p, x, y, rc ? rc : n);
            stbi_image_free(p);
        }
    }
#endif

#ifndef STBI_NO_GIF
    /* --- animated GIF ---------------------------------------------- */
    {
        int *delays = NULL;
        int x=0, y=0, z=0, n=0;
        unsigned char *p = stbi_load_gif_from_memory(data, len, &delays, &x, &y, &z, &n, rc);
        if (p) {
            int comp = rc ? rc : (n ? n : 4);
            if (z > 0 && x > 0 && y > 0) {
                volatile unsigned long long acc = 0;
                size_t sz = (size_t)z * (size_t)x * (size_t)y * (size_t)comp;
                for (size_t i = 0; i < sz; ++i) acc += p[i];
                (void)acc;
            }
            stbi_image_free(p);
        }
        if (delays) free(delays);
    }
#endif

    /* --- info queries ---------------------------------------------- */
    {
        int x=0, y=0, n=0;
        (void)stbi_info_from_memory(data, len, &x, &y, &n);
        (void)stbi_is_16_bit_from_memory(data, len);
#ifndef STBI_NO_HDR
        (void)stbi_is_hdr_from_memory(data, len);
#endif
    }

    /* --- trickling callbacks exercise refill ----------------------- */
    {
        stbi_io_callbacks cb = { trickle_read, trickle_skip, trickle_eof };
        trickle_state t = { data, len, 0 };
        int x=0, y=0, n=0;
        unsigned char *p = stbi_load_from_callbacks(&cb, &t, &x, &y, &n, rc);
        if (p) {
            touch8(p, x, y, rc ? rc : n);
            stbi_image_free(p);
        }
    }

#ifndef STBI_NO_PNG
    /* --- zlib decode ----------------------------------------------- */
    {
        int ol = 0;
        char *p = stbi_zlib_decode_malloc((const char *)data, len, &ol);
        if (p) free(p);
    }
#endif

    return 0;
}
