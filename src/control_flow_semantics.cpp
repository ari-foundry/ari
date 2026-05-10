#include "control_flow_semantics.hpp"

#include <cstddef>
#include <memory>
#include <utility>

namespace ari {

IrExprPtr make_tuple_match_block_value(SourceLocation loc,
                                       IrType result_type,
                                       std::vector<IrStmtPtr> body,
                                       IrExprPtr value) {
    auto block = std::make_unique<IrExpr>();
    block->kind = IrExprKind::Block;
    block->loc = loc;
    block->type = std::move(result_type);
    block->block_body = std::move(body);
    block->block_value = std::move(value);
    return block;
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
        if_stmt->then_body = std::move(arm.body);
        if_stmt->else_body = std::move(current);
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

        auto if_expr = std::make_unique<IrExpr>();
        if_expr->kind = IrExprKind::If;
        if_expr->loc = arm.loc;
        if_expr->type = result_type;
        if_expr->condition = std::move(arm.condition);
        if_expr->then_body = std::move(arm.body);
        if_expr->then_value = std::move(arm.value);
        if_expr->else_value = std::move(current);
        current = std::move(if_expr);
    }
    return current;
}

} // namespace ari
