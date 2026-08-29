# Makefile for the stbi-hardened test suite.
#
#   make           build the ASan+UBSan test binaries
#   make test      run the unit + regression + pngsuite + regression-corpus suites
#   make fuzz      10k random-input fuzz iterations under ASan+UBSan
#   make fuzz-long 100k iterations
#   make msan      MSan build + a 10k MSan fuzz
#   make analyze   gcc -fanalyzer + clang --analyze
#   make pngsuite  upstream PngSuite golden-hash regression (258 files)
#   make regression replay every tests/regression_corpus/ PoC
#   make libfuzz   quick libFuzzer build + 60s run (needs clang)
#   make torture   the full torture.sh (15 min, all sanitizers)
#   make clean     remove build artifacts
#   make distclean also wipes /tmp/stbi_torture* persistent corpora
#
# Variables you can override:
#   CC, STD, OPT, WARN, DEBUG, SAN_FLAGS, MSAN_FLAGS, FUZZ_ITER, FUZZ_LEN

CC       ?= clang
STD      ?= -std=c99
OPT      ?= -O1
WARN     ?= -Wall -Wextra -Wpedantic
DEBUG    ?= -g
INCLUDE  := -I../include
CFLAGS   := $(STD) $(OPT) $(DEBUG) $(WARN) $(INCLUDE)
LDLIBS   := -lm

SAN_FLAGS  := -fsanitize=address,undefined
MSAN_FLAGS := -fsanitize=memory -fno-omit-frame-pointer -fsanitize-memory-track-origins=2

BUILD    := build
SUITES   := driver adversarial upstream_issues upstream_pngsuite regress_gif_delays
FUZZ_ITER ?= 10000
FUZZ_LEN  ?= 16384

SAN_BINS   := $(SUITES:%=$(BUILD)/%_san) $(BUILD)/fuzz2_san
MSAN_BINS  := $(BUILD)/driver_msan $(BUILD)/upstream_issues_msan $(BUILD)/fuzz2_msan
REG_BIN    := $(BUILD)/regression_au
REG_MSAN   := $(BUILD)/regression_msan

ASAN_OPT := detect_leaks=1:abort_on_error=1:halt_on_error=1
UBSAN_OPT := print_stacktrace=1:halt_on_error=1:abort_on_error=1
MSAN_OPT := halt_on_error=1:abort_on_error=1

.PHONY: all test fuzz fuzz-long msan libfuzz pngsuite regression analyze torture clean distclean help

all: $(SAN_BINS)

$(BUILD):
	@mkdir -p $@

# Generic build rules --------------------------------------------------------
$(BUILD)/%_san: %.c | $(BUILD)
	$(CC) $(CFLAGS) $(SAN_FLAGS) $< -o $@ $(LDLIBS)

$(BUILD)/%_msan: %.c | $(BUILD)
	$(CC) $(CFLAGS) $(MSAN_FLAGS) $< -o $@ $(LDLIBS)

# The regression replay harness reuses upstream's fuzz_main + stbi_read_fuzzer
$(REG_BIN): upstream_fuzzer/fuzz_main.c upstream_fuzzer/stbi_read_fuzzer.c | $(BUILD)
	$(CC) $(CFLAGS) $(SAN_FLAGS) $^ -o $@ $(LDLIBS)

$(REG_MSAN): upstream_fuzzer/fuzz_main.c upstream_fuzzer/stbi_read_fuzzer.c | $(BUILD)
	$(CC) $(CFLAGS) $(MSAN_FLAGS) $^ -o $@ $(LDLIBS)

# Targets --------------------------------------------------------------------
test: $(SAN_BINS) $(REG_BIN)
	@set -e; for s in $(SUITES); do \
	   echo "=== $$s (ASan+UBSan) ==="; \
	   ASAN_OPTIONS=$(ASAN_OPT) UBSAN_OPTIONS=$(UBSAN_OPT) ./$(BUILD)/$${s}_san . ; \
	done
	@echo "=== regression corpus replay ==="
	@set -e; for f in regression_corpus/*; do \
	   ASAN_OPTIONS=$(ASAN_OPT) UBSAN_OPTIONS=$(UBSAN_OPT) ./$(REG_BIN) "$$f" ; \
	done
	@echo "all tests OK"

fuzz: $(BUILD)/fuzz2_san
	ASAN_OPTIONS=$(ASAN_OPT) UBSAN_OPTIONS=$(UBSAN_OPT) \
	    ./$(BUILD)/fuzz2_san $(FUZZ_ITER) $(FUZZ_LEN)

fuzz-long: $(BUILD)/fuzz2_san
	ASAN_OPTIONS=$(ASAN_OPT) UBSAN_OPTIONS=$(UBSAN_OPT) \
	    ./$(BUILD)/fuzz2_san 100000 32768

msan: $(MSAN_BINS) $(REG_MSAN)
	@set -e; MSAN_OPTIONS=$(MSAN_OPT) ./$(BUILD)/driver_msan . ; \
	    MSAN_OPTIONS=$(MSAN_OPT) ./$(BUILD)/upstream_issues_msan . ; \
	    MSAN_OPTIONS=$(MSAN_OPT) ./$(BUILD)/fuzz2_msan 10000 8192 ; \
	    for f in regression_corpus/*; do \
	       MSAN_OPTIONS=$(MSAN_OPT) ./$(REG_MSAN) "$$f" ; \
	    done
	@echo "MSan OK"

libfuzz: $(BUILD)/libfuzz_san | $(BUILD)/corpus
	ASAN_OPTIONS=$(ASAN_OPT) UBSAN_OPTIONS=$(UBSAN_OPT) \
	    ./$(BUILD)/libfuzz_san -max_total_time=60 -rss_limit_mb=2048 \
	    -artifact_prefix=$(BUILD)/crash- \
	    $(BUILD)/corpus regression_corpus

$(BUILD)/corpus: | $(BUILD)
	@mkdir -p $@

$(BUILD)/libfuzz_san: libfuzz_entry.c | $(BUILD)
	clang $(CFLAGS) -fsanitize=address,undefined,fuzzer \
	    -DSTBI_MAX_DIMENSIONS=4096 $< -o $@ $(LDLIBS)

pngsuite: $(BUILD)/upstream_pngsuite_san
	ASAN_OPTIONS=$(ASAN_OPT) UBSAN_OPTIONS=$(UBSAN_OPT) \
	    ./$(BUILD)/upstream_pngsuite_san .

regression: $(REG_BIN)
	@set -e; for f in regression_corpus/*; do \
	   ASAN_OPTIONS=$(ASAN_OPT) UBSAN_OPTIONS=$(UBSAN_OPT) ./$(REG_BIN) "$$f" ; \
	done
	@echo "regression corpus OK"

analyze:
	@gcc -std=c99 -O2 -Wall -Wextra -Wpedantic -fanalyzer \
	    $(INCLUDE) upstream_issues.c -o /dev/null $(LDLIBS) \
	    && echo "gcc -fanalyzer: clean"
	@clang --analyze -std=c99 $(INCLUDE) \
	    -Xclang -analyzer-output=text \
	    -Xclang -analyzer-checker=core,deadcode,security,unix,nullability \
	    upstream_issues.c 2>&1 \
	    | { nonnoise="$$(grep 'warning:' | grep -v deadcode.DeadStores || true)"; \
	        if [ -z "$$nonnoise" ]; then echo "clang --analyze: clean (dead-store noise only)"; \
	        else echo "$$nonnoise"; exit 1; fi; }

torture:
	./torture.sh

clean:
	rm -rf $(BUILD) *.plist

distclean: clean
	rm -rf /tmp/stbi_torture /tmp/stbi_torture_corpus_ours /tmp/stbi_torture_corpus_upstream

help:
	@sed -n '/^#.*make/,/^[A-Z]/p' Makefile | sed -n '/^#/p'
