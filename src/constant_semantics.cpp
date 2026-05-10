#include "constant_semantics.hpp"

#include "common.hpp"
#include "type_semantics.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

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

std::uint64_t constant_integer_payload_bits(const ConstantValue& value, const IrType& payload_type) {
    std::uint64_t raw = value.int_negative ? 0 - value.int_value : value.int_value;
    unsigned width = integer_primitive_bit_width(payload_type.primitive);
    if (width == 64) return raw;
    std::uint64_t mask = (1ULL << width) - 1ULL;
    raw &= mask;
    if (is_signed_integer_primitive(payload_type.primitive) && (raw & (1ULL << (width - 1)))) {
        raw |= ~mask;
    }
    return raw;
}

void add_payload_literal_condition(IrMatchArm& lowered_arm,
                                   std::uint32_t payload_index,
                                   std::uint64_t payload_bits) {
    lowered_arm.payload_literal_conditions.push_back(IrPayloadLiteralCondition{
        payload_index,
        payload_bits,
        false,
        false
    });
}

} // namespace

static std::uint64_t int64_min_magnitude() {
    return static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 1ULL;
}

ConstantEvaluationStackGuard::ConstantEvaluationStackGuard(std::vector<std::string>& stack, std::string name)
    : stack_(stack) {
    stack_.push_back(std::move(name));
}

ConstantEvaluationStackGuard::~ConstantEvaluationStackGuard() {
    stack_.pop_back();
}

std::string format_constant_cycle_path(const std::vector<std::string>& stack,
                                       const std::string& repeated_name) {
    auto first = std::find(stack.begin(), stack.end(), repeated_name);
    std::string path;
    if (first == stack.end()) first = stack.begin();
    for (auto it = first; it != stack.end(); ++it) {
        if (!path.empty()) path += " -> ";
        path += *it;
    }
    if (!path.empty()) path += " -> ";
    path += repeated_name;
    return path;
}

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

std::int64_t constant_integer_to_i64(SourceLocation loc, const ConstantValue& value) {
    if (value.int_negative) {
        if (value.int_value == int64_min_magnitude()) return std::numeric_limits<std::int64_t>::min();
        if (value.int_value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            fail(loc, "constant integer expression operand is out of range for i64");
        }
        return -static_cast<std::int64_t>(value.int_value);
    }
    if (value.int_value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        fail(loc, "constant integer expression operand is out of range for i64");
    }
    return static_cast<std::int64_t>(value.int_value);
}

std::uint64_t constant_integer_to_u64(SourceLocation loc, const ConstantValue& value) {
    if (value.int_negative) fail(loc, "unsigned constant arithmetic cannot use negative operands");
    return value.int_value;
}

std::uint64_t integer_bit_mask(unsigned width) {
    if (width >= 64) return std::numeric_limits<std::uint64_t>::max();
    return (1ULL << width) - 1ULL;
}

std::uint64_t constant_integer_raw_bits(const ConstantValue& value, unsigned width) {
    std::uint64_t raw = value.int_negative ? 0 - value.int_value : value.int_value;
    return raw & integer_bit_mask(width);
}

std::int64_t sign_extend_integer_bits(std::uint64_t raw, unsigned width) {
    raw &= integer_bit_mask(width);
    if (width == 0) return 0;
    if (width < 64 && (raw & (1ULL << (width - 1)))) {
        raw |= ~integer_bit_mask(width);
    }
    return static_cast<std::int64_t>(raw);
}

unsigned constant_shift_amount(SourceLocation loc, const ConstantValue& value, unsigned width) {
    if (value.int_negative) fail(loc, "constant shift amount must be non-negative");
    if (width == 0 || value.int_value >= width) {
        fail(loc, "constant shift amount must be less than " + std::to_string(width));
    }
    return static_cast<unsigned>(value.int_value);
}

ConstantValue make_signed_integer_constant(SourceLocation loc, const IrType& type, std::int64_t result) {
    ConstantValue value;
    value.kind = ConstantValueKind::Integer;
    value.type = type;
    value.int_negative = result < 0;
    value.int_value = result == std::numeric_limits<std::int64_t>::min()
        ? int64_min_magnitude()
        : static_cast<std::uint64_t>(value.int_negative ? -result : result);

    IrExpr literal;
    literal.kind = IrExprKind::Integer;
    literal.loc = loc;
    literal.type = type;
    literal.int_value = value.int_value;
    literal.int_negative = value.int_negative;
    if (!integer_literal_fits(literal, type)) {
        fail(loc, "constant integer expression result is out of range for " + type_name(type));
    }
    return value;
}

ConstantValue make_unsigned_integer_constant(SourceLocation loc, const IrType& type, std::uint64_t result) {
    ConstantValue value;
    value.kind = ConstantValueKind::Integer;
    value.type = type;
    value.int_value = result;
    value.int_negative = false;

    IrExpr literal;
    literal.kind = IrExprKind::Integer;
    literal.loc = loc;
    literal.type = type;
    literal.int_value = value.int_value;
    literal.int_negative = false;
    if (!integer_literal_fits(literal, type)) {
        fail(loc, "constant integer expression result is out of range for " + type_name(type));
    }
    return value;
}

ConstantValue make_integer_literal_constant(SourceLocation loc,
                                            const IrType& expected,
                                            const std::string& literal_suffix,
                                            std::uint64_t int_value,
                                            bool int_negative) {
    if (!is_integer_type(expected)) {
        fail(loc, "integer constant literal requires an integer result type");
    }

    IrType literal_type = literal_suffix.empty()
        ? expected
        : integer_literal_suffix_type(literal_suffix, loc);
    IrExpr literal;
    literal.kind = IrExprKind::Integer;
    literal.loc = loc;
    literal.type = literal_type;
    literal.int_value = int_value;
    literal.int_negative = int_negative;
    if (!integer_literal_fits(literal, literal_type)) {
        fail(loc, "integer literal " + integer_literal_name(literal) +
                  " is out of range for " + type_name(literal_type));
    }
    require_assignable(loc, expected, literal_type);

    ConstantValue value;
    value.kind = ConstantValueKind::Integer;
    value.type = expected;
    value.int_value = int_value;
    value.int_negative = int_negative;
    return value;
}

ConstantValue make_bool_constant(SourceLocation loc, const IrType& expected, bool result) {
    if (expected.qualifier != TypeQualifier::Value || expected.primitive != IrPrimitiveKind::Bool) {
        fail(loc, "type mismatch: expected " + type_name(expected) + ", got bool");
    }
    ConstantValue value;
    value.kind = ConstantValueKind::Bool;
    value.type = expected;
    value.is_bool = true;
    value.bool_value = result;
    return value;
}

ConstantValue make_bool_literal_constant(SourceLocation loc, const IrType& expected, bool result) {
    return make_bool_constant(loc, expected, result);
}

ConstantValue evaluate_constant_bool_binary(SourceLocation loc,
                                            TokenKind op,
                                            const IrType& expected,
                                            const ConstantValue& left,
                                            const ConstantValue& right) {
    switch (op) {
        case TokenKind::AmpAmp:
            return make_bool_constant(loc, expected, left.bool_value && right.bool_value);
        case TokenKind::PipePipe:
            return make_bool_constant(loc, expected, left.bool_value || right.bool_value);
        default:
            fail(loc, "constant bool expressions support logical and comparison operators");
    }
}

ConstantValue evaluate_constant_bool_comparison(SourceLocation loc,
                                                TokenKind op,
                                                const IrType& expected,
                                                const ConstantValue& left,
                                                const ConstantValue& right) {
    const bool equal = left.bool_value == right.bool_value;
    switch (op) {
        case TokenKind::EqEq:
            return make_bool_constant(loc, expected, equal);
        case TokenKind::BangEq:
            return make_bool_constant(loc, expected, !equal);
        default:
            fail(loc, "constant bool expressions support logical and comparison operators");
    }
}

ConstantValue evaluate_constant_integer_comparison(SourceLocation loc,
                                                   TokenKind op,
                                                   const IrType& expected,
                                                   const IrType& operand_type,
                                                   const ConstantValue& left,
                                                   const ConstantValue& right) {
    bool result = false;
    if (is_signed_integer_type(operand_type)) {
        std::int64_t lhs = constant_integer_to_i64(loc, left);
        std::int64_t rhs = constant_integer_to_i64(loc, right);
        switch (op) {
            case TokenKind::EqEq: result = lhs == rhs; break;
            case TokenKind::BangEq: result = lhs != rhs; break;
            case TokenKind::Less: result = lhs < rhs; break;
            case TokenKind::LessEq: result = lhs <= rhs; break;
            case TokenKind::Greater: result = lhs > rhs; break;
            case TokenKind::GreaterEq: result = lhs >= rhs; break;
            default:
                fail(loc, "constant bool expressions support logical and comparison operators");
        }
    } else if (is_integer_type(operand_type)) {
        std::uint64_t lhs = constant_integer_to_u64(loc, left);
        std::uint64_t rhs = constant_integer_to_u64(loc, right);
        switch (op) {
            case TokenKind::EqEq: result = lhs == rhs; break;
            case TokenKind::BangEq: result = lhs != rhs; break;
            case TokenKind::Less: result = lhs < rhs; break;
            case TokenKind::LessEq: result = lhs <= rhs; break;
            case TokenKind::Greater: result = lhs > rhs; break;
            case TokenKind::GreaterEq: result = lhs >= rhs; break;
            default:
                fail(loc, "constant bool expressions support logical and comparison operators");
        }
    } else {
        fail(loc, "constant comparison operands must be integers, got " + type_name(operand_type));
    }
    return make_bool_constant(loc, expected, result);
}

ConstantValue evaluate_constant_integer_binary(SourceLocation loc,
                                               TokenKind op,
                                               const IrType& expected,
                                               const ConstantValue& left,
                                               const ConstantValue& right) {
    if (!is_integer_type(expected)) {
        fail(loc, "constant arithmetic expressions require an integer result type");
    }
    switch (op) {
        case TokenKind::Plus:
        case TokenKind::Minus:
        case TokenKind::Star:
        case TokenKind::Slash:
        case TokenKind::Percent:
        case TokenKind::Amp:
        case TokenKind::Pipe:
        case TokenKind::Caret:
        case TokenKind::LessLess:
        case TokenKind::GreaterGreater:
            break;
        default:
            fail(loc, "constant integer expressions support +, -, *, /, %, &, |, ^, <<, and >>");
    }

    const unsigned width = integer_primitive_bit_width(expected.primitive);
    if (is_signed_integer_type(expected)) {
        std::int64_t lhs = constant_integer_to_i64(loc, left);
        std::int64_t rhs = constant_integer_to_i64(loc, right);
        if (op == TokenKind::Amp ||
            op == TokenKind::Pipe ||
            op == TokenKind::Caret ||
            op == TokenKind::LessLess ||
            op == TokenKind::GreaterGreater) {
            std::uint64_t lhs_bits = constant_integer_raw_bits(left, width);
            std::uint64_t result_bits = 0;
            switch (op) {
                case TokenKind::Amp:
                    result_bits = lhs_bits & constant_integer_raw_bits(right, width);
                    break;
                case TokenKind::Pipe:
                    result_bits = lhs_bits | constant_integer_raw_bits(right, width);
                    break;
                case TokenKind::Caret:
                    result_bits = lhs_bits ^ constant_integer_raw_bits(right, width);
                    break;
                case TokenKind::LessLess: {
                    unsigned shift = constant_shift_amount(loc, right, width);
                    result_bits = (lhs_bits << shift) & integer_bit_mask(width);
                    break;
                }
                case TokenKind::GreaterGreater: {
                    unsigned shift = constant_shift_amount(loc, right, width);
                    result_bits = static_cast<std::uint64_t>(lhs >> shift) & integer_bit_mask(width);
                    break;
                }
                default:
                    break;
            }
            return make_signed_integer_constant(loc, expected, sign_extend_integer_bits(result_bits, width));
        }
        if ((op == TokenKind::Slash || op == TokenKind::Percent) && rhs == 0) {
            fail(loc, "constant expression divides by zero");
        }
        if ((op == TokenKind::Slash || op == TokenKind::Percent) &&
            lhs == std::numeric_limits<std::int64_t>::min() && rhs == -1) {
            fail(loc, "constant integer expression result is out of range for " + type_name(expected));
        }

        std::int64_t result = 0;
        bool overflow = false;
        switch (op) {
            case TokenKind::Plus: overflow = __builtin_add_overflow(lhs, rhs, &result); break;
            case TokenKind::Minus: overflow = __builtin_sub_overflow(lhs, rhs, &result); break;
            case TokenKind::Star: overflow = __builtin_mul_overflow(lhs, rhs, &result); break;
            case TokenKind::Slash: result = lhs / rhs; break;
            case TokenKind::Percent: result = lhs % rhs; break;
            default: break;
        }
        if (overflow) {
            fail(loc, "constant integer expression result is out of range for " + type_name(expected));
        }
        return make_signed_integer_constant(loc, expected, result);
    }

    std::uint64_t lhs = constant_integer_to_u64(loc, left);
    std::uint64_t rhs = constant_integer_to_u64(loc, right);
    if (op == TokenKind::Amp ||
        op == TokenKind::Pipe ||
        op == TokenKind::Caret ||
        op == TokenKind::LessLess ||
        op == TokenKind::GreaterGreater) {
        std::uint64_t result = 0;
        switch (op) {
            case TokenKind::Amp:
                result = lhs & rhs;
                break;
            case TokenKind::Pipe:
                result = lhs | rhs;
                break;
            case TokenKind::Caret:
                result = lhs ^ rhs;
                break;
            case TokenKind::LessLess: {
                unsigned shift = constant_shift_amount(loc, right, width);
                result = (lhs << shift) & integer_bit_mask(width);
                break;
            }
            case TokenKind::GreaterGreater: {
                unsigned shift = constant_shift_amount(loc, right, width);
                result = lhs >> shift;
                break;
            }
            default:
                break;
        }
        return make_unsigned_integer_constant(loc, expected, result);
    }
    if ((op == TokenKind::Slash || op == TokenKind::Percent) && rhs == 0) {
        fail(loc, "constant expression divides by zero");
    }
    std::uint64_t result = 0;
    bool overflow = false;
    switch (op) {
        case TokenKind::Plus: overflow = __builtin_add_overflow(lhs, rhs, &result); break;
        case TokenKind::Minus: overflow = __builtin_sub_overflow(lhs, rhs, &result); break;
        case TokenKind::Star: overflow = __builtin_mul_overflow(lhs, rhs, &result); break;
        case TokenKind::Slash: result = lhs / rhs; break;
        case TokenKind::Percent: result = lhs % rhs; break;
        default: break;
    }
    if (overflow) {
        fail(loc, "constant integer expression result is out of range for " + type_name(expected));
    }
    return make_unsigned_integer_constant(loc, expected, result);
}

IrExprPtr make_constant_expr(SourceLocation loc, const ConstantValue& value) {
    auto expr = std::make_unique<IrExpr>();
    expr->loc = loc;
    expr->type = value.type;

    switch (value.kind) {
        case ConstantValueKind::Bool:
            expr->kind = IrExprKind::Bool;
            expr->bool_value = value.bool_value;
            return expr;
        case ConstantValueKind::Tuple:
        case ConstantValueKind::Struct:
            expr->kind = IrExprKind::Tuple;
            expr->args.reserve(value.elements.size());
            for (const auto& item : value.elements) {
                expr->args.push_back(make_constant_expr(loc, item));
            }
            return expr;
        case ConstantValueKind::Array:
            expr->kind = IrExprKind::Vector;
            expr->args.reserve(value.elements.size());
            for (const auto& item : value.elements) {
                expr->args.push_back(make_constant_expr(loc, item));
            }
            return expr;
        case ConstantValueKind::Enum:
            expr->kind = IrExprKind::EnumConstruct;
            expr->enum_name = value.enum_name;
            expr->case_name = value.case_name;
            expr->enum_tag = value.enum_tag;
            expr->has_payload = !value.elements.empty();
            if (has_aggregate_enum_layout(value.type)) {
                expr->args.reserve(value.elements.size());
                for (const auto& item : value.elements) {
                    expr->args.push_back(make_constant_expr(loc, item));
                }
            } else if (!value.elements.empty()) {
                expr->payload_type = value.elements[0].type;
                expr->payload = make_constant_expr(loc, value.elements[0]);
            }
            return expr;
        case ConstantValueKind::Integer:
            break;
    }
    expr->kind = IrExprKind::Integer;
    expr->int_value = value.int_value;
    expr->int_negative = value.int_negative;
    return expr;
}

IrMatchArm make_scalar_constant_match_arm(SourceLocation loc,
                                          const ConstantValue& value,
                                          const IrType& match_type) {
    IrMatchArm lowered_arm;
    lowered_arm.loc = loc;

    if (match_type.qualifier == TypeQualifier::Value && match_type.primitive == IrPrimitiveKind::Bool) {
        if (!value.is_bool) {
            fail(loc, "bool match constant pattern must have type bool");
        }
        lowered_arm.has_literal = true;
        lowered_arm.literal_is_bool = true;
        lowered_arm.literal_bool = value.bool_value;
        return lowered_arm;
    }

    if (!is_value_integer_type(match_type)) {
        fail(loc, "constant match patterns require integer or bool match values");
    }
    if (value.is_bool || !is_value_integer_type(value.type)) {
        fail(loc, "integer match constant pattern must have an integer type");
    }
    require_assignable(loc, match_type, value.type);

    lowered_arm.has_literal = true;
    lowered_arm.literal_int = value.int_value;
    lowered_arm.literal_negative = value.int_negative;
    return lowered_arm;
}

void set_nested_enum_payload_constant_literal(SourceLocation loc,
                                              const ConstantValue& value,
                                              const IrType& nested_payload_type,
                                              IrPayloadEnumCondition& condition) {
    if (nested_payload_type.qualifier == TypeQualifier::Value &&
        nested_payload_type.primitive == IrPrimitiveKind::Bool) {
        if (!value.is_bool) fail(loc, "bool nested enum payload constant pattern must have type bool");
        condition.has_payload_literal = true;
        condition.payload_literal_int = value.bool_value ? 1ULL : 0ULL;
        condition.payload_literal_negative = false;
        condition.payload_literal_is_bool = true;
        condition.payload_literal_bool = value.bool_value;
        condition.payload_type = nested_payload_type;
        return;
    }

    if (!is_value_integer_type(nested_payload_type)) {
        fail(loc, "constant nested enum payload patterns require integer or bool payloads");
    }
    if (value.is_bool || !is_value_integer_type(value.type)) {
        fail(loc, "integer nested enum payload constant pattern must have an integer type");
    }
    require_assignable(loc, nested_payload_type, value.type);
    condition.has_payload_literal = true;
    condition.payload_literal_int = value.int_value;
    condition.payload_literal_negative = value.int_negative;
    condition.payload_literal_is_bool = false;
    condition.payload_type = nested_payload_type;
}

void lower_aggregate_enum_payload_constant_pattern(SourceLocation loc,
                                                   const ConstantValue& value,
                                                   const IrType& payload_type,
                                                   IrMatchArm& lowered_arm,
                                                   std::uint32_t payload_index) {
    if (payload_type.qualifier == TypeQualifier::Value &&
        payload_type.primitive == IrPrimitiveKind::Bool) {
        if (!value.is_bool) fail(loc, "bool payload constant pattern must have type bool");
        add_payload_literal_condition(lowered_arm, payload_index, value.bool_value ? 1ULL : 0ULL);
        lowered_arm.payload_literal_conditions.back().is_bool = true;
        lowered_arm.payload_literal_conditions.back().bool_value = value.bool_value;
        return;
    }

    if (!is_value_integer_type(payload_type)) {
        fail(loc, "constant enum payload patterns require integer or bool payloads");
    }
    if (value.is_bool || !is_value_integer_type(value.type)) {
        fail(loc, "integer payload constant pattern must have an integer type");
    }
    require_assignable(loc, payload_type, value.type);
    add_payload_literal_condition(lowered_arm, payload_index, constant_integer_payload_bits(value, payload_type));
}

void lower_compact_enum_payload_constant_pattern(SourceLocation loc,
                                                 const ConstantValue& value,
                                                 const IrType& payload_type,
                                                 std::uint32_t enum_tag,
                                                 IrMatchArm& lowered_arm) {
    if (payload_type.qualifier == TypeQualifier::Value &&
        payload_type.primitive == IrPrimitiveKind::Bool) {
        if (!value.is_bool) fail(loc, "bool payload constant pattern must have type bool");
        lowered_arm.has_literal = true;
        lowered_arm.literal_int = ((value.bool_value ? 1ULL : 0ULL) << 32) | enum_tag;
        lowered_arm.literal_negative = false;
        return;
    }

    if (!is_value_integer_type(payload_type)) {
        fail(loc, "constant enum payload patterns require integer or bool payloads");
    }
    if (value.is_bool || !is_value_integer_type(value.type)) {
        fail(loc, "integer payload constant pattern must have an integer type");
    }
    require_assignable(loc, payload_type, value.type);

    std::uint64_t payload_bits = constant_integer_payload_bits(value, payload_type);
    lowered_arm.has_literal = true;
    lowered_arm.literal_int = (payload_bits << 32) | enum_tag;
    lowered_arm.literal_negative = false;
}

bool fold_static_integer_unary(TokenKind op,
                               const StaticIntegerValue& operand,
                               StaticIntegerValue& out) {
    std::int64_t value = 0;
    if (!static_integer_value_to_i64(operand, value)) return false;
    switch (op) {
        case TokenKind::Minus:
            if (value == std::numeric_limits<std::int64_t>::min()) return false;
            out = static_integer_value_from_i64(-value);
            return true;
        default:
            return false;
    }
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
        case TokenKind::Amp:
        case TokenKind::Pipe:
        case TokenKind::Caret:
        case TokenKind::LessLess:
        case TokenKind::GreaterGreater:
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
        case TokenKind::Amp:
            result = lhs & rhs;
            break;
        case TokenKind::Pipe:
            result = lhs | rhs;
            break;
        case TokenKind::Caret:
            result = lhs ^ rhs;
            break;
        case TokenKind::LessLess:
            if (lhs < 0 || rhs < 0 || rhs >= 64) return false;
            if (lhs > (std::numeric_limits<std::int64_t>::max() >> rhs)) return false;
            result = lhs << rhs;
            break;
        case TokenKind::GreaterGreater:
            if (lhs < 0 || rhs < 0 || rhs >= 64) return false;
            result = lhs >> rhs;
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
        case IrBinaryOp::BitAnd:
        case IrBinaryOp::BitOr:
        case IrBinaryOp::BitXor:
        case IrBinaryOp::Shl:
        case IrBinaryOp::Shr:
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
        case IrBinaryOp::BitAnd:
            source_op = TokenKind::Amp;
            break;
        case IrBinaryOp::BitOr:
            source_op = TokenKind::Pipe;
            break;
        case IrBinaryOp::BitXor:
            source_op = TokenKind::Caret;
            break;
        case IrBinaryOp::Shl:
            source_op = TokenKind::LessLess;
            break;
        case IrBinaryOp::Shr:
            source_op = TokenKind::GreaterGreater;
            break;
        default:
            return false;
    }
    return fold_static_integer_binary(source_op, left, right, out);
}

} // namespace ari
