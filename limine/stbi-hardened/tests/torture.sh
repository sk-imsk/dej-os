#!/usr/bin/env bash
# Torture test. Runs every sanitizer / compiler / optimisation combo we have,
# plus coverage-guided libFuzzer under ASan+UBSan and MSan, against both our
# own fuzz harness and upstream's. Also runs the upstream PngSuite golden-hash
# suite and sweeps every file in tests/regression_corpus/ through the decoder
# so no historical bug ever silently reappears.

set -u
cd "$(dirname "$0")/.."
BUILD=/tmp/stbi_torture
mkdir -p "$BUILD"
LOG="$BUILD/log"
: > "$LOG"

# Persistent corpora across runs: each libFuzzer run writes back into these
# directories, so every torture invocation starts where the last left off.
# Wiped only by: rm -rf /tmp/stbi_torture_corpus*
PERSIST_OURS="/tmp/stbi_torture_corpus_ours"
PERSIST_UP="/tmp/stbi_torture_corpus_upstream"
mkdir -p "$PERSIST_OURS" "$PERSIST_UP"

FAIL=0
banner() { printf '\n==== %s ====\n' "$1" | tee -a "$LOG"; }
pass()   { printf '  OK    %s\n' "$1"    | tee -a "$LOG"; }
fail()   { printf '  FAIL  %s\n' "$1"    | tee -a "$LOG"; FAIL=1; }

# ---- Sanitizer flag sets ---------------------------------------------------
#
# AUBSAN_BASE: the classic ASan+UBSan pair. "undefined" enables the default
# UBSan subset (includes float-cast-overflow, signed-integer-overflow, shift,
# null, etc.) but *not* unsigned-integer-overflow / implicit-conversion.
#
# AUBSAN_STRICT: same + the UBSan checks that aren't in the default umbrella.
# We use this on clang only (gcc doesn't recognise all of them).
AUBSAN_BASE="-fsanitize=address,undefined"
AUBSAN_STRICT="-fsanitize=address,undefined,integer,implicit-conversion,nullability \
-fno-sanitize-recover=all \
-fno-sanitize=unsigned-integer-overflow,implicit-unsigned-integer-truncation,implicit-integer-sign-change,unsigned-shift-base"
#   ^ the sanitizers we *leave out* above report on code paths that use modular
#   arithmetic intentionally (size calculations cap via the mad*sizes_valid
#   helpers, zlib bit buffer, etc.). Keeping them on would create false
#   positives that mask real findings.

# ============================================================================
# Phase 1 — random + seeded fuzz under ASan+UBSan
# ============================================================================
banner "Phase 1 — random + seeded fuzz under ASan+UBSan"
declare -A FUZZ_VARIANTS=(
  [gcc_O0]="gcc -O0"
  [gcc_O2]="gcc -O2"
  [clang_O0]="clang -O0"
  [clang_O2]="clang -O2"
)
ITER=30000
BUFLEN=32768

for name in "${!FUZZ_VARIANTS[@]}"; do
  bin="$BUILD/fuzz2_$name"
  cmd=${FUZZ_VARIANTS[$name]}
  if $cmd -std=c99 -g -Wall -Wextra -Wpedantic $AUBSAN_BASE \
        -I include tests/fuzz2.c -o "$bin" -lm >> "$LOG" 2>&1
  then
      if ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:halt_on_error=1 \
         UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1:abort_on_error=1 \
         "$bin" "$ITER" "$BUFLEN" >> "$LOG" 2>&1
      then pass "fuzz2 $name ($ITER iters x $BUFLEN bytes)"
      else fail "fuzz2 $name ($ITER iters x $BUFLEN bytes)"
      fi
  else
      fail "fuzz2 $name build"
  fi
done

# Phase 1b: the strict-UBSan variant on clang -O0 only. Only the check
# categories that don't intentionally rely on modular arithmetic.
banner "Phase 1b — strict UBSan checks (clang -O0)"
bin="$BUILD/fuzz2_strict"
if clang -std=c99 -O0 -g -Wall -Wextra -Wpedantic $AUBSAN_STRICT \
     -I include tests/fuzz2.c -o "$bin" -lm >> "$LOG" 2>&1
then
    if UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1:abort_on_error=1 \
       "$bin" 10000 16384 >> "$LOG" 2>&1
    then pass "fuzz2 strict-UBSan (10k iters x 16k bytes)"
    else fail "fuzz2 strict-UBSan (10k iters x 16k bytes)"
    fi
else
    fail "fuzz2 strict-UBSan build"
fi

# ============================================================================
# Phase 2 — full unit & regression suites under ASan+UBSan
# ============================================================================
banner "Phase 2 — full unit & regression suites under ASan+UBSan"
for suite in driver adversarial upstream_issues; do
  bin="$BUILD/${suite}_san"
  if clang -std=c99 -O1 -g -Wall -Wextra -Wpedantic $AUBSAN_BASE \
       -I include tests/${suite}.c -o "$bin" -lm >> "$LOG" 2>&1
  then
    if ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:halt_on_error=1 \
       UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1 \
       "$bin" >> "$LOG" 2>&1
    then pass "$suite (clang ASan+UBSan)"
    else fail "$suite (clang ASan+UBSan)"
    fi
  else
      fail "$suite build"
  fi
done

# ============================================================================
# Phase 3 — MSan (uninit read detection)
# ============================================================================
banner "Phase 3 — MSan (uninit read detection)"
for suite in driver upstream_issues; do
  bin="$BUILD/${suite}_msan"
  if clang -std=c99 -O1 -g -Wall -Wextra -Wpedantic -fsanitize=memory \
       -fno-omit-frame-pointer -fsanitize-memory-track-origins=2 \
       -I include tests/${suite}.c -o "$bin" -lm >> "$LOG" 2>&1
  then
    if MSAN_OPTIONS=print_stats=0:halt_on_error=1:abort_on_error=1 \
       "$bin" >> "$LOG" 2>&1
    then pass "$suite (MSan)"
    else fail "$suite (MSan)"
    fi
  else
    fail "$suite MSan build"
  fi
done

bin="$BUILD/fuzz2_msan"
if clang -std=c99 -O1 -g -fsanitize=memory -fno-omit-frame-pointer \
     -fsanitize-memory-track-origins=2 \
     -I include tests/fuzz2.c -o "$bin" -lm >> "$LOG" 2>&1
then
  if MSAN_OPTIONS=halt_on_error=1:abort_on_error=1 "$bin" 10000 8192 >> "$LOG" 2>&1
  then pass "fuzz2 (MSan 10k x 8k)"
  else fail "fuzz2 (MSan 10k x 8k)"
  fi
else
    fail "fuzz2 MSan build"
fi

# ============================================================================
# Phase 3b — upstream PngSuite golden-hash regression
# ============================================================================
banner "Phase 3b — upstream PngSuite golden-hash regression"
bin="$BUILD/upstream_pngsuite_san"
if clang -std=c99 -O1 -g -Wall -Wextra -Wpedantic $AUBSAN_BASE \
     -I include tests/upstream_pngsuite.c -o "$bin" -lm >> "$LOG" 2>&1
then
    if ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:halt_on_error=1 \
       UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1 \
       "$bin" tests >> "$LOG" 2>&1
    then pass "upstream PngSuite hash match (258 files)"
    else fail "upstream PngSuite hash match (see log)"
    fi
else
    fail "upstream PngSuite build"
fi

# ============================================================================
# Phase 3c — regression corpus (every historical PoC)
#   Each file in tests/regression_corpus/ is fed through the standalone fuzz
#   harness under ASan+UBSan and then MSan. Any non-zero exit is a regression.
# ============================================================================
banner "Phase 3c — regression corpus replay"
RCBIN_AU="$BUILD/regression_au"
RCBIN_MS="$BUILD/regression_ms"
ok=1
if ! clang -std=c99 -O1 -g $AUBSAN_BASE \
     -I include tests/upstream_fuzzer/fuzz_main.c tests/upstream_fuzzer/stbi_read_fuzzer.c \
     -o "$RCBIN_AU" -lm >> "$LOG" 2>&1
then fail "regression ASan+UBSan build"; ok=0
fi
if ! clang -std=c99 -O1 -g -fsanitize=memory -fno-omit-frame-pointer \
     -fsanitize-memory-track-origins=2 \
     -I include tests/upstream_fuzzer/fuzz_main.c tests/upstream_fuzzer/stbi_read_fuzzer.c \
     -o "$RCBIN_MS" -lm >> "$LOG" 2>&1
then fail "regression MSan build"; ok=0
fi

if [ $ok = 1 ]; then
    n_total=0 n_bad=0
    for f in tests/regression_corpus/*; do
        n_total=$((n_total + 1))
        bad=0
        if ! ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:halt_on_error=1 \
             UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1 \
             "$RCBIN_AU" "$f" >> "$LOG" 2>&1
        then echo "  bad ASan+UBSan: $f" >> "$LOG"; bad=1
        fi
        if ! MSAN_OPTIONS=halt_on_error=1:abort_on_error=1 \
             "$RCBIN_MS" "$f" >> "$LOG" 2>&1
        then echo "  bad MSan: $f" >> "$LOG"; bad=1
        fi
        n_bad=$((n_bad + bad))
    done
    if [ $n_bad = 0 ]; then
        pass "regression corpus replay ($n_total files × ASan+UBSan + MSan)"
    else
        fail "regression corpus replay ($n_bad / $n_total files regressed — see log)"
    fi
fi

# ============================================================================
# Phase 4 — libFuzzer, coverage-guided, 180 s per sub-phase
#
# We use richer seed files than before — anything in tests/regression_corpus/
# plus small format-magic stubs — so the fuzzer spends its budget exploring
# payloads, not recovering a valid header.
# ============================================================================
banner "Phase 4 — libFuzzer coverage-guided (180s each sub-phase)"

# Seed files that libFuzzer starts from. We reuse the regression corpus (every
# past PoC) plus the primary PngSuite images; both subdirs are read-only seeds.
SEEDS="$BUILD/seeds"
rm -rf "$SEEDS"; mkdir -p "$SEEDS"
cp tests/regression_corpus/* "$SEEDS/"
# add a handful of PngSuite primary cases to seed PNG exploration
cp tests/pngsuite/primary/basn0g08.png "$SEEDS/seed_png_gray8.png" 2>/dev/null || true
cp tests/pngsuite/primary/basn2c08.png "$SEEDS/seed_png_rgb8.png" 2>/dev/null || true
cp tests/pngsuite/primary/basn6a08.png "$SEEDS/seed_png_rgba8.png" 2>/dev/null || true
cp tests/pngsuite/primary/basn3p08.png "$SEEDS/seed_png_pal8.png" 2>/dev/null || true
cp tests/pngsuite/16bit/basn6a16.png   "$SEEDS/seed_png_rgba16.png" 2>/dev/null || true
cp tests/pngsuite/iphone/iphone_xxxx.png "$SEEDS/seed_png_cgbi.png" 2>/dev/null || true

LIBFUZZ_TIME=180
FUZZ_LIMITS="-DSTBI_MAX_DIMENSIONS=4096"

# Our entry, ASan+UBSan
LFBIN="$BUILD/libfuzz_asan"
if clang -std=c99 -O1 -g \
     -fsanitize=address,undefined,fuzzer \
     $FUZZ_LIMITS \
     -I include tests/libfuzz_entry.c -o "$LFBIN" -lm >> "$LOG" 2>&1
then
    if ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:halt_on_error=1 \
       UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1 \
       "$LFBIN" -max_total_time=$LIBFUZZ_TIME -rss_limit_mb=2048 \
       -artifact_prefix="$BUILD/crash-asan-" \
       "$PERSIST_OURS" "$SEEDS" >> "$LOG" 2>&1
    then pass "libFuzzer ours ASan+UBSan (${LIBFUZZ_TIME}s)"
    else fail "libFuzzer ours ASan+UBSan (${LIBFUZZ_TIME}s)"
    fi
else
    fail "libFuzzer ours ASan build"
fi

# Upstream entry, ASan+UBSan (mirrors OSS-Fuzz)
UPBIN="$BUILD/libfuzz_upstream"
if clang -std=c99 -O1 -g \
     -fsanitize=address,undefined,fuzzer \
     $FUZZ_LIMITS \
     -I include tests/upstream_fuzzer/stbi_read_fuzzer.c -o "$UPBIN" -lm >> "$LOG" 2>&1
then
    if ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:halt_on_error=1 \
       UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1 \
       "$UPBIN" -max_total_time=$LIBFUZZ_TIME -rss_limit_mb=2048 \
       -dict=tests/upstream_fuzzer/stb_png.dict \
       -artifact_prefix="$BUILD/crash-upstream-" \
       "$PERSIST_UP" "$SEEDS" >> "$LOG" 2>&1
    then pass "libFuzzer upstream ASan+UBSan (${LIBFUZZ_TIME}s)"
    else fail "libFuzzer upstream ASan+UBSan (${LIBFUZZ_TIME}s)"
    fi
else
    fail "libFuzzer upstream ASan build"
fi

# Our entry, MSan
LFBIN="$BUILD/libfuzz_msan"
if clang -std=c99 -O1 -g -fsanitize=memory,fuzzer \
     -fno-omit-frame-pointer -fsanitize-memory-track-origins=2 \
     $FUZZ_LIMITS \
     -I include tests/libfuzz_entry.c -o "$LFBIN" -lm >> "$LOG" 2>&1
then
    if MSAN_OPTIONS=halt_on_error=1:abort_on_error=1 \
       "$LFBIN" -max_total_time=$LIBFUZZ_TIME -rss_limit_mb=2048 \
       -artifact_prefix="$BUILD/crash-msan-" \
       "$PERSIST_OURS" "$SEEDS" >> "$LOG" 2>&1
    then pass "libFuzzer ours MSan (${LIBFUZZ_TIME}s)"
    else fail "libFuzzer ours MSan (${LIBFUZZ_TIME}s)"
    fi
else
    fail "libFuzzer ours MSan build"
fi

# Upstream entry, MSan
UPBIN="$BUILD/libfuzz_upstream_msan"
if clang -std=c99 -O1 -g -fsanitize=memory,fuzzer \
     -fno-omit-frame-pointer -fsanitize-memory-track-origins=2 \
     $FUZZ_LIMITS \
     -I include tests/upstream_fuzzer/stbi_read_fuzzer.c -o "$UPBIN" -lm >> "$LOG" 2>&1
then
    if MSAN_OPTIONS=halt_on_error=1:abort_on_error=1 \
       "$UPBIN" -max_total_time=$LIBFUZZ_TIME -rss_limit_mb=2048 \
       -dict=tests/upstream_fuzzer/stb_png.dict \
       -artifact_prefix="$BUILD/crash-upstream-msan-" \
       "$PERSIST_UP" "$SEEDS" >> "$LOG" 2>&1
    then pass "libFuzzer upstream MSan (${LIBFUZZ_TIME}s)"
    else fail "libFuzzer upstream MSan (${LIBFUZZ_TIME}s)"
    fi
else
    fail "libFuzzer upstream MSan build"
fi

# Run upstream's fuzz_main against the full regression corpus (standalone, not
# libFuzzer — catches any corpus file that the read-fuzzer crashes on)
STANDALONE="$BUILD/upstream_fuzz_standalone"
if clang -std=c99 -O1 -g \
     -fsanitize=address,undefined \
     $FUZZ_LIMITS \
     -I include tests/upstream_fuzzer/fuzz_main.c tests/upstream_fuzzer/stbi_read_fuzzer.c \
     -o "$STANDALONE" -lm >> "$LOG" 2>&1
then
    ok=1
    for seed in "$SEEDS"/*; do
        if ! ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:halt_on_error=1 \
             UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1 \
             "$STANDALONE" "$seed" >> "$LOG" 2>&1
        then ok=0; break
        fi
    done
    if [ $ok = 1 ]; then
        pass "upstream fuzz_main standalone (every seed)"
    else
        fail "upstream fuzz_main standalone (crash on a seed)"
    fi
else
    fail "upstream fuzz_main standalone build"
fi

# ============================================================================
# Phase 5 — static analyzers
# ============================================================================
banner "Phase 5 — static analyzers"
if gcc -std=c99 -O2 -Wall -Wextra -Wpedantic -fanalyzer \
     -I include tests/upstream_issues.c -o "$BUILD/analyzer_gcc" -lm \
     >> "$LOG" 2>&1
then pass "gcc -fanalyzer"
else fail "gcc -fanalyzer"
fi

nwarn=$(clang --analyze -std=c99 -I include \
          -Xclang -analyzer-output=text \
          -Xclang -analyzer-checker=core,deadcode,security,unix,nullability \
          tests/upstream_issues.c 2>&1 | grep -c "warning:" | tr -d '[:space:]')
nnoise=$(clang --analyze -std=c99 -I include \
          -Xclang -analyzer-output=text \
          -Xclang -analyzer-checker=core,deadcode,security,unix,nullability \
          tests/upstream_issues.c 2>&1 | grep "warning:" | grep -c "deadcode.DeadStores" | tr -d '[:space:]')
if [ "$nwarn" = "$nnoise" ]; then
    pass "clang --analyze (deadcode-only: $nwarn)"
else
    fail "clang --analyze: $nwarn - $nnoise dead-store = non-noise warnings"
fi

# ============================================================================
# Phase 6 — build-config sweep
# ============================================================================
banner "Phase 6 — build-config sweep"
CONFIGS=(
  ""
  "-DSTBI_NO_JPEG"
  "-DSTBI_NO_PNG"
  "-DSTBI_NO_GIF"
  "-DSTBI_NO_BMP"
  "-DSTBI_NO_TGA"
  "-DSTBI_NO_PSD"
  "-DSTBI_NO_PIC"
  "-DSTBI_NO_PNM"
  "-DSTBI_NO_HDR -DSTBI_NO_LINEAR"
  "-DSTBI_NO_STDIO"
  "-DSTBI_NO_SIMD"
  "-DSTBI_NO_FAILURE_STRINGS"
  "-DSTBI_FAILURE_USERMSG"
  "-DSTBI_NO_JPEG -DSTBI_NO_PNG -DSTBI_NO_GIF -DSTBI_NO_BMP -DSTBI_NO_TGA -DSTBI_NO_PSD -DSTBI_NO_PIC"
  "-DSTBI_NO_HDR -DSTBI_NO_LINEAR -DSTBI_NO_PNM -DSTBI_NO_STDIO"
  "-DSTBI_MAX_DIMENSIONS=2048"
  "-DSTBI_MAX_DIMENSIONS=65536"
)
for cfg in "${CONFIGS[@]}"; do
  label="build ${cfg:-default}"
  if gcc -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-function \
       $cfg -I include tests/driver.c -o "$BUILD/cfgtest" -lm >> "$LOG" 2>&1
  then pass "$label"
  else fail "$label"
  fi
done

# ============================================================================
# SUMMARY
# ============================================================================
banner "SUMMARY"
if [ "$FAIL" = 0 ]; then
    echo "ALL TORTURE PHASES PASSED" | tee -a "$LOG"
    exit 0
else
    echo "TORTURE TEST FAILED — see $LOG" | tee -a "$LOG"
    echo "Tail:"
    tail -80 "$LOG"
    exit 1
fi
