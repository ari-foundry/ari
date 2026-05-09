#pragma once

#include "common.hpp"

#include <cstdint>
#include <string>

namespace ari {

int digit_value(char c);
bool is_digit_for_base(char c, int base);
std::uint64_t parse_integer_digits(SourceLocation loc, const std::string& digits, int base, const std::string& literal_kind);
void append_utf8(std::string& out, SourceLocation loc, std::uint32_t codepoint);

} // namespace ari
