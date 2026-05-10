#include "ast_builders.hpp"

#include <memory>
#include <utility>

namespace ari {

ExprPtr make_ast_integer_expr(SourceLocation loc,
                              std::uint64_t value,
                              bool negative,
                              std::string literal_suffix) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Integer;
    expr->loc = loc;
    expr->int_value = value;
    expr->int_negative = negative;
    expr->literal_suffix = std::move(literal_suffix);
    return expr;
}

ExprPtr make_ast_float_expr(SourceLocation loc, double value, std::string literal_suffix) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Float;
    expr->loc = loc;
    expr->float_value = value;
    expr->literal_suffix = std::move(literal_suffix);
    return expr;
}

ExprPtr make_ast_bool_expr(SourceLocation loc, bool value) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Bool;
    expr->loc = loc;
    expr->bool_value = value;
    return expr;
}

ExprPtr make_ast_name_expr(SourceLocation loc, std::string name) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Name;
    expr->loc = loc;
    expr->name = std::move(name);
    return expr;
}

ExprPtr make_ast_tuple_index_expr(SourceLocation loc, ExprPtr operand, std::uint64_t index) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::TupleIndex;
    expr->loc = loc;
    expr->operand = std::move(operand);
    expr->tuple_index = index;
    return expr;
}

ExprPtr make_ast_borrow_expr(SourceLocation loc, ExprPtr operand, bool mutable_borrow) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Borrow;
    expr->loc = loc;
    expr->mutable_borrow = mutable_borrow;
    if (operand && operand->kind == ExprKind::Name) expr->name = operand->name;
    expr->operand = std::move(operand);
    return expr;
}

} // namespace ari
