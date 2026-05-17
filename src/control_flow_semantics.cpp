#include "control_flow_semantics.hpp"

#include "ari_builtin.hpp"
#include "ir_builders.hpp"
#include "type_semantics.hpp"

#include <cstddef>
#include <optional>
#include <utility>

namespace ari {

namespace {

const IrExpr* enum_construct_payload_at(const IrExpr& construct, std::uint32_t index) {
    if (!construct.args.empty()) {
        if (index >= construct.args.size()) return nullptr;
        return construct.args[index].get();
    }
    if (index != 0) return nullptr;
    return ir_expr_payload(construct).get();
}

bool static_integer_payload_bits(const IrExpr& expr, const IrType& payload_type, std::uint64_t& bits) {
    if (expr.kind != IrExprKind::Integer) return false;
    unsigned width = integer_primitive_bit_width(payload_type.primitive);
    if (width == 0) return false;

    std::uint64_t raw = expr.int_negative ? 0 - expr.int_value : expr.int_value;
    if (width < 64) {
        std::uint64_t mask = (1ULL << width) - 1;
        raw &= mask;
        if (is_signed_integer_primitive(payload_type.primitive) && (raw & (1ULL << (width - 1)))) {
            raw |= ~mask;
        }
    }
    bits = raw;
    return true;
}

bool static_integer_literal_payload_bits(
    std::uint64_t value,
    bool negative,
    const IrType& payload_type,
    std::uint64_t& bits
) {
    unsigned width = integer_primitive_bit_width(payload_type.primitive);
    if (width == 0) return false;

    std::uint64_t raw = negative ? 0 - value : value;
    if (width < 64) {
        std::uint64_t mask = (1ULL << width) - 1;
        raw &= mask;
        if (is_signed_integer_primitive(payload_type.primitive) && (raw & (1ULL << (width - 1)))) {
            raw |= ~mask;
        }
    }
    bits = raw;
    return true;
}

bool static_payload_literal_matches(const IrExpr& payload, const IrPayloadLiteralCondition& condition) {
    if (condition.is_bool) {
        return payload.kind == IrExprKind::Bool && payload.bool_value == condition.bool_literal();
    }

    std::uint64_t bits = 0;
    return static_integer_payload_bits(payload, payload.type, bits) && bits == condition.bits();
}

std::uint64_t unsigned_range_endpoint(std::uint64_t value, bool negative) {
    return negative ? 0 - value : value;
}

struct SignedEndpoint {
    std::uint64_t magnitude = 0;
    bool negative = false;
};

int compare_signed_endpoint(SignedEndpoint left, SignedEndpoint right) {
    if (left.negative != right.negative) return left.negative ? -1 : 1;
    if (left.magnitude == right.magnitude) return 0;
    if (left.negative) return left.magnitude > right.magnitude ? -1 : 1;
    return left.magnitude < right.magnitude ? -1 : 1;
}

bool static_payload_range_matches(const IrExpr& payload, const IrPayloadRangeCondition& condition) {
    if (payload.kind != IrExprKind::Integer) return false;
    if (condition.is_unsigned) {
        if (payload.int_negative) return false;
        std::uint64_t value = payload.int_value;
        std::uint64_t start = unsigned_range_endpoint(condition.start_int, condition.start_negative);
        std::uint64_t end = unsigned_range_endpoint(condition.end_int, condition.end_negative);
        return value >= start && (condition.inclusive ? value <= end : value < end);
    }

    SignedEndpoint value{payload.int_value, payload.int_negative};
    SignedEndpoint start{condition.start_int, condition.start_negative};
    SignedEndpoint end{condition.end_int, condition.end_negative};
    return compare_signed_endpoint(value, start) >= 0 &&
           (condition.inclusive
                ? compare_signed_endpoint(value, end) <= 0
                : compare_signed_endpoint(value, end) < 0);
}

bool static_nested_payload_literal_matches(const IrExpr& payload, const IrPayloadEnumCondition& condition) {
    if (condition.payload_literal_is_bool) {
        return payload.kind == IrExprKind::Bool &&
               payload.bool_value == condition.payload_literal.boolean;
    }

    std::uint64_t payload_bits = 0;
    std::uint64_t condition_bits = 0;
    return static_integer_payload_bits(payload, condition.payload_type, payload_bits) &&
           static_integer_literal_payload_bits(
               condition.payload_literal.integer,
               condition.payload_literal_negative,
               condition.payload_type,
               condition_bits) &&
           payload_bits == condition_bits;
}

bool static_nested_payload_range_matches(const IrExpr& payload, const IrPayloadEnumCondition& condition) {
    if (payload.kind != IrExprKind::Integer) return false;
    if (condition.range_is_unsigned) {
        if (payload.int_negative) return false;
        std::uint64_t value = payload.int_value;
        std::uint64_t start = unsigned_range_endpoint(condition.range_start_int, condition.range_start_negative);
        std::uint64_t end = unsigned_range_endpoint(condition.range_end_int, condition.range_end_negative);
        return value >= start && (condition.range_inclusive ? value <= end : value < end);
    }

    SignedEndpoint value{payload.int_value, payload.int_negative};
    SignedEndpoint start{condition.range_start_int, condition.range_start_negative};
    SignedEndpoint end{condition.range_end_int, condition.range_end_negative};
    return compare_signed_endpoint(value, start) >= 0 &&
           (condition.range_inclusive
                ? compare_signed_endpoint(value, end) <= 0
                : compare_signed_endpoint(value, end) < 0);
}

bool static_payload_enum_condition_matches(const IrExpr& payload, const IrPayloadEnumCondition& condition) {
    if (payload.kind != IrExprKind::EnumConstruct) return false;
    if (ir_expr_enum_result_payload(payload).tag != condition.tag) return false;

    if (condition.has_payload_literal) {
        const IrExpr* nested_payload = enum_construct_payload_at(payload, condition.nested_payload_index);
        return nested_payload && static_nested_payload_literal_matches(*nested_payload, condition);
    }
    if (condition.has_payload_range) {
        const IrExpr* nested_payload = enum_construct_payload_at(payload, condition.nested_payload_index);
        return nested_payload && static_nested_payload_range_matches(*nested_payload, condition);
    }
    return true;
}

bool compact_payload_literal_matches(const IrExpr& construct, const IrMatchArm& arm) {
    const IrExpr* payload = enum_construct_payload_at(construct, 0);
    if (!payload || arm.literal_negative || arm.literal_is_bool) return false;

    std::uint64_t payload_bits = 0;
    if (payload->kind == IrExprKind::Bool) {
        payload_bits = payload->bool_value ? 1ULL : 0ULL;
    } else if (!static_integer_payload_bits(*payload, payload->type, payload_bits)) {
        return false;
    }

    return ((payload_bits << 32) | arm.enum_tag) == arm.literal_int;
}

bool enum_construct_matches_arm(const IrExpr& construct, const IrMatchArm& arm) {
    if (arm.wildcard || arm.case_name != ir_expr_case_name(construct)) return false;
    if (arm.has_range) return false;
    if (arm.has_literal && !compact_payload_literal_matches(construct, arm)) return false;

    for (const auto& condition : arm.payload_literal_conditions) {
        const IrExpr* payload = enum_construct_payload_at(construct, condition.index);
        if (!payload || !static_payload_literal_matches(*payload, condition)) return false;
    }
    for (const auto& condition : arm.payload_range_conditions) {
        const IrExpr* payload = enum_construct_payload_at(construct, condition.index);
        if (!payload || !static_payload_range_matches(*payload, condition)) return false;
    }
    for (const auto& condition : arm.payload_enum_conditions) {
        const IrExpr* payload = enum_construct_payload_at(construct, condition.index);
        if (!payload || !static_payload_enum_condition_matches(*payload, condition)) return false;
    }
    return true;
}

} // namespace

IrExprPtr make_tuple_match_block_value(SourceLocation loc,
                                       IrType result_type,
                                       std::vector<IrStmtPtr> body,
                                       IrExprPtr value) {
    return make_ir_block_expr(loc, {}, std::move(result_type), std::move(body), std::move(value));
}

std::vector<IrStmtPtr> build_tuple_match_if_chain(std::vector<TupleCheckedStmtArm>& arms) {
    std::vector<IrStmtPtr> current;
    for (std::size_t i = arms.size(); i-- > 0;) {
        TupleCheckedStmtArm& arm = arms[i];
        if (!arm.condition) {
            current = std::move(arm.body);
            continue;
        }

        auto if_stmt = std::make_unique<IrStmt>();
        if_stmt->kind = IrStmtKind::If;
        if_stmt->loc = arm.loc;
        if_stmt->condition = std::move(arm.condition);
        set_ir_stmt_then_body(*if_stmt, std::move(arm.body));
        set_ir_stmt_else_body(*if_stmt, std::move(current));
        current.clear();
        current.push_back(std::move(if_stmt));
    }
    return current;
}

IrExprPtr build_tuple_match_if_expr_chain(
    std::vector<TupleCheckedExprArm>& arms,
    const IrType& result_type,
    const std::function<IrExprPtr(SourceLocation, const IrType&)>& make_fallback
) {
    IrExprPtr current;
    for (std::size_t i = arms.size(); i-- > 0;) {
        TupleCheckedExprArm& arm = arms[i];
        if (!arm.condition) {
            current = make_tuple_match_block_value(
                arm.loc,
                result_type,
                std::move(arm.body),
                std::move(arm.value)
            );
            continue;
        }
        if (!current) {
            current = make_fallback(arm.loc, result_type);
        }

        current = make_ir_if_expr(
            arm.loc,
            result_type,
            std::move(arm.condition),
            std::move(arm.body),
            std::move(arm.value),
            {},
            std::move(current)
        );
    }
    return current;
}

bool is_diverging_builtin_symbol(const std::string& symbol) {
    return symbol == "ari_builtin_panic";
}

bool is_diverging_builtin_source_name(const std::string& source_name) {
    std::optional<std::string> symbol = ari_builtin_symbol_for_source_name(source_name);
    return symbol && is_diverging_builtin_symbol(*symbol);
}

bool is_diverging_builtin_call(const IrExpr& expr) {
    return expr.kind == IrExprKind::Call && is_diverging_builtin_source_name(ir_expr_name(expr));
}

bool is_diverging_control_flow_value(const IrExpr& expr) {
    if (is_diverging_builtin_call(expr)) return true;
    if (expr.kind == IrExprKind::Block &&
        ir_expr_block_label(expr).empty() &&
        ir_expr_block_value(expr)) {
        return is_diverging_control_flow_value(*ir_expr_block_value(expr));
    }
    return false;
}

bool enum_construct_matches_arm_statically(
    const IrExpr& match_value,
    const std::vector<IrMatchArm>& pattern_arms
) {
    if (match_value.kind != IrExprKind::EnumConstruct) return false;
    for (const auto& arm : pattern_arms) {
        if (enum_construct_matches_arm(match_value, arm)) return true;
    }
    return false;
}

} // namespace ari
