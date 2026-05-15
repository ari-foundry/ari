CXX ?= c++
CC ?= cc
LLVM_CC_CANDIDATES := clang clang-21 clang-20 clang-19 clang-18 clang-17 clang-16 clang-15 clang-14
LLVM_CC_PATH_CANDIDATES := $(wildcard /usr/bin/clang /usr/bin/clang-[0-9]* /usr/lib/llvm-*/bin/clang)
LLVM_CC_DETECTED := $(firstword $(foreach cc,$(LLVM_CC_CANDIDATES),$(shell command -v $(cc) 2>/dev/null)) $(LLVM_CC_PATH_CANDIDATES))
LLVM_CC ?= $(if $(LLVM_CC_DETECTED),$(LLVM_CC_DETECTED),clang)
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
DEBUG_CXXFLAGS ?= -std=c++17 -O0 -g3 -Wall -Wextra -Wpedantic -DARI_DEBUG=1
SANITIZE_CXXFLAGS ?= -std=c++17 -O1 -g3 -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer -DARI_DEBUG=1
EXEEXT ?=

BUILD_DIR := build
TARGET := $(BUILD_DIR)/ari$(EXEEXT)
DEBUG_TARGET := $(BUILD_DIR)/debug/ari$(EXEEXT)
SANITIZE_TARGET := $(BUILD_DIR)/sanitize/ari$(EXEEXT)
LINT_TARGET := $(BUILD_DIR)/ari-lint$(EXEEXT)
LSP_TARGET := $(BUILD_DIR)/ari-lsp$(EXEEXT)
SRC := $(wildcard src/*.cpp)
HEADERS := $(wildcard src/*.hpp)
TOOLING_SRC := $(wildcard tools/ari_tooling/*.cpp)
TOOLING_HEADERS := $(wildcard tools/ari_tooling/*.hpp)
LINT_SRC := $(wildcard tools/lint/*.cpp)
LINT_HEADERS := $(wildcard tools/lint/*.hpp)
LSP_SRC := $(wildcard tools/lsp/*.cpp)
LSP_HEADERS := $(wildcard tools/lsp/*.hpp)

.PHONY: all release debug sanitize tools lint lsp clean sample

all: $(TARGET)
release: $(TARGET)
debug: $(DEBUG_TARGET)
sanitize: $(SANITIZE_TARGET)
tools: lint lsp
lint: $(LINT_TARGET)
lsp: $(LSP_TARGET)

$(TARGET): $(SRC) $(HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $@

$(DEBUG_TARGET): $(SRC) $(HEADERS)
	mkdir -p $(BUILD_DIR)/debug
	$(CXX) $(DEBUG_CXXFLAGS) $(SRC) -o $@

$(SANITIZE_TARGET): $(SRC) $(HEADERS)
	mkdir -p $(BUILD_DIR)/sanitize
	$(CXX) $(SANITIZE_CXXFLAGS) $(SRC) -o $@

$(LINT_TARGET): $(LINT_SRC) $(LINT_HEADERS) $(TOOLING_SRC) $(TOOLING_HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TOOLING_SRC) $(LINT_SRC) -o $@

$(LSP_TARGET): $(LSP_SRC) $(LSP_HEADERS) $(TOOLING_SRC) $(TOOLING_HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TOOLING_SRC) $(LSP_SRC) -o $@

sample: $(TARGET)
	$(TARGET) examples/count.ari --emit-llvm $(BUILD_DIR)/count.ll

include tests/Makefile

clean:
	rm -rf $(BUILD_DIR)
