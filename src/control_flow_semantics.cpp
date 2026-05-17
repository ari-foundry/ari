#include "control_flow_semantics.hpp"

#include "ari_builtin.hpp"
#include "ir_builders.hpp"

#include <cstddef>
#include <optional>
#include <utility>

namespace ari {

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

bool enum_match_arm_has_refutable_payload_condition(const IrMatchArm& arm) {
    return arm.has_literal ||
           arm.has_range ||
           !arm.payload_literal_conditions.empty() ||
           !arm.payload_range_conditions.empty() ||
           !arm.payload_enum_conditions.empty();
}

bool enum_construct_matches_arm_without_refutable_payload_conditions(
    const IrExpr& match_value,
    const std::vector<IrMatchArm>& pattern_arms
) {
    if (match_value.kind != IrExprKind::EnumConstruct) return false;
    const std::string& constructed_case = ir_expr_case_name(match_value);
    if (constructed_case.empty()) return false;
    for (const auto& arm : pattern_arms) {
        if (arm.case_name == constructed_case &&
            !enum_match_arm_has_refutable_payload_condition(arm)) {
            return true;
        }
    }
    return false;
}

} // namespace ari
