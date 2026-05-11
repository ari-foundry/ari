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

ExprPtr make_ast_string_expr(SourceLocation loc, std::string value) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::String;
    expr->loc = loc;
    expr->string_value = std::move(value);
    return expr;
}

ExprPtr make_ast_bool_expr(SourceLocation loc, bool value) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Bool;
    expr->loc = loc;
    expr->bool_value = value;
    return expr;
}

ExprPtr make_ast_null_expr(SourceLocation loc) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Null;
    expr->loc = loc;
    return expr;
}

ExprPtr make_ast_name_expr(SourceLocation loc, std::string name) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Name;
    expr->loc = loc;
    expr->name = std::move(name);
    return expr;
}

ExprPtr make_ast_unary_expr(SourceLocation loc, TokenKind op, ExprPtr operand) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Unary;
    expr->loc = loc;
    expr->op = op;
    set_expr_operand(*expr, std::move(operand));
    return expr;
}

ExprPtr make_ast_binary_expr(SourceLocation loc, TokenKind op, ExprPtr left, ExprPtr right) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Binary;
    expr->loc = loc;
    expr->op = op;
    set_expr_left(*expr, std::move(left));
    set_expr_right(*expr, std::move(right));
    return expr;
}

ExprPtr make_ast_cast_expr(SourceLocation loc, ExprPtr operand, TypeRef type) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Cast;
    expr->loc = loc;
    expr->cast_type = std::move(type);
    set_expr_operand(*expr, std::move(operand));
    return expr;
}

ExprPtr make_ast_try_expr(SourceLocation loc, ExprPtr operand) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Try;
    expr->loc = loc;
    set_expr_operand(*expr, std::move(operand));
    return expr;
}

ExprPtr make_ast_tuple_expr(SourceLocation loc, std::vector<ExprPtr> elements) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Tuple;
    expr->loc = loc;
    expr->args = std::move(elements);
    return expr;
}

ExprPtr make_ast_vector_expr(SourceLocation loc, std::vector<ExprPtr> elements) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Vector;
    expr->loc = loc;
    expr->args = std::move(elements);
    return expr;
}

ExprPtr make_ast_struct_literal_expr(SourceLocation loc,
                                     std::string name,
                                     std::vector<TypeRef> type_args,
                                     std::vector<std::string> field_names,
                                     std::vector<ExprPtr> field_values) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::StructLiteral;
    expr->loc = loc;
    expr->name = std::move(name);
    set_expr_type_args(*expr, std::move(type_args));
    set_expr_field_names(*expr, std::move(field_names));
    expr->args = std::move(field_values);
    return expr;
}

ExprPtr make_ast_block_expr(SourceLocation loc,
                            std::string label,
                            std::vector<StmtPtr> body,
                            ExprPtr value) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Block;
    expr->loc = loc;
    set_expr_block_payload(*expr, std::move(label), std::move(body), std::move(value));
    return expr;
}

ExprPtr make_ast_if_expr(SourceLocation loc,
                         ExprPtr condition,
                         std::unique_ptr<Pattern> condition_pattern,
                         std::vector<StmtPtr> then_body,
                         ExprPtr then_value,
                         std::vector<StmtPtr> else_body,
                         ExprPtr else_value) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::If;
    expr->loc = loc;
    set_expr_if_payload(
        *expr,
        std::move(condition),
        std::move(condition_pattern),
        std::move(then_body),
        std::move(then_value),
        std::move(else_body),
        std::move(else_value));
    return expr;
}

ExprPtr make_ast_match_expr(SourceLocation loc, ExprPtr value, std::vector<ExprMatchArm> arms) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Match;
    expr->loc = loc;
    set_expr_match_payload(*expr, std::move(value), std::move(arms));
    return expr;
}

ExprPtr make_ast_macro_call_expr(SourceLocation loc, std::string name, std::vector<Token> tokens) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::MacroCall;
    expr->loc = loc;
    expr->name = std::move(name);
    expr->macro_tokens = std::make_unique<std::vector<Token>>(std::move(tokens));
    return expr;
}

ExprPtr make_ast_tuple_index_expr(SourceLocation loc, ExprPtr operand, std::uint64_t index) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::TupleIndex;
    expr->loc = loc;
    set_expr_operand(*expr, std::move(operand));
    expr->tuple_index = index;
    return expr;
}

ExprPtr make_ast_index_expr(SourceLocation loc, ExprPtr operand, ExprPtr index) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Index;
    expr->loc = loc;
    set_expr_operand(*expr, std::move(operand));
    set_expr_right(*expr, std::move(index));
    return expr;
}

ExprPtr make_ast_field_access_expr(SourceLocation loc, ExprPtr operand, std::string name) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::FieldAccess;
    expr->loc = loc;
    expr->name = std::move(name);
    set_expr_operand(*expr, std::move(operand));
    return expr;
}

ExprPtr make_ast_borrow_expr(SourceLocation loc, ExprPtr operand, bool mutable_borrow) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Borrow;
    expr->loc = loc;
    expr->mutable_borrow = mutable_borrow;
    if (operand && operand->kind == ExprKind::Name) expr->name = operand->name;
    set_expr_operand(*expr, std::move(operand));
    return expr;
}

ExprPtr make_ast_call_expr(SourceLocation loc, std::string name, ExprPtr operand, std::vector<ExprPtr> args) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Call;
    expr->loc = loc;
    expr->name = std::move(name);
    set_expr_operand(*expr, std::move(operand));
    expr->args = std::move(args);
    return expr;
}

ExprPtr make_ast_method_call_expr(SourceLocation loc,
                                  ExprPtr operand,
                                  std::string name,
                                  std::vector<TypeRef> type_args,
                                  std::vector<ExprPtr> args) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::MethodCall;
    expr->loc = loc;
    expr->name = std::move(name);
    set_expr_operand(*expr, std::move(operand));
    set_expr_type_args(*expr, std::move(type_args));
    expr->args = std::move(args);
    return expr;
}

} // namespace ari
