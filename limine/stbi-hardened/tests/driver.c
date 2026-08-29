/*
 * Minimal driver that instantiates stb_image once so we can compile-test
 * the hardened header. Also includes a handful of synthetic inputs to
 * exercise the main error paths.
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void expect_null_load(const char *name, const unsigned char *buf, int len)
{
    int x = -1, y = -1, n = -1;
    unsigned char *p = stbi_load_from_memory(buf, len, &x, &y, &n, 0);
    if (p) {
        fprintf(stderr, "%s: expected NULL but got %p (x=%d y=%d n=%d)\n", name, (void *)p, x, y, n);
        free(p);
        exit(1);
    }
    printf("%s: correctly rejected (%s)\n", name, stbi_failure_reason() ? stbi_failure_reason() : "");
}

static void check_null_len(void)
{
    int x, y, n;
    unsigned char *p = stbi_load_from_memory(NULL, 0, &x, &y, &n, 0);
    if (p) { fprintf(stderr, "NULL buffer should fail\n"); exit(1); }

    p = stbi_load_from_memory((const unsigned char*)"", -1, &x, &y, &n, 0);
    if (p) { fprintf(stderr, "negative len should fail\n"); exit(1); }

    printf("null_and_negative_len: ok\n");
}

static void check_zlib_bad_args(void)
{
#ifndef STBI_NO_PNG
    int outlen;
    char *p;

    p = stbi_zlib_decode_malloc_guesssize(NULL, 100, 1024, &outlen);
    if (p) { fprintf(stderr, "zlib NULL buffer should fail\n"); exit(1); }

    p = stbi_zlib_decode_malloc_guesssize("abc", -1, 1024, &outlen);
    if (p) { fprintf(stderr, "zlib negative len should fail\n"); exit(1); }

    p = stbi_zlib_decode_malloc_guesssize("abc", 3, 0, &outlen);
    if (p) { fprintf(stderr, "zlib zero initial should fail\n"); exit(1); }

    p = stbi_zlib_decode_malloc_guesssize("abc", 3, -5, &outlen);
    if (p) { fprintf(stderr, "zlib negative initial should fail\n"); exit(1); }

    printf("zlib_bad_args: ok\n");
#else
    printf("zlib_bad_args: skipped (no PNG)\n");
#endif
}

static void check_valid_png(void)
{
    /* Known-valid 2x2 RGBA PNG. */
    static const unsigned char rgba2x2_png[] = {
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
    int x=0, y=0, n=0;
    unsigned char *p = stbi_load_from_memory(rgba2x2_png, sizeof rgba2x2_png, &x, &y, &n, 0);
    if (!p) {
        fprintf(stderr, "valid 2x2 png failed to decode: %s\n", stbi_failure_reason());
        exit(1);
    }
    if (x != 2 || y != 2) {
        fprintf(stderr, "valid 2x2 png has wrong dims %dx%d\n", x, y);
        free(p);
        exit(1);
    }
    printf("valid_png_2x2: ok (x=%d, y=%d, n=%d)\n", x, y, n);
    free(p);
}

int main(void)
{
    /* empty buffer */
    expect_null_load("empty", (const unsigned char *)"", 0);

    /* one byte */
    {
        unsigned char b = 0;
        expect_null_load("one_byte", &b, 1);
    }

    /* PNG magic followed by garbage (should fail past signature) */
    {
        static const unsigned char bad_png[] = {
            0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A,
            0xff,0xff,0xff,0xff,'I','H','D','R',
            0,0,0,0,0,0,0,0,0,0,0,0,0
        };
        expect_null_load("bad_png_ihdr", bad_png, sizeof bad_png);
    }

    /* BMP that claims a huge width to exercise the dimension guard */
    {
        static const unsigned char huge_bmp[] = {
            'B','M',
            0,0,0,0, 0,0, 0,0, 54,0,0,0,            /* header */
            40,0,0,0,                               /* DIB size */
            0xff,0xff,0xff,0x7f,                    /* width = INT_MAX */
            0xff,0xff,0xff,0x7f,                    /* height = INT_MAX */
            1,0, 24,0, 0,0,0,0,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
        };
        expect_null_load("huge_bmp", huge_bmp, sizeof huge_bmp);
    }

    /* GIF with 0-byte dimensions */
    {
        static const unsigned char empty_gif[] = {
            'G','I','F','8','9','a',
            0,0, 0,0,       /* width=0, height=0 */
            0, 0, 0
        };
        expect_null_load("zero_gif", empty_gif, sizeof empty_gif);
    }

    check_null_len();
    check_zlib_bad_args();
#ifndef STBI_NO_PNG
    check_valid_png();
#endif

    printf("all driver checks passed\n");
    return 0;
}
