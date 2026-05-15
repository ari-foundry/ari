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

bool lookup_local_rename(const CloneContext& context, const std::string& name, std::string& renamed) {
    for (auto it = context.local_renames.rbegin(); it != context.local_renames.rend(); ++it) {
        if (it->source == name) {
            renamed = it->target;
            return true;
        }
    }
    return false;
}

void add_pattern_binding_rename(CloneContext& context, std::string& name) {
    if (name.empty()) return;
    std::string original = name;
    if (!context.hygiene_prefix.empty()) {
        name = context.hygiene_prefix + "_" + std::to_string(context.next_binding++) + "_" + original;
    }
    context.local_renames.push_back(LocalRename{std::move(original), name});
}

void rename_pattern_bindings(Pattern& pattern, CloneContext& context) {
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

std::vector<StmtPtr> clone_expression_only_statement_list(const std::vector<StmtPtr>& statements) {
    if (!statements.empty()) {
        throw CompileError("internal error: statementful ast meta expression clone is not supported yet");
    }
    return {};
}

void clone_if_payload(const Expr& source,
                      Expr& target,
                      CloneContext& context) {
    if (!source.if_payload) return;
    std::size_t rename_mark = context.local_renames.size();
    std::unique_ptr<Pattern> condition_pattern;
    if (expr_if_condition_pattern(source)) {
        condition_pattern =
            std::make_unique<Pattern>(clone_pattern_with_local_renames(*expr_if_condition_pattern(source), context));
    }
    ExprPtr then_value = clone_optional_expression(expr_if_then_value(source), context);
    context.local_renames.resize(rename_mark);
    set_expr_if_payload(
        target,
        clone_optional_expression(expr_if_condition(source), context),
        std::move(condition_pattern),
        clone_expression_only_statement_list(expr_if_then_body(source)),
        std::move(then_value),
        clone_expression_only_statement_list(expr_if_else_body(source)),
        clone_optional_expression(expr_if_else_value(source), context));
}

void clone_block_payload(const Expr& source,
                         Expr& target,
                         CloneContext& context) {
    if (!source.block_payload) return;
    set_expr_block_payload(
        target,
        expr_block_label(source),
        clone_expression_only_statement_list(expr_block_body(source)),
        clone_optional_expression(expr_block_value(source), context));
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
