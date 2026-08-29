/*
 * Expanded fuzzer. Beyond tests/fuzz.c, this exercises:
 *   - stbi_load_16_from_memory, stbi_loadf_from_memory
 *   - stbi_info_from_memory, stbi_is_16_bit_from_memory, stbi_is_hdr_from_memory
 *   - stbi_load_gif_from_memory (animated path)
 *   - stbi_load_from_callbacks with a callback that trickles bytes one at a time
 *     (tests the refill-buffer path that is skipped by memory input)
 *   - seed inputs built from format-valid scaffolds with random mutations,
 *     not just random bytes
 *   - NULL output-parameter passing on the API boundary
 *
 * All calls run under whatever sanitizer the compiler was invoked with.
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>

/* ---- RNG ----------------------------------------------------------------- */

static unsigned int g_rng = 0xDEADBEEFu;
static unsigned int rng(void)
{
    unsigned int x = g_rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_rng = x ? x : 0x1234567u;
    return g_rng;
}
static int rng_int(int lo, int hi)
{
    if (hi <= lo) return lo;
    return lo + (int)(rng() % (unsigned)(hi - lo + 1));
}
static void rng_fill(unsigned char *buf, int n)
{
    int i;
    for (i = 0; i < n; ++i) buf[i] = (unsigned char)(rng() & 0xff);
}

/* ---- Seed generators ----------------------------------------------------- */

static int write_u32be(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v >> 24);
    p[1] = (unsigned char)(v >> 16);
    p[2] = (unsigned char)(v >>  8);
    p[3] = (unsigned char)(v      );
    return 4;
}
static int write_u32le(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v      );
    p[1] = (unsigned char)(v >>  8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
    return 4;
}
static int write_u16le(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v      );
    p[1] = (unsigned char)(v >>  8);
    return 2;
}

/* PNG seed: 2x2 RGBA */
static int seed_png(unsigned char *buf, int cap)
{
    static const unsigned char png[] = {
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
    if (cap < (int)sizeof png) return 0;
    memcpy(buf, png, sizeof png);
    return (int)sizeof png;
}

/* BMP seed: 2x2 24bpp */
static int seed_bmp(unsigned char *buf, int cap)
{
    int n = 0;
    if (cap < 70) return 0;
    buf[n++] = 'B'; buf[n++] = 'M';
    n += write_u32le(buf+n, 70);         /* file size */
    n += write_u16le(buf+n, 0);
    n += write_u16le(buf+n, 0);
    n += write_u32le(buf+n, 54);         /* pixel offset */
    n += write_u32le(buf+n, 40);         /* DIB size */
    n += write_u32le(buf+n, 2);          /* width */
    n += write_u32le(buf+n, 2);          /* height */
    n += write_u16le(buf+n, 1);          /* planes */
    n += write_u16le(buf+n, 24);         /* bpp */
    n += write_u32le(buf+n, 0);          /* compression */
    n += write_u32le(buf+n, 16);         /* image size */
    n += write_u32le(buf+n, 0);
    n += write_u32le(buf+n, 0);
    n += write_u32le(buf+n, 0);
    n += write_u32le(buf+n, 0);
    /* pixel data: 2 rows of 6 bytes + 2 pad bytes each */
    memcpy(buf+n, "\xff\x00\x00\x00\xff\x00\x00\x00", 8); n += 8;
    memcpy(buf+n, "\x00\x00\xff\xff\xff\x00\x00\x00", 8); n += 8;
    return n;
}

/* TGA seed: 2x2 uncompressed 24bpp */
static int seed_tga(unsigned char *buf, int cap)
{
    if (cap < 30) return 0;
    memcpy(buf, "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x02\x00\x18\x00", 18);
    memcpy(buf+18, "\xff\x00\x00\x00\xff\x00\x00\x00\xff\xff\xff\x00", 12);
    return 30;
}

/* GIF87a seed: 1x1 */
static int seed_gif(unsigned char *buf, int cap)
{
    static const unsigned char gif[] = {
        'G','I','F','8','7','a',
        0x01,0x00, 0x01,0x00,     /* width=1, height=1 */
        0xF0, 0x00, 0x00,          /* global color table flag, bgindex, aspect */
        0xFF,0xFF,0xFF, 0x00,0x00,0x00, /* palette: 2 entries */
        0x2C,                      /* image separator */
        0x00,0x00, 0x00,0x00,      /* top-left x, y */
        0x01,0x00, 0x01,0x00,      /* width, height */
        0x00,                      /* local color table flag */
        0x02,                      /* LZW min code size */
        0x02, 0x44, 0x01,          /* data block: 2 bytes */
        0x00,                      /* block terminator */
        0x3B                       /* GIF trailer */
    };
    if (cap < (int)sizeof gif) return 0;
    memcpy(buf, gif, sizeof gif);
    return (int)sizeof gif;
}

/* PSD seed: 1x1 RGB 8bpc */
static int seed_psd(unsigned char *buf, int cap)
{
    int n = 0;
    if (cap < 64) return 0;
    memcpy(buf+n, "8BPS", 4); n += 4;
    n += write_u32be(buf+n, 0x00010000);  /* version 1, then 6 reserved bytes start */
    /* Above wrote only 4 bytes for version/reserved; do it properly. */
    n = 0;
    memcpy(buf+n, "8BPS", 4); n += 4;
    buf[n++] = 0x00; buf[n++] = 0x01;     /* version 1 */
    memset(buf+n, 0, 6); n += 6;          /* reserved */
    buf[n++] = 0x00; buf[n++] = 0x03;     /* channels = 3 */
    n += write_u32be(buf+n, 1);           /* height */
    n += write_u32be(buf+n, 1);           /* width */
    buf[n++] = 0x00; buf[n++] = 0x08;     /* depth = 8 */
    buf[n++] = 0x00; buf[n++] = 0x03;     /* color mode = RGB */
    /* mode data (length prefix 0), resources (0), layer/mask (0) */
    n += write_u32be(buf+n, 0);
    n += write_u32be(buf+n, 0);
    n += write_u32be(buf+n, 0);
    /* compression: 0 = none */
    buf[n++] = 0x00; buf[n++] = 0x00;
    /* 1 byte per channel * 3 channels = 3 bytes of raw RGB */
    buf[n++] = 0xFF; buf[n++] = 0x80; buf[n++] = 0x20;
    return n;
}

static int (* const SEEDS[])(unsigned char *, int) = {
    seed_png, seed_bmp, seed_tga, seed_gif, seed_psd
};
#define NSEEDS ((int)(sizeof SEEDS / sizeof SEEDS[0]))

/* Mutate buf in-place: random byte flips, size pushes, byte insertions. */
static void mutate(unsigned char *buf, int *len_p, int cap)
{
    int len = *len_p;
    int ops = rng_int(1, 16);
    while (ops--) {
        int mode = (int)(rng() % 5);
        if (mode == 0 && len > 0) {
            /* flip a byte */
            int idx = (int)(rng() % (unsigned)len);
            buf[idx] = (unsigned char)(rng() & 0xff);
        } else if (mode == 1 && len > 0) {
            /* zero a range */
            int idx = (int)(rng() % (unsigned)len);
            int span = (int)(rng() % 32u) + 1;
            if (idx + span > len) span = len - idx;
            memset(buf + idx, 0, (size_t)span);
        } else if (mode == 2 && len > 0) {
            /* max-out a range */
            int idx = (int)(rng() % (unsigned)len);
            int span = (int)(rng() % 32u) + 1;
            if (idx + span > len) span = len - idx;
            memset(buf + idx, 0xFF, (size_t)span);
        } else if (mode == 3 && len < cap) {
            /* append a random byte */
            buf[len++] = (unsigned char)(rng() & 0xff);
        } else if (mode == 4 && len > 0) {
            /* truncate */
            len = (int)(rng() % (unsigned)len);
        }
    }
    *len_p = len;
}

/* ---- Callback IO that trickles bytes --------------------------------------- */

typedef struct {
    const unsigned char *buf;
    int len;
    int pos;
} trickle_state;

static int trickle_read(void *user, char *data, int size)
{
    trickle_state *ts = (trickle_state *)user;
    int n = ts->len - ts->pos;
    /* never deliver more than 7 bytes per call to exercise refill */
    if (n > 7) n = 7;
    if (n > size) n = size;
    if (n > 0) memcpy(data, ts->buf + ts->pos, (size_t)n);
    ts->pos += n;
    return n;
}
static void trickle_skip(void *user, int n)
{
    trickle_state *ts = (trickle_state *)user;
    if (n < 0) n = 0;
    if (ts->pos + n > ts->len) ts->pos = ts->len;
    else ts->pos += n;
}
static int trickle_eof(void *user)
{
    trickle_state *ts = (trickle_state *)user;
    return ts->pos >= ts->len;
}

/* ---- Iteration body ------------------------------------------------------- */

static void touch_output_8(unsigned char *p, int x, int y, int comp)
{
    if (!p || x <= 0 || y <= 0 || comp <= 0) return;
    {
        volatile unsigned long long acc = 0;
        size_t sz = (size_t)x * (size_t)y * (size_t)comp;
        size_t k;
        for (k = 0; k < sz; ++k) acc += p[k];
        (void)acc;
    }
}
static void touch_output_16(unsigned short *p, int x, int y, int comp)
{
    if (!p || x <= 0 || y <= 0 || comp <= 0) return;
    {
        volatile unsigned long long acc = 0;
        size_t sz = (size_t)x * (size_t)y * (size_t)comp;
        size_t k;
        for (k = 0; k < sz; ++k) acc += p[k];
        (void)acc;
    }
}
static void touch_output_f(float *p, int x, int y, int comp)
{
    if (!p || x <= 0 || y <= 0 || comp <= 0) return;
    {
        volatile double acc = 0;
        size_t sz = (size_t)x * (size_t)y * (size_t)comp;
        size_t k;
        for (k = 0; k < sz; ++k) acc += (double)p[k];
        (void)acc;
    }
}

static void one_pass(const unsigned char *buf, int len)
{
    int rc_want = (int)(rng() % 5);   /* 0..4 */
    int x, y, n;

    /* 8-bit memory load */
    {
        x=0; y=0; n=0;
        unsigned char *p = stbi_load_from_memory(buf, len, &x, &y, &n, rc_want);
        if (p) {
            touch_output_8(p, x, y, rc_want ? rc_want : n);
            stbi_image_free(p);
        }
    }

    /* 8-bit NULL outputs (pointer safety) */
    {
        unsigned char *p = stbi_load_from_memory(buf, len, NULL, NULL, NULL, rc_want);
        if (p) stbi_image_free(p);
    }

    /* 16-bit memory load */
    {
        x=0; y=0; n=0;
        unsigned short *p = stbi_load_16_from_memory(buf, len, &x, &y, &n, rc_want);
        if (p) {
            touch_output_16(p, x, y, rc_want ? rc_want : n);
            stbi_image_free(p);
        }
    }

#ifndef STBI_NO_LINEAR
    /* float memory load */
    {
        x=0; y=0; n=0;
        float *p = stbi_loadf_from_memory(buf, len, &x, &y, &n, rc_want);
        if (p) {
            touch_output_f(p, x, y, rc_want ? rc_want : n);
            stbi_image_free(p);
        }
    }
#endif

    /* info queries */
    {
        x=0; y=0; n=0;
        (void)stbi_info_from_memory(buf, len, &x, &y, &n);
        (void)stbi_info_from_memory(buf, len, NULL, NULL, NULL);
        (void)stbi_is_16_bit_from_memory(buf, len);
#ifndef STBI_NO_HDR
        (void)stbi_is_hdr_from_memory(buf, len);
#endif
    }

    /* callbacks path with trickled reads */
    {
        stbi_io_callbacks cb;
        trickle_state ts;
        cb.read = trickle_read;
        cb.skip = trickle_skip;
        cb.eof  = trickle_eof;
        ts.buf = buf; ts.len = len; ts.pos = 0;

        x=0; y=0; n=0;
        {
            unsigned char *p = stbi_load_from_callbacks(&cb, &ts, &x, &y, &n, rc_want);
            if (p) {
                touch_output_8(p, x, y, rc_want ? rc_want : n);
                stbi_image_free(p);
            }
        }

        ts.pos = 0;
        (void)stbi_info_from_callbacks(&cb, &ts, NULL, NULL, NULL);
        ts.pos = 0;
        (void)stbi_is_16_bit_from_callbacks(&cb, &ts);
#ifndef STBI_NO_HDR
        ts.pos = 0;
        (void)stbi_is_hdr_from_callbacks(&cb, &ts);
#endif
    }

#ifndef STBI_NO_GIF
    /* animated GIF path */
    {
        int *delays = NULL;
        int z = 0;
        x=0; y=0; n=0;
        unsigned char *p = stbi_load_gif_from_memory(buf, len, &delays, &x, &y, &z, &n, rc_want);
        if (p) {
            int comp_out = rc_want ? rc_want : (n ? n : 4);
            if (z > 0 && x > 0 && y > 0) {
                volatile unsigned long long acc = 0;
                size_t sz = (size_t)z * (size_t)x * (size_t)y * (size_t)comp_out;
                size_t k;
                for (k = 0; k < sz; ++k) acc += p[k];
                (void)acc;
            }
            stbi_image_free(p);
        }
        if (delays) free(delays);
    }
#endif

#ifndef STBI_NO_PNG
    /* zlib decode path */
    {
        int ol = 0;
        char *p = stbi_zlib_decode_malloc((const char *)buf, len, &ol);
        if (p) free(p);
    }
#endif
}

int main(int argc, char **argv)
{
    int iterations = 5000;
    int max_len = 4096;
    int i;

    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) max_len    = atoi(argv[2]);
    if (iterations <= 0) iterations = 5000;
    if (max_len    < 32) max_len    = 32;

    g_rng = (unsigned int)time(NULL) ^ 0xDEADBEEFu;
    if (!g_rng) g_rng = 1;

    for (i = 0; i < iterations; ++i) {
        int len = 0;
        unsigned char *buf = (unsigned char *)malloc((size_t)max_len);
        if (!buf) { fprintf(stderr, "oom\n"); return 1; }

        /* 3 in 4 iterations: start from a seed then mutate */
        if ((rng() & 3) != 0) {
            int kind = (int)(rng() % (unsigned)NSEEDS);
            len = SEEDS[kind](buf, max_len);
            mutate(buf, &len, max_len);
        } else {
            /* 1 in 4: pure random */
            len = rng_int(0, max_len);
            rng_fill(buf, len);
        }

        one_pass(buf, len);

        free(buf);
    }

    printf("fuzz2: %d iterations OK\n", iterations);
    return 0;
}
