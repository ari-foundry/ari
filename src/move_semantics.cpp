#include "move_semantics.hpp"

namespace ari {

bool is_take_place_expression(const Expr& expr) {
    switch (expr.kind) {
        case ExprKind::Name:
            return true;
        case ExprKind::FieldAccess:
        case ExprKind::TupleIndex:
            return expr_operand(expr) && is_take_place_expression(*expr_operand(expr));
        case ExprKind::Index:
            return expr_operand(expr) && is_take_place_expression(*expr_operand(expr));
        default:
            return false;
    }
}

const char* take_place_expectation() {
    return "take expects a local binding, field, tuple index, or index expression";
}

} // namespace ari
