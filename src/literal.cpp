#include "literal.hpp"

#include <limits>

namespace ari {

int digit_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    return -1;
}

bool is_digit_for_base(char c, int base) {
    int value = digit_value(c);
    return value >= 0 && value < base;
}

std::uint64_t parse_integer_digits(SourceLocation loc, const std::string& digits, int base, const std::string& literal_kind) {
    std::uint64_t value = 0;
    std::uint64_t limit = std::numeric_limits<std::uint64_t>::max();
    for (char c : digits) {
        int digit = digit_value(c);
        if (digit < 0 || digit >= base) {
            throw CompileError(where(loc) + ": invalid digit '" + std::string(1, c) + "' in " + literal_kind + " literal");
        }
        if (value > (limit - static_cast<std::uint64_t>(digit)) / static_cast<std::uint64_t>(base)) {
            throw CompileError(where(loc) + ": integer literal is too large");
        }
        value = value * static_cast<std::uint64_t>(base) + static_cast<std::uint64_t>(digit);
    }
    return value;
}

void append_utf8(std::string& out, SourceLocation loc, std::uint32_t codepoint) {
    if (codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff)) {
        throw CompileError(where(loc) + ": invalid Unicode scalar value in string escape");
    }

    if (codepoint <= 0x7f) {
        out.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7ff) {
        out.push_back(static_cast<char>(0xc0 | (codepoint >> 6)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    } else if (codepoint <= 0xffff) {
        out.push_back(static_cast<char>(0xe0 | (codepoint >> 12)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    } else {
        out.push_back(static_cast<char>(0xf0 | (codepoint >> 18)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
}

} // namespace ari
