#pragma once

#include "ir.hpp"
#include "token.hpp"

#include <cstdint>

namespace ari {

struct StaticIntegerValue {
    std::uint64_t value = 0;
    bool negative = false;
};

bool static_integer_value_to_i64(const StaticIntegerValue& value, std::int64_t& out);
StaticIntegerValue static_integer_value_from_i64(std::int64_t value);
bool fold_static_integer_unary(TokenKind op,
                               const StaticIntegerValue& operand,
                               StaticIntegerValue& out);
bool fold_static_integer_binary(TokenKind op,
                                const StaticIntegerValue& left,
                                const StaticIntegerValue& right,
                                StaticIntegerValue& out);
bool try_fold_static_integer_value(const IrExpr& expr, StaticIntegerValue& out);

} // namespace ari
