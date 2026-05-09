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
SRC := $(wildcard src/*.cpp)
HEADERS := $(wildcard src/*.hpp)

.PHONY: all release debug sanitize clean sample

all: $(TARGET)
release: $(TARGET)
debug: $(DEBUG_TARGET)
sanitize: $(SANITIZE_TARGET)

$(TARGET): $(SRC) $(HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

$(DEBUG_TARGET): $(SRC) $(HEADERS)
	mkdir -p $(BUILD_DIR)/debug
	$(CXX) $(DEBUG_CXXFLAGS) $(SRC) -o $(DEBUG_TARGET)

$(SANITIZE_TARGET): $(SRC) $(HEADERS)
	mkdir -p $(BUILD_DIR)/sanitize
	$(CXX) $(SANITIZE_CXXFLAGS) $(SRC) -o $(SANITIZE_TARGET)

sample: $(TARGET)
	$(TARGET) examples/count.ari --emit-llvm $(BUILD_DIR)/count.ll

include tests/Makefile

clean:
	rm -rf $(BUILD_DIR)
