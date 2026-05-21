#pragma once

#include "ast.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ari {

ExprPtr make_ast_integer_expr(SourceLocation loc,
                              std::uint64_t value,
                              bool negative = false,
                              std::string literal_suffix = {});
ExprPtr make_ast_float_expr(SourceLocation loc, double value, std::string literal_suffix = {});
ExprPtr make_ast_string_expr(SourceLocation loc, std::string value);
ExprPtr make_ast_bool_expr(SourceLocation loc, bool value);
ExprPtr make_ast_null_expr(SourceLocation loc);
ExprPtr make_ast_name_expr(SourceLocation loc, std::string name);
ExprPtr make_ast_unary_expr(SourceLocation loc, TokenKind op, ExprPtr operand);
ExprPtr make_ast_binary_expr(SourceLocation loc, TokenKind op, ExprPtr left, ExprPtr right);
ExprPtr make_ast_cast_expr(SourceLocation loc, ExprPtr operand, TypeRef type);
ExprPtr make_ast_try_expr(SourceLocation loc, ExprPtr operand);
ExprPtr make_ast_tuple_index_expr(SourceLocation loc, ExprPtr operand, std::uint64_t index);
ExprPtr make_ast_index_expr(SourceLocation loc, ExprPtr operand, ExprPtr index);
ExprPtr make_ast_field_access_expr(SourceLocation loc, ExprPtr operand, std::string name);
ExprPtr make_ast_borrow_expr(SourceLocation loc, ExprPtr operand, bool mutable_borrow);
ExprPtr make_ast_tuple_expr(SourceLocation loc, std::vector<ExprPtr> elements = {});
ExprPtr make_ast_vector_expr(SourceLocation loc, std::vector<ExprPtr> elements = {});
ExprPtr make_ast_struct_literal_expr(SourceLocation loc,
                                     std::string name,
                                     std::vector<TypeRef> type_args,
                                     std::vector<std::string> field_names,
                                     std::vector<ExprPtr> field_values);
ExprPtr make_ast_block_expr(SourceLocation loc,
                            std::string label,
                            std::vector<StmtPtr> body,
                            ExprPtr value);
ExprPtr make_ast_lambda_expr(SourceLocation loc,
                             std::vector<Param> params,
                             bool has_result_type,
                             TypeRef result_type,
                             std::vector<StmtPtr> body,
                             ExprPtr value);
ExprPtr make_ast_if_expr(SourceLocation loc,
                         ExprPtr condition,
                         std::unique_ptr<Pattern> condition_pattern,
                         std::vector<StmtPtr> then_body,
                         ExprPtr then_value,
                         std::vector<StmtPtr> else_body,
                         ExprPtr else_value);
ExprPtr make_ast_call_expr(SourceLocation loc,
                           std::string name,
                           ExprPtr operand,
                           std::vector<ExprPtr> args = {});
ExprPtr make_ast_method_call_expr(SourceLocation loc,
                                  ExprPtr operand,
                                  std::string name,
                                  std::vector<TypeRef> type_args,
                                  std::vector<ExprPtr> args = {});
ExprPtr make_ast_match_expr(SourceLocation loc, ExprPtr value, std::vector<ExprMatchArm> arms);
ExprPtr make_ast_macro_call_expr(SourceLocation loc, std::string name, std::vector<Token> tokens);

} // namespace ari
