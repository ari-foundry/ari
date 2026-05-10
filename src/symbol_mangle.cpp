#include "symbol_mangle.hpp"

#include <cctype>
#include <cstddef>
#include <string>
#include <vector>

namespace ari {

namespace {

std::vector<std::string> split_path(const std::string& name) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    for (;;) {
        std::size_t split = name.find("::", start);
        if (split == std::string::npos) {
            parts.push_back(name.substr(start));
            break;
        }
        parts.push_back(name.substr(start, split - start));
        start = split + 2;
    }
    return parts;
}

char hex_digit(unsigned value) {
    static const char* digits = "0123456789abcdef";
    return digits[value & 0xf];
}

std::string encode_segment(const std::string& text) {
    std::string out;
    for (unsigned char c : text) {
        if (std::isalnum(c) || c == '_') {
            out.push_back(static_cast<char>(c));
        } else {
            out += "_u";
            out.push_back(hex_digit(c >> 4));
            out.push_back(hex_digit(c));
        }
    }
    if (out.empty()) return "_";
    return out;
}

} // namespace

std::string mangle_function_name(const std::string& name) {
    // Rust v0-style shape: fixed prefix plus length-prefixed path segments.
    // Ari has no argument-based function overloading, so parameter names/types
    // and return types are intentionally excluded from public symbol identity.
    std::string out = "_ARNv";
    for (const auto& part : split_path(name)) {
        std::string encoded = encode_segment(part);
        out += std::to_string(encoded.size());
        out += encoded;
    }
    return out;
}

} // namespace ari
