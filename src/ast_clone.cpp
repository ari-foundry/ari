#include "ast_clone.hpp"

#include "common.hpp"
#include "pattern_semantics.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

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
    set_expr_type_args(*clone, expr_type_args(expr));
    set_expr_field_names(*clone, expr_field_names(expr));
    if (expr.macro_tokens) {
        clone->macro_tokens = std::make_unique<std::vector<Token>>(*expr.macro_tokens);
    }
    copy_scalar_payload(expr, *clone);
    return clone;
}

struct NameReplacement {
    const std::string* name = nullptr;
    const Expr* expr = nullptr;
};

struct LocalRename {
    std::string source;
    std::string target;
};

struct CloneContext {
    NameReplacement replacement;
    std::string hygiene_prefix;
    std::size_t next_binding = 0;
    std::vector<LocalRename> local_renames;
};

ExprPtr clone_expression_tree_impl(const Expr& expr, CloneContext& context);
StmtPtr clone_statement_impl(const Stmt& stmt, CloneContext& context);

bool lookup_local_rename(const CloneContext& context, const std::string& name, std::string& renamed) {
    for (auto it = context.local_renames.rbegin(); it != context.local_renames.rend(); ++it) {
        if (it->source == name) {
            renamed = it->target;
            return true;
        }
    }
    return false;
}

void add_local_binding_rename(CloneContext& context, std::string& name) {
    if (name.empty()) return;
    std::string original = name;
    if (!context.hygiene_prefix.empty()) {
        name = context.hygiene_prefix + "_" + std::to_string(context.next_binding++) + "_" + original;
    }
    context.local_renames.push_back(LocalRename{std::move(original), name});
}

void add_pattern_binding_rename(CloneContext& context, std::string& name) {
    add_local_binding_rename(context, name);
}

void rename_pattern_bindings(Pattern& pattern, CloneContext& context) {
    if (!pattern.rest_alias_name.empty()) {
        add_pattern_binding_rename(context, pattern.rest_alias_name);
    }
    switch (pattern.kind) {
        case PatternKind::Binding:
            add_pattern_binding_rename(context, pattern.payload_name);
            break;
        case PatternKind::Alias:
            add_pattern_binding_rename(context, pattern.alias_name);
            break;
        case PatternKind::Wildcard:
        case PatternKind::EnumCase:
        case PatternKind::IntegerLiteral:
        case PatternKind::BoolLiteral:
        case PatternKind::Range:
        case PatternKind::Or:
        case PatternKind::Tuple:
        case PatternKind::Array:
        case PatternKind::Struct:
            break;
    }
    if (pattern.payload_pattern) {
        rename_pattern_bindings(*pattern.payload_pattern, context);
        if (pattern.has_payload_binding && pattern.payload_pattern->kind == PatternKind::Binding) {
            pattern.payload_name = pattern.payload_pattern->payload_name;
        }
    } else if (pattern.has_payload_binding) {
        add_pattern_binding_rename(context, pattern.payload_name);
    }
    if (pattern.alias_pattern) rename_pattern_bindings(*pattern.alias_pattern, context);
    for (auto& alternative : pattern.alternatives) rename_pattern_bindings(alternative, context);
    for (auto& element : pattern.elements) rename_pattern_bindings(element, context);
}

Pattern clone_pattern_with_local_renames(const Pattern& pattern, CloneContext& context) {
    Pattern copy = clone_pattern(pattern);
    rename_pattern_bindings(copy, context);
    return copy;
}

ExprPtr clone_optional_expression(const ExprPtr& expr, CloneContext& context) {
    return expr ? clone_expression_tree_impl(*expr, context) : nullptr;
}

std::vector<ExprPtr> clone_expression_list(const std::vector<ExprPtr>& exprs, CloneContext& context) {
    std::vector<ExprPtr> copies;
    copies.reserve(exprs.size());
    for (const auto& expr : exprs) {
        copies.push_back(clone_expression_tree_impl(*expr, context));
    }
    return copies;
}

Binding clone_binding_impl(const Binding& binding, CloneContext& context) {
    Binding copy;
    copy.name = binding.name;
    copy.type = binding.type;
    copy.has_type = binding.has_type;
    copy.mutable_binding = binding.mutable_binding;
    copy.loc = binding.loc;
    copy.init = clone_optional_expression(binding.init, context);
    copy.has_pattern = binding.has_pattern;
    if (binding.has_pattern) {
        copy.pattern = clone_pattern_with_local_renames(binding.pattern, context);
    } else {
        add_local_binding_rename(context, copy.name);
    }
    return copy;
}

std::vector<StmtPtr> clone_statement_list(const std::vector<StmtPtr>& statements, CloneContext& context) {
    std::vector<StmtPtr> copies;
    copies.reserve(statements.size());
    for (const auto& statement : statements) {
        copies.push_back(clone_statement_impl(*statement, context));
    }
    return copies;
}

void clone_if_payload(const Expr& source,
                      Expr& target,
                      CloneContext& context) {
    if (!source.if_payload) return;
    ExprPtr condition = clone_optional_expression(expr_if_condition(source), context);

    std::size_t then_rename_mark = context.local_renames.size();
    std::unique_ptr<Pattern> condition_pattern;
    if (expr_if_condition_pattern(source)) {
        condition_pattern =
            std::make_unique<Pattern>(clone_pattern_with_local_renames(*expr_if_condition_pattern(source), context));
    }
    std::vector<StmtPtr> then_body = clone_statement_list(expr_if_then_body(source), context);
    ExprPtr then_value = clone_optional_expression(expr_if_then_value(source), context);
    context.local_renames.resize(then_rename_mark);

    std::size_t else_rename_mark = context.local_renames.size();
    std::vector<StmtPtr> else_body = clone_statement_list(expr_if_else_body(source), context);
    ExprPtr else_value = clone_optional_expression(expr_if_else_value(source), context);
    context.local_renames.resize(else_rename_mark);

    set_expr_if_payload(
        target,
        std::move(condition),
        std::move(condition_pattern),
        std::move(then_body),
        std::move(then_value),
        std::move(else_body),
        std::move(else_value));
}

void clone_block_payload(const Expr& source,
                         Expr& target,
                         CloneContext& context) {
    if (!source.block_payload) return;
    std::size_t rename_mark = context.local_renames.size();
    std::vector<StmtPtr> body = clone_statement_list(expr_block_body(source), context);
    ExprPtr value = clone_optional_expression(expr_block_value(source), context);
    context.local_renames.resize(rename_mark);
    set_expr_block_payload(
        target,
        expr_block_label(source),
        std::move(body),
        std::move(value));
}

void clone_match_payload(const Expr& source,
                         Expr& target,
                         CloneContext& context) {
    if (!source.match_payload) return;
    std::vector<ExprMatchArm> arms;
    arms.reserve(expr_match_arms(source).size());
    for (const auto& arm : expr_match_arms(source)) {
        std::size_t rename_mark = context.local_renames.size();
        ExprMatchArm copy;
        copy.pattern = clone_pattern_with_local_renames(arm.pattern, context);
        copy.value = clone_optional_expression(arm.value, context);
        copy.loc = arm.loc;
        arms.push_back(std::move(copy));
        context.local_renames.resize(rename_mark);
    }
    set_expr_match_payload(
        target,
        clone_optional_expression(expr_match_value(source), context),
        std::move(arms));
}

ExprPtr clone_expression_tree_impl(const Expr& expr, CloneContext& context) {
    if (expr.kind == ExprKind::Name) {
        std::string renamed;
        if (lookup_local_rename(context, expr.name, renamed)) {
            ExprPtr clone = make_shallow_clone(expr);
            clone->name = std::move(renamed);
            return clone;
        }
        if (context.replacement.name &&
            expr.name == *context.replacement.name) {
            CloneContext replacement_context;
            return clone_expression_tree_impl(*context.replacement.expr, replacement_context);
        }
    }

    ExprPtr clone = make_shallow_clone(expr);
    if (expr_operand(expr)) {
        set_expr_operand(*clone, clone_expression_tree_impl(*expr_operand(expr), context));
    }
    if (expr_left(expr)) set_expr_left(*clone, clone_expression_tree_impl(*expr_left(expr), context));
    if (expr_right(expr)) {
        set_expr_right(*clone, clone_expression_tree_impl(*expr_right(expr), context));
    }
    for (const auto& arg : expr.args) clone->args.push_back(clone_expression_tree_impl(*arg, context));
    clone_if_payload(expr, *clone, context);
    clone_block_payload(expr, *clone, context);
    clone_match_payload(expr, *clone, context);
    return clone;
}

std::string clone_local_name_reference(const std::string& name, const CloneContext& context) {
    std::string renamed;
    if (lookup_local_rename(context, name, renamed)) return renamed;
    return name;
}

void clone_condition_pattern_body(const Stmt& source,
                                  Stmt& target,
                                  CloneContext& context,
                                  const std::vector<StmtPtr>& source_body,
                                  bool set_as_loop_body) {
    ExprPtr condition = clone_optional_expression(source.condition, context);
    std::size_t rename_mark = context.local_renames.size();
    if (source.has_condition_pattern && source.condition_pattern) {
        target.has_condition_pattern = true;
        target.condition_pattern =
            std::make_unique<Pattern>(clone_pattern_with_local_renames(*source.condition_pattern, context));
    }
    std::vector<StmtPtr> body = clone_statement_list(source_body, context);
    context.local_renames.resize(rename_mark);
    target.condition = std::move(condition);
    if (set_as_loop_body) {
        set_stmt_loop_body(target, std::move(body));
    } else {
        set_stmt_then_body(target, std::move(body));
    }
}

StmtPtr clone_statement_impl(const Stmt& stmt, CloneContext& context) {
    auto clone = std::make_unique<Stmt>();
    clone->kind = stmt.kind;
    clone->loc = stmt.loc;
    set_stmt_label(*clone, stmt_label(stmt));

    switch (stmt.kind) {
        case StmtKind::Block: {
            std::size_t rename_mark = context.local_renames.size();
            set_stmt_statements(*clone, clone_statement_list(stmt_statements(stmt), context));
            context.local_renames.resize(rename_mark);
            break;
        }
        case StmtKind::VarDecl:
            clone->binding = clone_binding_impl(stmt.binding, context);
            break;
        case StmtKind::Assign:
            set_stmt_assign_name(*clone, clone_local_name_reference(stmt_assign_name(stmt), context));
            if (stmt_assign_target(stmt)) {
                set_stmt_assign_target(*clone, clone_expression_tree_impl(*stmt_assign_target(stmt), context));
            }
            if (stmt_assign_rhs(stmt)) {
                set_stmt_assign_rhs(*clone, clone_expression_tree_impl(*stmt_assign_rhs(stmt), context));
            }
            break;
        case StmtKind::ExprStmt:
        case StmtKind::Return:
            clone->expr = clone_optional_expression(stmt.expr, context);
            break;
        case StmtKind::If: {
            clone_condition_pattern_body(stmt, *clone, context, stmt_then_body(stmt), false);
            std::size_t else_mark = context.local_renames.size();
            set_stmt_else_body(*clone, clone_statement_list(stmt_else_body(stmt), context));
            context.local_renames.resize(else_mark);
            break;
        }
        case StmtKind::While:
        case StmtKind::WhileLet:
            clone_condition_pattern_body(stmt, *clone, context, stmt_loop_body(stmt), true);
            break;
        case StmtKind::For: {
            clone->for_pattern_filter = stmt.for_pattern_filter;
            clone->for_iterable = clone_optional_expression(stmt.for_iterable, context);
            std::size_t rename_mark = context.local_renames.size();
            if (stmt.for_pattern) {
                clone->for_pattern =
                    std::make_unique<Pattern>(clone_pattern_with_local_renames(*stmt.for_pattern, context));
            }
            set_stmt_loop_body(*clone, clone_statement_list(stmt_loop_body(stmt), context));
            context.local_renames.resize(rename_mark);
            break;
        }
        case StmtKind::InitWhile: {
            std::size_t rename_mark = context.local_renames.size();
            for (const auto& binding : stmt.init_bindings) {
                clone->init_bindings.push_back(clone_binding_impl(binding, context));
            }
            clone->condition = clone_optional_expression(stmt.condition, context);
            std::size_t body_mark = context.local_renames.size();
            set_stmt_loop_body(*clone, clone_statement_list(stmt_loop_body(stmt), context));
            context.local_renames.resize(body_mark);
            clone->updates = clone_expression_list(stmt.updates, context);
            context.local_renames.resize(rename_mark);
            break;
        }
        case StmtKind::Continue:
            clone->updates = clone_expression_list(stmt.updates, context);
            break;
        case StmtKind::Break:
            set_stmt_break_label(*clone, stmt_break_label(stmt));
            set_stmt_break_value(*clone, clone_optional_expression(stmt_break_value(stmt), context));
            break;
        case StmtKind::Match: {
            clone->match_value = clone_optional_expression(stmt.match_value, context);
            for (const auto& arm : stmt_match_arms(stmt)) {
                std::size_t rename_mark = context.local_renames.size();
                MatchArm copy;
                copy.pattern = clone_pattern_with_local_renames(arm.pattern, context);
                copy.body = clone_statement_list(arm.body, context);
                copy.loc = arm.loc;
                ensure_stmt_match_arms(*clone).push_back(std::move(copy));
                context.local_renames.resize(rename_mark);
            }
            break;
        }
        case StmtKind::Drop:
            set_stmt_drop_name(*clone, clone_local_name_reference(stmt_drop_name(stmt), context));
            break;
    }

    return clone;
}

} // namespace

ExprPtr clone_expression_tree(const Expr& expr) {
    CloneContext context;
    return clone_expression_tree_impl(expr, context);
}

ExprPtr clone_expression_tree_substituting_name(const Expr& expr,
                                                const std::string& name,
                                                const Expr& replacement) {
    CloneContext context;
    context.replacement = NameReplacement{&name, &replacement};
    return clone_expression_tree_impl(expr, context);
}

ExprPtr clone_expression_tree_substituting_name_hygienic(const Expr& expr,
                                                         const std::string& name,
                                                         const Expr& replacement,
                                                         const std::string& hygiene_prefix) {
    CloneContext context;
    context.replacement = NameReplacement{&name, &replacement};
    context.hygiene_prefix = hygiene_prefix;
    return clone_expression_tree_impl(expr, context);
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
            if (!expr_operand(expr)) return nullptr;
            set_expr_operand(*clone, clone_borrowable_receiver_expr(*expr_operand(expr)));
            if (!expr_operand(*clone)) return nullptr;
            return clone;
        case ExprKind::Index:
            if (!expr_operand(expr) || !expr_right(expr) || expr_right(expr)->kind != ExprKind::Integer) return nullptr;
            set_expr_operand(*clone, clone_borrowable_receiver_expr(*expr_operand(expr)));
            if (!expr_operand(*clone)) return nullptr;
            set_expr_right(*clone, clone_borrowable_receiver_expr(*expr_right(expr)));
            if (!expr_right(*clone)) return nullptr;
            return clone;
        default:
            return nullptr;
    }
}

} // namespace ari
