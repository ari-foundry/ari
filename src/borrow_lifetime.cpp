#include "borrow_lifetime.hpp"

#include <utility>

namespace ari {

namespace {

void add_name_use(NameUseCounts& counts, const std::string& name) {
    if (!name.empty()) ++counts[name];
}

void collect_stmt_uses(const Stmt& stmt, NameUseCounts& counts);

void collect_stmt_list_uses(const std::vector<StmtPtr>& statements, NameUseCounts& counts) {
    for (const auto& stmt : statements) {
        if (stmt) collect_stmt_uses(*stmt, counts);
    }
}

void collect_expr_uses(const Expr* expr, NameUseCounts& counts) {
    if (!expr) return;

    switch (expr->kind) {
        case ExprKind::Integer:
        case ExprKind::Float:
        case ExprKind::String:
        case ExprKind::Bool:
        case ExprKind::Null:
        case ExprKind::MacroCall:
            if (expr->macro_tokens) {
                for (const Token& token : *expr->macro_tokens) {
                    if (token.kind == TokenKind::Identifier) add_name_use(counts, token.text);
                }
            }
            return;
        case ExprKind::Name:
            add_name_use(counts, expr->name);
            return;
        case ExprKind::Borrow:
            if (expr_operand(*expr)) {
                collect_expr_uses(expr_operand(*expr).get(), counts);
            } else {
                add_name_use(counts, expr->name);
            }
            return;
        case ExprKind::Unary:
        case ExprKind::Cast:
        case ExprKind::Try:
        case ExprKind::TupleIndex:
        case ExprKind::FieldAccess:
            collect_expr_uses(expr_operand(*expr).get(), counts);
            return;
        case ExprKind::Index:
        case ExprKind::Binary:
        case ExprKind::NullCoalesce:
            collect_expr_uses(expr_left(*expr).get(), counts);
            collect_expr_uses(expr_right(*expr).get(), counts);
            return;
        case ExprKind::Tuple:
        case ExprKind::StructLiteral:
        case ExprKind::Vector:
            for (const auto& arg : expr->args) collect_expr_uses(arg.get(), counts);
            return;
        case ExprKind::Call:
            add_name_use(counts, expr->name);
            for (const auto& arg : expr->args) collect_expr_uses(arg.get(), counts);
            return;
        case ExprKind::MethodCall:
            collect_expr_uses(expr_operand(*expr).get(), counts);
            for (const auto& arg : expr->args) collect_expr_uses(arg.get(), counts);
            return;
        case ExprKind::If:
            collect_expr_uses(expr_if_condition(*expr).get(), counts);
            collect_stmt_list_uses(expr_if_then_body(*expr), counts);
            collect_expr_uses(expr_if_then_value(*expr).get(), counts);
            collect_stmt_list_uses(expr_if_else_body(*expr), counts);
            collect_expr_uses(expr_if_else_value(*expr).get(), counts);
            return;
        case ExprKind::Block:
            collect_stmt_list_uses(expr_block_body(*expr), counts);
            collect_expr_uses(expr_block_value(*expr).get(), counts);
            return;
        case ExprKind::Match:
            collect_expr_uses(expr_match_value(*expr).get(), counts);
            for (const auto& arm : expr_match_arms(*expr)) {
                collect_expr_uses(arm.value.get(), counts);
            }
            return;
    }
}

void collect_binding_init_uses(const Binding& binding, NameUseCounts& counts) {
    collect_expr_uses(binding.init.get(), counts);
}

void collect_stmt_uses(const Stmt& stmt, NameUseCounts& counts) {
    switch (stmt.kind) {
        case StmtKind::Block:
            collect_stmt_list_uses(stmt_statements(stmt), counts);
            return;
        case StmtKind::VarDecl:
            collect_binding_init_uses(stmt.binding, counts);
            return;
        case StmtKind::Assign:
            collect_expr_uses(stmt_assign_target(stmt).get(), counts);
            collect_expr_uses(stmt_assign_rhs(stmt).get(), counts);
            return;
        case StmtKind::ExprStmt:
        case StmtKind::Return:
            collect_expr_uses(stmt.expr.get(), counts);
            return;
        case StmtKind::If:
            collect_expr_uses(stmt.condition.get(), counts);
            collect_stmt_list_uses(stmt_then_body(stmt), counts);
            collect_stmt_list_uses(stmt_else_body(stmt), counts);
            return;
        case StmtKind::While:
        case StmtKind::WhileLet:
            collect_expr_uses(stmt.condition.get(), counts);
            collect_stmt_list_uses(stmt_loop_body(stmt), counts);
            return;
        case StmtKind::For:
            collect_expr_uses(stmt.for_iterable.get(), counts);
            collect_stmt_list_uses(stmt_loop_body(stmt), counts);
            return;
        case StmtKind::InitWhile:
            for (const auto& binding : stmt.init_bindings) collect_binding_init_uses(binding, counts);
            collect_expr_uses(stmt.condition.get(), counts);
            collect_stmt_list_uses(stmt_loop_body(stmt), counts);
            for (const auto& update : stmt.updates) collect_expr_uses(update.get(), counts);
            return;
        case StmtKind::Continue:
            for (const auto& update : stmt.updates) collect_expr_uses(update.get(), counts);
            return;
        case StmtKind::Break:
            collect_expr_uses(stmt_break_value(stmt).get(), counts);
            return;
        case StmtKind::Match:
            collect_expr_uses(stmt.match_value.get(), counts);
            for (const auto& arm : stmt_match_arms(stmt)) {
                collect_stmt_list_uses(arm.body, counts);
            }
            return;
        case StmtKind::Drop:
        case StmtKind::Forget:
            add_name_use(counts, stmt_drop_name(stmt));
            return;
    }
}

} // namespace

StatementNameUses collect_statement_name_uses(const std::vector<StmtPtr>& statements) {
    StatementNameUses result;
    result.per_statement.reserve(statements.size());
    for (const auto& stmt : statements) {
        NameUseCounts counts;
        if (stmt) collect_stmt_uses(*stmt, counts);
        for (const auto& item : counts) result.remaining[item.first] += item.second;
        result.per_statement.push_back(std::move(counts));
    }
    return result;
}

void subtract_name_uses(NameUseCounts& remaining, const NameUseCounts& used) {
    for (const auto& item : used) {
        auto found = remaining.find(item.first);
        if (found == remaining.end()) continue;
        if (found->second <= item.second) {
            remaining.erase(found);
        } else {
            found->second -= item.second;
        }
    }
}

bool has_remaining_name_use(const NameUseCounts& remaining, const std::string& name) {
    auto found = remaining.find(name);
    return found != remaining.end() && found->second > 0;
}

} // namespace ari
