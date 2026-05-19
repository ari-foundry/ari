# Source-library build entry point.
#
# This is intentionally small until Ari has a package manager. The root
# Makefile delegates here so library-oriented builds can grow without turning
# the compiler build file into a package manifest.

ROOT ?= ..
ARI ?= build/ari
LLVM_CC ?= clang
BUILD_DIR ?= build/lib

STD_SOURCES := std.arih $(sort $(wildcard std/*.arih))
STD_API_CHECKER := tests/check_std_api_manifest.py
STD_SMOKE := tests/cases/standard-library/ok/smoke/std-library-smoke.ari
STD_SMOKE_PATH := $(ROOT)/$(STD_SMOKE)

# LIBRARY is relative to ROOT because recipes run Ari from the repository root.
LIBRARY ?= tests/cases/ffi/ok/library-export.ari
LIBRARY_PATH := $(ROOT)/$(LIBRARY)
LIBRARY_NAME ?= ari_sample
LIBRARY_BASE := $(BUILD_DIR)/lib$(LIBRARY_NAME)
LIBRARY_LL := $(LIBRARY_BASE).ll
LIBRARY_OBJ := $(LIBRARY_BASE).o
LIBRARY_HEADER := $(LIBRARY_BASE).h
LIBRARY_SO := $(LIBRARY_BASE).so
LIBRARY_SYMBOL ?= _ARNv3add

.PHONY: all build check check-api prepare std-smoke shared-library clean

all: build

build: std-smoke shared-library

check: check-api build

check-api:
	cd $(ROOT) && python3 $(STD_API_CHECKER)

prepare:
	mkdir -p $(ROOT)/$(BUILD_DIR)

std-smoke: prepare $(STD_SOURCES) $(STD_SMOKE_PATH)
	cd $(ROOT) && $(ARI) $(STD_SMOKE) --emit-module-metadata $(BUILD_DIR)/std-smoke.arimeta --emit-module-cache $(BUILD_DIR)/std-smoke.aricache --emit-llvm $(BUILD_DIR)/std-smoke.ll
	cd $(ROOT) && grep -q 'define i64 @"_ARNv4main"' $(BUILD_DIR)/std-smoke.ll
	cd $(ROOT) && $(ARI) $(STD_SMOKE) --check-module-metadata $(BUILD_DIR)/std-smoke.arimeta --check
	cd $(ROOT) && $(ARI) $(STD_SMOKE) -o $(BUILD_DIR)/std-smoke.elf
	@cd $(ROOT) && set +e; $(BUILD_DIR)/std-smoke.elf; code=$$?; test $$code -eq 232

shared-library: prepare $(LIBRARY_PATH)
	cd $(ROOT) && $(ARI) $(LIBRARY) --shared --emit-c-header $(LIBRARY_HEADER) --emit-llvm $(LIBRARY_LL)
	@if test -n "$(LIBRARY_SYMBOL)"; then cd $(ROOT) && grep -q '$(LIBRARY_SYMBOL)' $(LIBRARY_LL); fi
	cd $(ROOT) && test -s $(LIBRARY_HEADER)
	cd $(ROOT) && $(ARI) $(LIBRARY) --emit-c-header $(LIBRARY_HEADER) --emit-obj $(LIBRARY_OBJ)
	cd $(ROOT) && test -s $(LIBRARY_OBJ)
	@if command -v $(LLVM_CC) >/dev/null 2>&1; then cd $(ROOT) && $(ARI) $(LIBRARY) --llvm-cc $(LLVM_CC) --shared -o $(LIBRARY_SO) && test -s $(LIBRARY_SO); fi

clean:
	rm -rf $(ROOT)/$(BUILD_DIR)
