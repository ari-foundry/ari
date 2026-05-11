#include "ast_clone.hpp"

#include "common.hpp"

#include <memory>

namespace ari {

namespace {

void copy_scalar_payload(const Expr& source, Expr& target) {
    switch (source.kind) {
        case ExprKind::Integer:
            target.int_value = source.int_value;
            break;
        case ExprKind::Float:
            target.float_value = source.float_value;
            break;
        case ExprKind::Bool:
            target.bool_value = source.bool_value;
            break;
        case ExprKind::TupleIndex:
            target.tuple_index = source.tuple_index;
            break;
        default:
            break;
    }
}

ExprPtr make_shallow_clone(const Expr& expr) {
    auto clone = std::make_unique<Expr>();
    clone->kind = expr.kind;
    clone->loc = expr.loc;
    clone->int_negative = expr.int_negative;
    clone->literal_suffix = expr.literal_suffix;
    clone->string_value = expr.string_value;
    clone->name = expr.name;
    clone->mutable_borrow = expr.mutable_borrow;
    clone->op = expr.op;
    clone->cast_type = expr.cast_type;
    set_expr_receiver_type_args(*clone, expr_receiver_type_args(expr));
    clone->type_args = expr.type_args;
    clone->field_names = expr.field_names;
    if (expr.macro_tokens) {
        clone->macro_tokens = std::make_unique<std::vector<Token>>(*expr.macro_tokens);
    }
    copy_scalar_payload(expr, *clone);
    return clone;
}

} // namespace

ExprPtr clone_expression_tree(const Expr& expr) {
    ExprPtr clone = make_shallow_clone(expr);
    if (expr.operand) clone->operand = clone_expression_tree(*expr.operand);
    if (expr.left) clone->left = clone_expression_tree(*expr.left);
    if (expr.right) clone->right = clone_expression_tree(*expr.right);
    for (const auto& arg : expr.args) clone->args.push_back(clone_expression_tree(*arg));
    return clone;
}

bool is_assignment_target_expr(const Expr& expr) {
    return expr.kind == ExprKind::Name ||
           expr.kind == ExprKind::FieldAccess ||
           expr.kind == ExprKind::TupleIndex ||
           expr.kind == ExprKind::Index ||
           (expr.kind == ExprKind::Unary && expr.op == TokenKind::Star);
}

ExprPtr clone_assignment_target(const Expr& expr) {
    if (!is_assignment_target_expr(expr)) {
        throw CompileError(where(expr.loc) +
                           ": assignment target must be a binding, field access, index access, or pointer dereference");
    }
    return clone_expression_tree(expr);
}

ExprPtr clone_borrowable_receiver_expr(const Expr& expr) {
    ExprPtr clone = make_shallow_clone(expr);

    switch (expr.kind) {
        case ExprKind::Name:
            return clone;
        case ExprKind::Integer:
            return clone;
        case ExprKind::FieldAccess:
        case ExprKind::TupleIndex:
            if (!expr.operand) return nullptr;
            clone->operand = clone_borrowable_receiver_expr(*expr.operand);
            if (!clone->operand) return nullptr;
            return clone;
        case ExprKind::Index:
            if (!expr.operand || !expr.right || expr.right->kind != ExprKind::Integer) return nullptr;
            clone->operand = clone_borrowable_receiver_expr(*expr.operand);
            if (!clone->operand) return nullptr;
            clone->right = clone_borrowable_receiver_expr(*expr.right);
            if (!clone->right) return nullptr;
            return clone;
        default:
            return nullptr;
    }
}

} // namespace ari
