CXX ?= c++
CC ?= cc
LLVM_CC_CANDIDATES := clang clang-21 clang-20 clang-19 clang-18 clang-17 clang-16 clang-15 clang-14
LLVM_CC_PATH_CANDIDATES := $(wildcard /usr/bin/clang /usr/bin/clang-[0-9]* /usr/lib/llvm-*/bin/clang)
LLVM_CC_DETECTED := $(firstword $(foreach cc,$(LLVM_CC_CANDIDATES),$(shell command -v $(cc) 2>/dev/null)) $(LLVM_CC_PATH_CANDIDATES))
LLVM_CC ?= $(if $(LLVM_CC_DETECTED),$(LLVM_CC_DETECTED),clang)
CPPFLAGS ?=
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
DEBUG_CXXFLAGS ?= -std=c++17 -O0 -g3 -Wall -Wextra -Wpedantic -DARI_DEBUG=1
SANITIZE_CXXFLAGS ?= -std=c++17 -O1 -g3 -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer -DARI_DEBUG=1
DEPFLAGS ?= -MMD -MP
LDFLAGS ?=
LDLIBS ?=
EXEEXT ?=
PREFIX ?= $(HOME)/.local
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
ARI_SHAREDIR ?= $(DATADIR)/ari
INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL) -m 755
INSTALL_DATA ?= $(INSTALL) -m 644
INSTALL_DIR ?= $(INSTALL) -d

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
RELEASE_OBJ_DIR := $(OBJ_DIR)/release
DEBUG_OBJ_DIR := $(OBJ_DIR)/debug
SANITIZE_OBJ_DIR := $(OBJ_DIR)/sanitize
TOOLS_OBJ_DIR := $(OBJ_DIR)/tools
TARGET := $(BUILD_DIR)/ari$(EXEEXT)
DEBUG_TARGET := $(BUILD_DIR)/debug/ari$(EXEEXT)
SANITIZE_TARGET := $(BUILD_DIR)/sanitize/ari$(EXEEXT)
LINT_TARGET := $(BUILD_DIR)/ari-lint$(EXEEXT)
LSP_TARGET := $(BUILD_DIR)/ari-lsp$(EXEEXT)
SRC := $(wildcard src/*.cpp)
TOOLING_SRC := $(wildcard tools/ari_tooling/*.cpp)
LINT_SRC := $(wildcard tools/lint/*.cpp)
LINT_LIB_SRC := $(filter-out tools/lint/main.cpp,$(LINT_SRC))
LSP_SRC := $(wildcard tools/lsp/*.cpp)
STD_MODULE_SRCS := $(sort $(wildcard lib/std/*.arih))
RELEASE_OBJS := $(patsubst %.cpp,$(RELEASE_OBJ_DIR)/%.o,$(SRC))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJ_DIR)/%.o,$(SRC))
SANITIZE_OBJS := $(patsubst %.cpp,$(SANITIZE_OBJ_DIR)/%.o,$(SRC))
TOOLING_OBJS := $(patsubst tools/%.cpp,$(TOOLS_OBJ_DIR)/%.o,$(TOOLING_SRC))
LINT_OBJS := $(patsubst tools/%.cpp,$(TOOLS_OBJ_DIR)/%.o,$(LINT_SRC))
LINT_LIB_OBJS := $(patsubst tools/%.cpp,$(TOOLS_OBJ_DIR)/%.o,$(LINT_LIB_SRC))
LSP_OBJS := $(patsubst tools/%.cpp,$(TOOLS_OBJ_DIR)/%.o,$(LSP_SRC))
DEP_FILES := $(RELEASE_OBJS:.o=.d) $(DEBUG_OBJS:.o=.d) $(SANITIZE_OBJS:.o=.d) $(TOOLING_OBJS:.o=.d) $(LINT_OBJS:.o=.d) $(LSP_OBJS:.o=.d)

.PHONY: all release debug sanitize tools lint lsp install uninstall clean examples check-examples example run-example libraries build-lib check-lib

all: $(TARGET)
release: $(TARGET)
debug: $(DEBUG_TARGET)
sanitize: $(SANITIZE_TARGET)
tools: lint lsp
lint: $(LINT_TARGET)
lsp: $(LSP_TARGET)
libraries: build-lib

install: release
	$(INSTALL_DIR) $(DESTDIR)$(BINDIR)
	$(INSTALL_DIR) $(DESTDIR)$(ARI_SHAREDIR)/lib/std
	$(INSTALL_PROGRAM) $(TARGET) $(DESTDIR)$(BINDIR)/ari$(EXEEXT)
	$(INSTALL_DATA) lib/std.arih $(DESTDIR)$(ARI_SHAREDIR)/lib/std.arih
	$(INSTALL_DATA) $(STD_MODULE_SRCS) $(DESTDIR)$(ARI_SHAREDIR)/lib/std/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/ari$(EXEEXT)
	rm -rf $(DESTDIR)$(ARI_SHAREDIR)

EXAMPLE ?= count
EXAMPLE_SRCS := $(sort $(wildcard examples/*.ari))
EXAMPLE_BINS := $(patsubst examples/%.ari,$(BUILD_DIR)/examples/%$(EXEEXT),$(EXAMPLE_SRCS))
EXAMPLE_BIN := $(BUILD_DIR)/examples/$(EXAMPLE)$(EXEEXT)

$(TARGET): $(RELEASE_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(DEBUG_TARGET): $(DEBUG_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(DEBUG_CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(SANITIZE_TARGET): $(SANITIZE_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(SANITIZE_CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(LINT_TARGET): $(TOOLING_OBJS) $(LINT_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(LSP_TARGET): $(TOOLING_OBJS) $(LINT_LIB_OBJS) $(LSP_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(RELEASE_OBJ_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(DEBUG_OBJ_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(DEBUG_CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(SANITIZE_OBJ_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(SANITIZE_CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(TOOLS_OBJ_DIR)/%.o: tools/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

examples: $(EXAMPLE_BINS)

check-examples: $(TARGET)
	@set -e; for src in $(EXAMPLE_SRCS); do echo "checking $$src"; $(TARGET) $$src --check; done

example: $(EXAMPLE_BIN)

run-example: $(EXAMPLE_BIN)
	@set +e; $(EXAMPLE_BIN); code=$$?; echo "$(EXAMPLE) exited $$code"

$(BUILD_DIR)/examples/%$(EXEEXT): examples/%.ari $(TARGET)
	mkdir -p $(BUILD_DIR)/examples
	$(TARGET) $< -o $@

build-lib: $(TARGET)
	$(MAKE) -C lib ARI=$(TARGET) LLVM_CC=$(LLVM_CC) build

check-lib: $(TARGET)
	$(MAKE) -C lib ARI=$(TARGET) LLVM_CC=$(LLVM_CC) check

include tests/Makefile

-include $(DEP_FILES)

clean:
	rm -rf $(BUILD_DIR)
