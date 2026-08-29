/*
 * Hand-crafted adversarial inputs: each test case targets a specific class
 * of vulnerability that the hardening is meant to catch. Each case must
 * either reject the input (returning NULL) or decode it successfully without
 * any sanitizer or out-of-bounds memory access.
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

static void must_reject(const char *name, const unsigned char *buf, int len)
{
    int x=-1, y=-1, n=-1;
    unsigned char *p = stbi_load_from_memory(buf, len, &x, &y, &n, 0);
    if (p) {
        fprintf(stderr, "FAIL %s: expected rejection, got %p (x=%d y=%d n=%d)\n",
                name, (void*)p, x, y, n);
        free(p);
        failures++;
        return;
    }
    printf("OK %-40s rejected (%s)\n", name, stbi_failure_reason() ? stbi_failure_reason() : "(null)");
}

static void must_not_crash(const char *name, const unsigned char *buf, int len)
{
    int x=0, y=0, n=0;
    unsigned char *p = stbi_load_from_memory(buf, len, &x, &y, &n, 0);
    /* Accept either NULL or a valid decode; we just must not crash. */
    printf("OK %-40s did-not-crash (p=%p, failure=\"%s\")\n",
           name, (void*)p, stbi_failure_reason() ? stbi_failure_reason() : "");
    if (p) free(p);
}

/* ===== PNG cases ========================================================== */

static void t_png_chunk_too_large(void)
{
    /* PNG signature + a chunk claiming length 0x80000001 (> INT_MAX) */
    static const unsigned char b[] = {
        0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
        0x80,0x00,0x00,0x01,                        /* length */
        'I','H','D','R',                            /* type */
        0,0,0,0, 0,0,0,0, 0,0,0,0,0                  /* body */
    };
    must_reject("png_chunk_too_large", b, (int)sizeof b);
}

static void t_png_zero_dims(void)
{
    /* Valid sig + IHDR with 0x0 dims */
    static const unsigned char b[] = {
        0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
        0x00,0x00,0x00,0x0D,
        'I','H','D','R',
        0x00,0x00,0x00,0x00,  /* width=0 */
        0x00,0x00,0x00,0x00,  /* height=0 */
        0x08, 0x06, 0x00, 0x00, 0x00,
        0x00,0x00,0x00,0x00   /* fake CRC */
    };
    must_reject("png_zero_dims", b, (int)sizeof b);
}

static void t_png_huge_dims(void)
{
    /* Valid sig + IHDR with 2^24 x 2^24 */
    static const unsigned char b[] = {
        0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
        0x00,0x00,0x00,0x0D,
        'I','H','D','R',
        0x02,0x00,0x00,0x00,   /* width = 2^25 (too large) */
        0x02,0x00,0x00,0x00,   /* height = 2^25 */
        0x08, 0x02, 0x00, 0x00, 0x00,
        0x00,0x00,0x00,0x00
    };
    must_reject("png_huge_dims", b, (int)sizeof b);
}

static void t_png_bad_filter(void)
{
    /* Valid-looking 2x2 with an IDAT that decompresses to a byte with
       filter byte >= 5; the decoder must reject. */
    static const unsigned char b[] = {
        0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
        0x00,0x00,0x00,0x0D,'I','H','D','R',
        0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,
        0x08,0x06,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        /* IDAT with zlib of raw bytes starting with filter=9 */
        0x00,0x00,0x00,0x10,'I','D','A','T',
        0x78,0x9C,0x63,0x09,0x00,0x00,0x00,0x00,  /* simple deflate block */
        0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,'I','E','N','D',0,0,0,0
    };
    must_not_crash("png_bad_filter", b, (int)sizeof b);
}

/* ===== BMP cases ========================================================== */

static void t_bmp_negative_height(void)
{
    /* BMP with negative height (top-down, valid) */
    static const unsigned char b[] = {
        'B','M',
        0x46,0x00,0x00,0x00, 0,0, 0,0, 0x36,0x00,0x00,0x00,
        0x28,0x00,0x00,0x00,
        0x02,0x00,0x00,0x00,                  /* width=2 */
        0xFE,0xFF,0xFF,0xFF,                  /* height=-2 (top-down) */
        0x01,0x00, 0x18,0x00, 0x00,0x00,0x00,0x00,
        0x10,0x00,0x00,0x00, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 16 bytes pixel data */
        0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,
        0x00,0x00,0xFF,0xFF,0xFF,0x00,0x00,0x00
    };
    must_not_crash("bmp_negative_height", b, (int)sizeof b);
}

static void t_bmp_huge_offset(void)
{
    static const unsigned char b[] = {
        'B','M',
        0x46,0x00,0x00,0x00, 0,0, 0,0,
        0xFF,0xFF,0xFF,0x7F,              /* huge pixel offset */
        0x28,0x00,0x00,0x00,
        0x02,0x00,0x00,0x00, 0x02,0x00,0x00,0x00,
        0x01,0x00, 0x18,0x00, 0,0,0,0, 0x10,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    };
    must_reject("bmp_huge_offset", b, (int)sizeof b);
}

/* ===== GIF cases ========================================================== */

static void t_gif_nested_bad(void)
{
    /* GIF header with huge width claim */
    static const unsigned char b[] = {
        'G','I','F','8','9','a',
        0xFF,0xFF, 0xFF,0xFF,
        0x00, 0x00, 0x00
    };
    must_reject("gif_huge_dims", b, (int)sizeof b);
}

static void t_gif_image_descriptor_overflow(void)
{
    /* Valid header 4x4 but ID sub-rect with x=3, w=10 (3+10 > 4) */
    static const unsigned char b[] = {
        'G','I','F','8','9','a',
        0x04,0x00, 0x04,0x00,
        0xF0, 0x00, 0x00,
        /* global palette (2 entries) */
        0xFF,0xFF,0xFF, 0x00,0x00,0x00,
        /* image descriptor */
        0x2C,
        0x03,0x00, 0x00,0x00,            /* x=3, y=0 */
        0x0A,0x00, 0x01,0x00,            /* w=10, h=1: overflows */
        0x00,
        0x02, 0x02, 0x44, 0x01, 0x00,
        0x3B
    };
    must_reject("gif_image_descriptor_overflow", b, (int)sizeof b);
}

/* ===== PSD cases ========================================================== */

static void t_psd_zero_dims(void)
{
    static const unsigned char b[] = {
        '8','B','P','S', 0x00,0x01, 0,0,0,0,0,0,
        0x00,0x03,                  /* channels */
        0x00,0x00,0x00,0x00,        /* height=0 */
        0x00,0x00,0x00,0x00,        /* width=0 */
        0x00,0x08,                  /* depth */
        0x00,0x03,                  /* mode=RGB */
        0,0,0,0, 0,0,0,0, 0,0,0,0,
        0x00,0x00
    };
    must_reject("psd_zero_dims", b, (int)sizeof b);
}

static void t_psd_huge_channels(void)
{
    static const unsigned char b[] = {
        '8','B','P','S', 0x00,0x01, 0,0,0,0,0,0,
        0xFF,0xFF,                  /* channels=65535 */
        0x00,0x00,0x00,0x01,        /* height=1 */
        0x00,0x00,0x00,0x01,        /* width=1 */
        0x00,0x08, 0x00,0x03,
        0,0,0,0, 0,0,0,0, 0,0,0,0,
        0x00,0x00
    };
    must_reject("psd_huge_channels", b, (int)sizeof b);
}

/* ===== Zlib cases ========================================================= */

static void t_zlib_null_and_bad_params(void)
{
    char *p;
    int outlen = 0;
    p = stbi_zlib_decode_malloc_guesssize(NULL, 10, 1024, &outlen);
    if (p) { fprintf(stderr, "FAIL zlib NULL buffer\n"); failures++; free(p); }
    p = stbi_zlib_decode_malloc_guesssize("abc", -1, 1024, &outlen);
    if (p) { fprintf(stderr, "FAIL zlib neg len\n"); failures++; free(p); }
    p = stbi_zlib_decode_malloc_guesssize("abc", 3, 0, &outlen);
    if (p) { fprintf(stderr, "FAIL zlib zero init\n"); failures++; free(p); }
    p = stbi_zlib_decode_malloc_guesssize("abc", 3, (1<<30)+1, &outlen);
    if (p) { fprintf(stderr, "FAIL zlib huge init\n"); failures++; free(p); }
    printf("OK %-40s rejected\n", "zlib_bad_params");
}

/* ===== API NULL safety ==================================================== */

static void t_api_null_outputs(void)
{
    /* Load a valid PNG but pass NULL for all output pointers. */
    static const unsigned char rgba[] = {
        0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
        0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
        0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,
        0x08,0x06,0x00,0x00,0x00,0x72,0xB6,0x0D,
        0x24,0x00,0x00,0x00,0x14,0x49,0x44,0x41,
        0x54,0x78,0x9C,0x63,0xF8,0xCF,0xC0,0xF0,
        0x1F,0x0C,0x81,0x34,0x10,0x30,0xFC,0x07,
        0x00,0x47,0xCA,0x08,0xF8,0x8B,0x4E,0x43,
        0x85,0x00,0x00,0x00,0x00,0x49,0x45,0x4E,
        0x44,0xAE,0x42,0x60,0x82
    };
    unsigned char *p = stbi_load_from_memory(rgba, sizeof rgba, NULL, NULL, NULL, 0);
    if (!p) {
        fprintf(stderr, "FAIL null-outputs couldn't decode valid PNG: %s\n", stbi_failure_reason());
        failures++;
        return;
    }
    free(p);

    /* info with NULL outputs */
    if (!stbi_info_from_memory(rgba, sizeof rgba, NULL, NULL, NULL)) {
        fprintf(stderr, "FAIL info with NULL outputs\n");
        failures++;
        return;
    }
    printf("OK %-40s null-outputs-safe\n", "api_null_outputs");
}

static void t_api_null_filename(void)
{
    int x, y, n;
    if (stbi_load(NULL, &x, &y, &n, 0)) { fprintf(stderr, "FAIL null filename load\n"); failures++; }
    if (stbi_info(NULL, &x, &y, &n))    { fprintf(stderr, "FAIL null filename info\n"); failures++; }
    if (stbi_is_hdr(NULL))              { fprintf(stderr, "FAIL null filename is_hdr\n"); failures++; }
    if (stbi_is_16_bit(NULL))           { fprintf(stderr, "FAIL null filename is_16\n"); failures++; }
    printf("OK %-40s null-filename-safe\n", "api_null_filename");
}

static void t_api_null_callbacks(void)
{
    int x, y, n;
    if (stbi_load_from_callbacks(NULL, NULL, &x, &y, &n, 0)) {
        fprintf(stderr, "FAIL null cb load\n"); failures++;
    }
    {
        stbi_io_callbacks cb = { NULL, NULL, NULL };
        if (stbi_load_from_callbacks(&cb, NULL, &x, &y, &n, 0)) {
            fprintf(stderr, "FAIL zero cb load\n"); failures++;
        }
    }
    printf("OK %-40s null-callbacks-safe\n", "api_null_callbacks");
}

/* ===== main ================================================================ */

int main(void)
{
    t_png_chunk_too_large();
    t_png_zero_dims();
    t_png_huge_dims();
    t_png_bad_filter();
    t_bmp_negative_height();
    t_bmp_huge_offset();
    t_gif_nested_bad();
    t_gif_image_descriptor_overflow();
    t_psd_zero_dims();
    t_psd_huge_channels();
    t_zlib_null_and_bad_params();
    t_api_null_outputs();
    t_api_null_filename();
    t_api_null_callbacks();

    printf("\nfailures=%d\n", failures);
    return failures ? 1 : 0;
}
