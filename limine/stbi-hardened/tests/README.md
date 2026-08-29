# tests

Test suite for `include/stb_image.h`.

## Quick start

```sh
make            # build the default ASan+UBSan test binaries
make test       # run them all
make torture    # the full torture suite (~15 min)
```

Use `make help` for the target list.

## What's here

| File / dir | Purpose |
|---|---|
| `Makefile` | the entry point for all of the below |
| `torture.sh` | the full sanitizer × compiler × opt × fuzz matrix |
| `driver.c` | sanity unit tests: empty buffers, NULL args, valid 2×2 PNG |
| `adversarial.c` | hand-crafted adversarial inputs (huge dims, bad offsets, etc.) |
| `upstream_issues.c` | one regression case per upstream nothings/stb issue we closed |
| `upstream_pngsuite.c` | golden-hash regression against the upstream PngSuite |
| `fuzz2.c` | random + seeded fuzzer, standalone (no libFuzzer needed) |
| `libfuzz_entry.c` | our `LLVMFuzzerTestOneInput`; drives every API path |
| `upstream_fuzzer/` | upstream's OSS-Fuzz harness, unmodified except for the include path |
| `pngsuite/` | upstream's 258-file PNG corpus + `ref_results.csv` (CC-by-upstream) |
| `regression_corpus/` | every PoC that has ever triggered a bug in this codebase |

## How to add a new regression case

Any time a fuzzer or a user finds a new bug, do all three:

1. Drop the failing input into `regression_corpus/` with a descriptive name.
2. Add a named case to `upstream_issues.c` that loads the input and asserts the expected behaviour (typically "does not crash" or "rejects cleanly").
3. `make test` — the torture suite replays the whole corpus on every future run, so the bug can't silently come back.

## Sanitizer coverage

| Sanitizer | Where it runs |
|---|---|
| ASan | every phase except the static analyzers |
| UBSan (default `-fsanitize=undefined`) | same as ASan |
| UBSan strict (`integer,implicit-conversion,nullability`) | Phase 1b of `torture.sh` |
| MSan | Phase 3 of `torture.sh` + `make msan` |
| libFuzzer + ASan+UBSan | Phase 4 |
| libFuzzer + MSan | Phase 4 |
| `gcc -fanalyzer` | Phase 5 |
| `clang --analyze` (core/security/unix/nullability) | Phase 5 |

## Torture test phases

Each row either passes or fails the whole suite:

1. **Phase 1** — `fuzz2` under gcc×O0, gcc×O2, clang×O0, clang×O2 with ASan+UBSan (30k iterations × 32 KB each).
2. **Phase 1b** — `fuzz2` under clang -O0 with the strict UBSan checks that are not in the default umbrella.
3. **Phase 2** — `driver`, `adversarial`, `upstream_issues` under ASan+UBSan.
4. **Phase 3** — same three + `fuzz2` under MSan.
5. **Phase 3b** — every PngSuite image must produce upstream's exact FNV-1a-32 hash (or the exact same error).
6. **Phase 3c** — every `regression_corpus/` file under ASan+UBSan and MSan.
7. **Phase 4** — libFuzzer for 180 s per sub-phase, four sub-phases:
   - our `libfuzz_entry.c`, ASan+UBSan, persistent corpus
   - upstream's `stbi_read_fuzzer.c`, ASan+UBSan, with the PNG dictionary
   - our entry, MSan
   - upstream's entry, MSan
   - plus the standalone `upstream fuzz_main` against every seed
8. **Phase 5** — `gcc -fanalyzer` and `clang --analyze`.
9. **Phase 6** — 18 `#define` combinations of `STBI_NO_*` / `STBI_FAILURE_*` / `STBI_MAX_DIMENSIONS=…` to make sure every decoder toggle still builds cleanly.

libFuzzer corpora are persistent between runs (at `/tmp/stbi_torture_corpus_*`), so each torture run picks up where the last left off and coverage compounds. `make distclean` wipes them.

## Overriding variables

```sh
make CC=gcc           # use gcc instead of cc
make OPT=-O2          # crank the optimiser
make FUZZ_ITER=100000 # longer `make fuzz`
```
