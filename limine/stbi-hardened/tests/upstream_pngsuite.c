/*
 * Upstream regression test: read pngsuite/ref_results.csv (maintained by
 * nothings/stb via test_png_regress.c) and verify that our hardened
 * decoder still produces the same width/height/ncomp/error/hash for every
 * file in pngsuite/.
 *
 * The reference data is the golden output of the *upstream* decoder; if
 * our hardening has introduced a behavior change for any input in the
 * corpus (primary/corrupt/iphone/16bit/unused), it shows up here.
 *
 * Uses the same FNV-1a-32 hash as upstream's test_png_regress.c so we're
 * bit-exact comparable.
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int fnv1a_hash32(const unsigned char *bytes, size_t len)
{
    unsigned int hash = 0x811c9dc5;
    unsigned int mul  = 0x01000193;
    size_t i;
    for (i = 0; i < len; ++i)
        hash = (hash ^ bytes[i]) * mul;
    return hash;
}

/* Strip trailing CR/LF/whitespace. */
static void rstrip(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n-1]=='\n' || s[n-1]=='\r' || s[n-1]=='\t' || s[n-1]==' '))
        s[--n] = 0;
}

static int passed = 0, mismatched = 0, missing = 0;

static void check(const char *base_dir, const char *rel_path,
                  int ref_w, int ref_h, int ref_n,
                  const char *ref_error, unsigned int ref_hash)
{
    char path[1024];
    snprintf(path, sizeof path, "%s/%s", base_dir, rel_path);

    int w = 0, h = 0, n = 0;
    unsigned char *pixels = stbi_load(path, &w, &h, &n, 0);
    const char *err = "";
    unsigned int hash = 0;

    if (!pixels) {
        err = stbi_failure_reason() ? stbi_failure_reason() : "";
    } else {
        hash = fnv1a_hash32(pixels, (size_t)w * (size_t)h * (size_t)n);
        stbi_image_free(pixels);
    }

    int had_ref_error = ref_error && ref_error[0] != 0;
    int had_our_error = !pixels;

    /* Compare. A loose policy on error strings: as long as both either
     * succeeded or both failed, we consider the row a match. If the
     * upstream golden says "success" we also verify the w/h/n/hash. */
    if (had_ref_error && had_our_error) {
        ++passed;
        return;
    }
    if (!had_ref_error && !had_our_error) {
        if (w == ref_w && h == ref_h && n == ref_n && hash == ref_hash) {
            ++passed;
            return;
        }
        ++mismatched;
        fprintf(stderr, "MISMATCH %s: ref %dx%d n=%d hash=0x%08x  got %dx%d n=%d hash=0x%08x (err=%s)\n",
                rel_path, ref_w, ref_h, ref_n, ref_hash,
                w, h, n, hash, err);
        return;
    }
    ++mismatched;
    fprintf(stderr, "DIVERGENCE %s: ref_error=\"%s\" our_error=\"%s\"\n",
            rel_path, ref_error, err);
}

int main(int argc, char **argv)
{
    const char *base_dir = argc > 1 ? argv[1] : "tests";
    char csv_path[1024];
    snprintf(csv_path, sizeof csv_path, "%s/pngsuite/ref_results.csv", base_dir);

    FILE *f = fopen(csv_path, "rb");
    if (!f) {
        fprintf(stderr, "cannot open %s\n", csv_path);
        return 2;
    }

    char line[4096];
    /* Skip the header row. */
    if (!fgets(line, sizeof line, f)) {
        fprintf(stderr, "empty csv\n");
        fclose(f);
        return 2;
    }

    while (fgets(line, sizeof line, f)) {
        rstrip(line);
        if (line[0] == 0) continue;

        /* CSV shape: filename,width,height,ncomp,error,hash
         * Error is possibly empty. filename doesn't contain commas in
         * this corpus. Hash is 0xXXXXXXXX. */
        char *cols[6] = {0};
        int c = 0;
        char *p = line;
        cols[c++] = p;
        while (*p && c < 6) {
            if (*p == ',') { *p = 0; cols[c++] = p + 1; }
            ++p;
        }
        if (c != 6) {
            fprintf(stderr, "bad csv row: %s\n", line);
            ++missing;
            continue;
        }

        int w = atoi(cols[1]);
        int h = atoi(cols[2]);
        int n = atoi(cols[3]);
        unsigned int hash = (unsigned int) strtoul(cols[5], NULL, 0);

        check(base_dir, cols[0], w, h, n, cols[4], hash);
    }
    fclose(f);

    printf("passed=%d mismatched=%d missing=%d\n", passed, mismatched, missing);
    return (mismatched == 0 && missing == 0) ? 0 : 1;
}
