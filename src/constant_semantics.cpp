#include "constant_semantics.hpp"

#include <limits>

namespace ari {
namespace {

bool is_integer_type(const IrType& type) {
    if (type.qualifier != TypeQualifier::Value) return false;
    switch (type.primitive) {
        case IrPrimitiveKind::I8:
        case IrPrimitiveKind::I16:
        case IrPrimitiveKind::I32:
        case IrPrimitiveKind::I64:
        case IrPrimitiveKind::U8:
        case IrPrimitiveKind::U16:
        case IrPrimitiveKind::U32:
        case IrPrimitiveKind::U64:
            return true;
        default:
            return false;
    }
}

bool is_signed_integer_type(const IrType& type) {
    if (type.qualifier != TypeQualifier::Value) return false;
    switch (type.primitive) {
        case IrPrimitiveKind::I8:
        case IrPrimitiveKind::I16:
        case IrPrimitiveKind::I32:
        case IrPrimitiveKind::I64:
            return true;
        default:
            return false;
    }
}

} // namespace

bool static_integer_value_to_i64(const StaticIntegerValue& value, std::int64_t& out) {
    if (value.negative) {
        const std::uint64_t min_magnitude =
            static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 1ULL;
        if (value.value == min_magnitude) {
            out = std::numeric_limits<std::int64_t>::min();
            return true;
        }
        if (value.value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            return false;
        }
        out = -static_cast<std::int64_t>(value.value);
        return true;
    }
    if (value.value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        return false;
    }
    out = static_cast<std::int64_t>(value.value);
    return true;
}

StaticIntegerValue static_integer_value_from_i64(std::int64_t value) {
    StaticIntegerValue out;
    out.negative = value < 0;
    out.value = value == std::numeric_limits<std::int64_t>::min()
        ? static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 1ULL
        : static_cast<std::uint64_t>(out.negative ? -value : value);
    return out;
}

bool fold_static_integer_unary(TokenKind op,
                               const StaticIntegerValue& operand,
                               StaticIntegerValue& out) {
    if (op != TokenKind::Minus) return false;
    std::int64_t value = 0;
    if (!static_integer_value_to_i64(operand, value) ||
        value == std::numeric_limits<std::int64_t>::min()) {
        return false;
    }
    out = static_integer_value_from_i64(-value);
    return true;
}

bool fold_static_integer_binary(TokenKind op,
                                const StaticIntegerValue& left,
                                const StaticIntegerValue& right,
                                StaticIntegerValue& out) {
    switch (op) {
        case TokenKind::Plus:
        case TokenKind::Minus:
        case TokenKind::Star:
        case TokenKind::Slash:
        case TokenKind::Percent:
            break;
        default:
            return false;
    }

    std::int64_t lhs = 0;
    std::int64_t rhs = 0;
    if (!static_integer_value_to_i64(left, lhs) || !static_integer_value_to_i64(right, rhs)) {
        return false;
    }

    std::int64_t result = 0;
    bool overflow = false;
    switch (op) {
        case TokenKind::Plus:
            overflow = __builtin_add_overflow(lhs, rhs, &result);
            break;
        case TokenKind::Minus:
            overflow = __builtin_sub_overflow(lhs, rhs, &result);
            break;
        case TokenKind::Star:
            overflow = __builtin_mul_overflow(lhs, rhs, &result);
            break;
        case TokenKind::Slash:
            if (rhs == 0 ||
                (lhs == std::numeric_limits<std::int64_t>::min() && rhs == -1)) {
                return false;
            }
            result = lhs / rhs;
            break;
        case TokenKind::Percent:
            if (rhs == 0 ||
                (lhs == std::numeric_limits<std::int64_t>::min() && rhs == -1)) {
                return false;
            }
            result = lhs % rhs;
            break;
        default:
            return false;
    }
    if (overflow) return false;
    out = static_integer_value_from_i64(result);
    return true;
}

bool try_fold_static_integer_value(const IrExpr& expr, StaticIntegerValue& out) {
    if (!is_integer_type(expr.type)) return false;
    if (expr.kind == IrExprKind::Integer) {
        out.value = expr.int_value;
        out.negative = expr.int_negative;
        return true;
    }
    if (expr.kind != IrExprKind::Binary) return false;
    if (!is_signed_integer_type(expr.type)) return false;

    switch (expr.op) {
        case IrBinaryOp::Add:
        case IrBinaryOp::Sub:
        case IrBinaryOp::Mul:
        case IrBinaryOp::Div:
        case IrBinaryOp::Mod:
            break;
        default:
            return false;
    }

    StaticIntegerValue left;
    StaticIntegerValue right;
    if (!expr.left || !expr.right ||
        !try_fold_static_integer_value(*expr.left, left) ||
        !try_fold_static_integer_value(*expr.right, right)) {
        return false;
    }

    TokenKind source_op = TokenKind::End;
    switch (expr.op) {
        case IrBinaryOp::Add:
            source_op = TokenKind::Plus;
            break;
        case IrBinaryOp::Sub:
            source_op = TokenKind::Minus;
            break;
        case IrBinaryOp::Mul:
            source_op = TokenKind::Star;
            break;
        case IrBinaryOp::Div:
            source_op = TokenKind::Slash;
            break;
        case IrBinaryOp::Mod:
            source_op = TokenKind::Percent;
            break;
        default:
            return false;
    }
    return fold_static_integer_binary(source_op, left, right, out);
}

} // namespace ari
