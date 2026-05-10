#pragma once

#include "ast.hpp"

namespace ari {

ExprPtr clone_expression_tree(const Expr& expr);
ExprPtr clone_assignment_target(const Expr& expr);
ExprPtr clone_borrowable_receiver_expr(const Expr& expr);
bool is_assignment_target_expr(const Expr& expr);

} // namespace ari
