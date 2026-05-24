#include "literal.hpp"

#include <limits>
#include <utility>

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

[[noreturn]] static void fail_literal(SourceLocation loc,
                                      const std::string& message,
                                      const std::string& note,
                                      const std::string& help) {
    CompileError error(std::move(loc), message);
    error.add_note(DiagnosticNote{std::nullopt, note, DiagnosticNoteKind::Note});
    error.add_note(DiagnosticNote{std::nullopt, help, DiagnosticNoteKind::Help});
    throw error;
}

std::uint64_t parse_integer_digits(SourceLocation loc, const std::string& digits, int base, const std::string& literal_kind) {
    std::uint64_t value = 0;
    std::uint64_t limit = std::numeric_limits<std::uint64_t>::max();
    for (char c : digits) {
        int digit = digit_value(c);
        if (digit < 0 || digit >= base) {
            fail_literal(
                loc,
                "invalid digit '" + std::string(1, c) + "' in " + literal_kind + " literal",
                "literal digits must match the base or escape format being parsed",
                "remove the digit or use a literal form that supports it");
        }
        if (value > (limit - static_cast<std::uint64_t>(digit)) / static_cast<std::uint64_t>(base)) {
            fail_literal(
                loc,
                "integer literal is too large",
                "integer literals must fit in u64 before later type checking narrows them",
                "use a smaller literal or split the value into smaller operations");
        }
        value = value * static_cast<std::uint64_t>(base) + static_cast<std::uint64_t>(digit);
    }
    return value;
}

void append_utf8(std::string& out, SourceLocation loc, std::uint32_t codepoint) {
    if (codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff)) {
        fail_literal(
            loc,
            "invalid Unicode scalar value in string escape",
            "Unicode string escapes must name valid Unicode scalar values",
            "choose a code point in U+0000..U+10FFFF outside the surrogate range");
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
