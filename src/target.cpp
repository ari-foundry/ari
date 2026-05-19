#include "target.hpp"

#include "common.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace ari {
namespace {

std::string lowercase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return text;
}

bool contains(const std::string& text, const std::string& needle) {
    return text.find(needle) != std::string::npos;
}

bool has_arch(const std::string& triple, const std::string& arch) {
    return triple == arch || triple.rfind(arch + "-", 0) == 0;
}

void set_arch_layout(TargetInfo& info) {
    if (has_arch(info.triple, "x86_64") || has_arch(info.triple, "amd64") ||
        has_arch(info.triple, "aarch64") || has_arch(info.triple, "arm64") ||
        has_arch(info.triple, "riscv64")) {
        info.pointer_bits = 64;
        if (has_arch(info.triple, "amd64")) info.arch = "x86_64";
        else if (has_arch(info.triple, "arm64")) info.arch = "aarch64";
        else info.arch = info.triple.substr(0, info.triple.find('-'));
        return;
    }

    if (has_arch(info.triple, "i386") || has_arch(info.triple, "i486") ||
        has_arch(info.triple, "i586") || has_arch(info.triple, "i686") ||
        has_arch(info.triple, "x86")) {
        info.pointer_bits = 32;
        info.arch = "x86";
        return;
    }

    if (has_arch(info.triple, "arm") || has_arch(info.triple, "armv7")) {
        info.pointer_bits = 32;
        info.arch = "arm";
        return;
    }

    throw CompileError("unsupported target triple '" + info.triple + "'");
}

void set_os_layout(TargetInfo& info) {
    info.linux = contains(info.triple, "linux");
    info.macos = contains(info.triple, "darwin") ||
                 contains(info.triple, "apple") ||
                 contains(info.triple, "macos");
    info.windows = contains(info.triple, "windows") ||
                   contains(info.triple, "win32") ||
                   contains(info.triple, "msvc") ||
                   contains(info.triple, "mingw");
    info.unix = info.linux || info.macos;

    if (!info.linux && !info.macos && !info.windows) {
        throw CompileError("unsupported target triple '" + info.triple + "'");
    }

    info.long_bits = info.windows ? 32 : info.pointer_bits;
    info.plain_char_signed = true;
}

} // namespace

std::string default_target_triple() {
#if defined(_WIN32)
#if defined(_M_X64)
    return "x86_64-pc-windows-msvc";
#elif defined(_M_ARM64)
    return "aarch64-pc-windows-msvc";
#else
    return "i686-pc-windows-msvc";
#endif
#elif defined(__APPLE__)
#if defined(__aarch64__)
    return "aarch64-apple-darwin";
#else
    return "x86_64-apple-darwin";
#endif
#elif defined(__linux__)
#if defined(__x86_64__)
    return "x86_64-pc-linux-gnu";
#elif defined(__aarch64__)
    return "aarch64-unknown-linux-gnu";
#elif defined(__i386__)
    return "i686-unknown-linux-gnu";
#else
    return "x86_64-pc-linux-gnu";
#endif
#else
    return "x86_64-pc-linux-gnu";
#endif
}

TargetInfo resolve_target_info(const std::string& triple) {
    TargetInfo info;
    info.triple = lowercase(triple.empty() ? default_target_triple() : triple);
    set_arch_layout(info);
    set_os_layout(info);
    return info;
}

bool target_predicate_active(const TargetInfo& target, const std::string& name) {
    if (name == target.arch) return true;
    if (name == "linux") return target.linux;
    if (name == "macos" || name == "darwin") return target.macos;
    if (name == "windows") return target.windows;
    if (name == "unix") return target.unix;
    if (name == "gnu" || name == "glibc") return target.linux && contains(target.triple, "gnu");
    if (name == "musl") return target.linux && contains(target.triple, "musl");
    if (name == "elf") return target.linux;
    if (name == "dwarf") return target.unix;
    return false;
}

} // namespace ari
