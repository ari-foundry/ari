#include "format_semantics.hpp"

#include "common.hpp"
#include "type_semantics.hpp"

#include <cctype>
#include <cstddef>
#include <string>
#include <utility>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

IrFormatSpec parse_format_placeholder(SourceLocation loc, const std::string& text) {
    if (text.empty()) return IrFormatSpec{};
    if (text == ":?") return IrFormatSpec{-1, true};
    if (text.size() < 2 || text[0] != ':' || text[1] != '.') {
        fail(loc, "format string only supports {}, {:?}, and {:.N} placeholders; escape literal { as {{");
    }
    int precision = 0;
    bool has_digit = false;
    for (std::size_t i = 2; i < text.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (!std::isdigit(c)) {
            fail(loc, "format string only supports {}, {:?}, and {:.N} placeholders; escape literal { as {{");
        }
        has_digit = true;
        precision = precision * 10 + static_cast<int>(text[i] - '0');
        if (precision > 64) fail(loc, "format precision must be at most 64");
    }
    if (!has_digit) fail(loc, "format precision placeholder expects digits after colon-dot");
    return IrFormatSpec{precision, false};
}

} // namespace

std::optional<FormatInAppendTarget> builtin_format_in_append_target_from_type(const IrType& type) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::String) {
        return FormatInAppendTarget{FormatInAppendKind::String, {}};
    }
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
        return FormatInAppendTarget{FormatInAppendKind::Bool, {}};
    }
    if (is_value_integer_type(type) && is_unsigned_integer_primitive(type.primitive)) {
        return FormatInAppendTarget{FormatInAppendKind::U64, {}};
    }
    if (is_value_integer_type(type)) {
        return FormatInAppendTarget{FormatInAppendKind::I64, {}};
    }
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::F32) {
        return FormatInAppendTarget{FormatInAppendKind::F32, {}};
    }
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::F64) {
        return FormatInAppendTarget{FormatInAppendKind::F64, {}};
    }
    return std::nullopt;
}

FormatInAppendTarget format_in_display_append_target(std::string trait_name) {
    return FormatInAppendTarget{FormatInAppendKind::Display, std::move(trait_name)};
}

FormatInAppendTarget format_in_debug_append_target(std::string trait_name) {
    return FormatInAppendTarget{FormatInAppendKind::Debug, std::move(trait_name)};
}

bool format_in_append_target_is_float(const FormatInAppendTarget& target) {
    return target.kind == FormatInAppendKind::F32 ||
           target.kind == FormatInAppendKind::F64;
}

bool format_in_append_target_is_display(const FormatInAppendTarget& target) {
    return target.kind == FormatInAppendKind::Display;
}

bool format_in_append_target_is_debug(const FormatInAppendTarget& target) {
    return target.kind == FormatInAppendKind::Debug;
}

const char* format_in_builtin_append_method_name(const FormatInAppendTarget& target) {
    switch (target.kind) {
        case FormatInAppendKind::String: return "append_string_in";
        case FormatInAppendKind::I64: return "append_i64_in";
        case FormatInAppendKind::U64: return "append_u64_in";
        case FormatInAppendKind::Bool: return "append_bool_in";
        case FormatInAppendKind::F32: return "append_f32_in";
        case FormatInAppendKind::F64: return "append_f64_in";
        case FormatInAppendKind::Display: break;
        case FormatInAppendKind::Debug: break;
    }
    return "append_i64_in";
}

std::string unsupported_format_in_value_message(const IrType& type) {
    return "format_in! currently supports string, integer, bool, f32, f64, and Display values, got " +
           type_name(type);
}

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
                fail(loc, "format string only supports {}, {:?}, and {:.N} placeholders; escape literal { as {{");
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
