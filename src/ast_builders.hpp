#pragma once

#include "ast.hpp"

#include <cstdint>
#include <string>

namespace ari {

ExprPtr make_ast_integer_expr(SourceLocation loc,
                              std::uint64_t value,
                              bool negative = false,
                              std::string literal_suffix = {});
ExprPtr make_ast_float_expr(SourceLocation loc, double value, std::string literal_suffix = {});
ExprPtr make_ast_bool_expr(SourceLocation loc, bool value);
ExprPtr make_ast_name_expr(SourceLocation loc, std::string name);
ExprPtr make_ast_tuple_index_expr(SourceLocation loc, ExprPtr operand, std::uint64_t index);
ExprPtr make_ast_borrow_expr(SourceLocation loc, ExprPtr operand, bool mutable_borrow);

} // namespace ari
