#pragma once

#include "ast.hpp"

#include <string>

namespace ari {

ExprPtr clone_expression_tree(const Expr& expr);
ExprPtr clone_expression_tree_substituting_name(const Expr& expr,
                                                const std::string& name,
                                                const Expr& replacement);
ExprPtr clone_expression_tree_substituting_name_hygienic(const Expr& expr,
                                                         const std::string& name,
                                                         const Expr& replacement,
                                                         const std::string& hygiene_prefix);
std::vector<StmtPtr> clone_statement_tree_list(const std::vector<StmtPtr>& statements);
ExprPtr clone_assignment_target(const Expr& expr);
ExprPtr clone_borrowable_receiver_expr(const Expr& expr);
bool is_assignment_target_expr(const Expr& expr);

} // namespace ari
