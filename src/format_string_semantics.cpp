#include "format_string_semantics.hpp"

#include "common.hpp"

#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

IrFormatSpec parse_format_placeholder(SourceLocation loc, const std::string& text) {
    if (text.empty()) return IrFormatSpec{};
    if (text.size() < 2 || text[0] != ':' || text[1] != '.') {
        fail(loc, "format string only supports {} and {:.N} placeholders; escape literal { as {{");
    }
    int precision = 0;
    bool has_digit = false;
    for (std::size_t i = 2; i < text.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (!std::isdigit(c)) {
            fail(loc, "format string only supports {} and {:.N} placeholders; escape literal { as {{");
        }
        has_digit = true;
        precision = precision * 10 + static_cast<int>(text[i] - '0');
        if (precision > 64) fail(loc, "format precision must be at most 64");
    }
    if (!has_digit) fail(loc, "format precision placeholder expects digits after colon-dot");
    return IrFormatSpec{precision};
}

} // namespace

ParsedFormatString parse_format_string(SourceLocation loc, const std::string& text, std::size_t arg_count) {
    ParsedFormatString parsed;
    std::string current;
    std::size_t placeholders = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (c == '{') {
            if (i + 1 < text.size() && text[i + 1] == '{') {
                current.push_back('{');
                ++i;
                continue;
            }
            std::size_t close = text.find('}', i + 1);
            if (close == std::string::npos) {
                fail(loc, "format string only supports {} and {:.N} placeholders; escape literal { as {{");
            }
            parsed.parts.push_back(current);
            current.clear();
            parsed.specs.push_back(parse_format_placeholder(loc, text.substr(i + 1, close - i - 1)));
            ++placeholders;
            i = close;
            continue;
        }
        if (c == '}') {
            if (i + 1 < text.size() && text[i + 1] == '}') {
                current.push_back('}');
                ++i;
                continue;
            }
            fail(loc, "unmatched } in format string; escape literal } as }}");
        }
        current.push_back(c);
    }
    parsed.parts.push_back(current);
    if (placeholders != arg_count) {
        fail(loc, "format string has " + std::to_string(placeholders) +
                  " placeholders but " + std::to_string(arg_count) + " values were provided");
    }
    return parsed;
}

} // namespace ari
