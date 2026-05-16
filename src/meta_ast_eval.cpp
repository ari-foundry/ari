#include "meta_ast_eval.hpp"

#include "common.hpp"
#include "meta_token_eval.hpp"
#include "module_path.hpp"
#include "type_semantics.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ari {
namespace {

[[noreturn]] void fail_eval(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

bool is_decl_ast_constructor(const Expr& expr) {
    return expr.kind == ExprKind::MacroCall && expr.name == "decl";
}

bool supported_decl_constructor(const Expr& expr, std::string& reason) {
    if (!is_decl_ast_constructor(expr)) return false;
    if (!expr.macro_tokens || expr.macro_tokens->empty()) {
        reason = "decl! ast constructor requires one or more declaration tokens";
        return false;
    }
    return true;
}

bool is_meta_input_name(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::Name && expr.name == input_name;
}

bool is_free_decl_call(const Expr& expr,
                       const std::string& name,
                       const std::string& input_name,
                       std::size_t arg_count) {
    return expr.kind == ExprKind::Call &&
           expr.name == name &&
           !expr_operand(expr) &&
           expr_type_args(expr).empty() &&
           expr.args.size() == arg_count &&
           !expr.args.empty() &&
           is_meta_input_name(*expr.args.front(), input_name);
}

bool is_decl_method_call(const Expr& expr,
                         const std::string& name,
                         const std::string& input_name,
                         std::size_t arg_count) {
    return expr.kind == ExprKind::MethodCall &&
           expr.name == name &&
           expr_receiver_type_args(expr).empty() &&
           expr_type_args(expr).empty() &&
           expr_operand(expr) &&
           is_meta_input_name(*expr_operand(expr), input_name) &&
           expr.args.size() == arg_count;
}

bool is_decl_kind_expr(const Expr& expr, const std::string& input_name) {
    return is_free_decl_call(expr, "decl_kind", input_name, 1) ||
           is_decl_method_call(expr, "kind", input_name, 0);
}

bool is_decl_name_expr(const Expr& expr, const std::string& input_name) {
    return is_free_decl_call(expr, "decl_name", input_name, 1) ||
           is_decl_method_call(expr, "name", input_name, 0);
}

bool is_decl_count_expr(const Expr& expr, const std::string& input_name) {
    return is_free_decl_call(expr, "decl_count", input_name, 1) ||
           is_decl_method_call(expr, "count", input_name, 0);
}

enum class DeclIntProperty {
    DeclarationCount,
    GenericCount,
    ParamCount,
    FieldCount,
    CaseCount,
    MethodCount,
    AssociatedTypeCount
};

bool is_decl_int_property_expr(const Expr& expr,
                               const std::string& input_name,
                               const std::string& free_name,
                               const std::string& method_name) {
    return is_free_decl_call(expr, free_name, input_name, 1) ||
           is_decl_method_call(expr, method_name, input_name, 0);
}

std::optional<DeclIntProperty> decl_int_property_expr(const Expr& expr,
                                                      const std::string& input_name) {
    if (is_decl_count_expr(expr, input_name)) return DeclIntProperty::DeclarationCount;
    if (is_decl_int_property_expr(expr, input_name, "decl_generic_count", "generic_count")) {
        return DeclIntProperty::GenericCount;
    }
    if (is_decl_int_property_expr(expr, input_name, "decl_param_count", "param_count")) {
        return DeclIntProperty::ParamCount;
    }
    if (is_decl_int_property_expr(expr, input_name, "decl_field_count", "field_count")) {
        return DeclIntProperty::FieldCount;
    }
    if (is_decl_int_property_expr(expr, input_name, "decl_case_count", "case_count")) {
        return DeclIntProperty::CaseCount;
    }
    if (is_decl_int_property_expr(expr, input_name, "decl_method_count", "method_count")) {
        return DeclIntProperty::MethodCount;
    }
    if (is_decl_int_property_expr(expr, input_name, "decl_associated_type_count", "associated_type_count")) {
        return DeclIntProperty::AssociatedTypeCount;
    }
    return std::nullopt;
}

bool is_decl_public_expr(const Expr& expr, const std::string& input_name) {
    return is_free_decl_call(expr, "decl_is_public", input_name, 1) ||
           is_decl_method_call(expr, "is_public", input_name, 0);
}

bool is_decl_is_expr(const Expr& expr, const std::string& input_name) {
    if (is_free_decl_call(expr, "decl_is", input_name, 2)) {
        return expr.args[1] && expr.args[1]->kind == ExprKind::String;
    }
    return is_decl_method_call(expr, "is", input_name, 1) &&
           expr.args[0] &&
           expr.args[0]->kind == ExprKind::String;
}

bool is_decl_string_expr(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::String ||
           is_decl_kind_expr(expr, input_name) ||
           is_decl_name_expr(expr, input_name);
}

bool is_decl_int_expr(const Expr& expr, const std::string& input_name) {
    return (expr.kind == ExprKind::Integer && !expr.int_negative) ||
           decl_int_property_expr(expr, input_name).has_value();
}

std::string decl_condition_support_message() {
    return
        "ast declaration meta branch conditions currently support bool literals, !, &&, ||, decl_is_public(input), input.is_public(), decl_is(input, \"kind\"), input.is(\"kind\"), string comparisons over decl_kind(input)/input.kind()/decl_name(input)/input.name(), and integer comparisons over decl_count(input)/input.count(), decl_generic_count(input)/input.generic_count(), decl_param_count(input)/input.param_count(), decl_field_count(input)/input.field_count(), decl_case_count(input)/input.case_count(), decl_method_count(input)/input.method_count(), and decl_associated_type_count(input)/input.associated_type_count()";
}

bool supported_decl_condition(const Expr& expr, const std::string& input_name, std::string& reason);

bool supported_decl_comparison(const Expr& left,
                               const Expr& right,
                               TokenKind op,
                               const std::string& input_name,
                               std::string& reason) {
    if (op == TokenKind::EqEq || op == TokenKind::BangEq) {
        if (is_decl_string_expr(left, input_name) && is_decl_string_expr(right, input_name)) return true;
        if (is_decl_int_expr(left, input_name) && is_decl_int_expr(right, input_name)) return true;
        if (supported_decl_condition(left, input_name, reason) &&
            supported_decl_condition(right, input_name, reason)) {
            return true;
        }
    }
    if ((op == TokenKind::Less || op == TokenKind::LessEq ||
         op == TokenKind::Greater || op == TokenKind::GreaterEq) &&
        is_decl_int_expr(left, input_name) &&
        is_decl_int_expr(right, input_name)) {
        return true;
    }
    reason = decl_condition_support_message();
    return false;
}

bool supported_decl_condition(const Expr& expr, const std::string& input_name, std::string& reason) {
    switch (expr.kind) {
        case ExprKind::Bool:
            return true;
        case ExprKind::Unary:
            if (expr.op == TokenKind::Bang && expr_operand(expr)) {
                return supported_decl_condition(*expr_operand(expr), input_name, reason);
            }
            break;
        case ExprKind::Binary:
            if (!expr_left(expr) || !expr_right(expr)) {
                reason = "malformed ast declaration meta branch condition";
                return false;
            }
            if (expr.op == TokenKind::AmpAmp || expr.op == TokenKind::PipePipe) {
                return supported_decl_condition(*expr_left(expr), input_name, reason) &&
                       supported_decl_condition(*expr_right(expr), input_name, reason);
            }
            return supported_decl_comparison(*expr_left(expr), *expr_right(expr), expr.op, input_name, reason);
        case ExprKind::Call:
        case ExprKind::MethodCall:
            if (is_decl_public_expr(expr, input_name) || is_decl_is_expr(expr, input_name)) return true;
            break;
        default:
            break;
    }
    reason = decl_condition_support_message();
    return false;
}

std::string evaluate_decl_string_expr(const Expr& expr,
                                      const std::string& input_name,
                                      const MetaAstDeclInput& input) {
    if (expr.kind == ExprKind::String) return expr.string_value;
    if (is_decl_kind_expr(expr, input_name)) return input.kind;
    if (is_decl_name_expr(expr, input_name)) return input.name;
    fail_eval(expr.loc, "internal error: unsupported ast declaration string expression");
}

std::uint64_t evaluate_decl_int_expr(const Expr& expr,
                                     const std::string& input_name,
                                     const MetaAstDeclInput& input) {
    if (expr.kind == ExprKind::Integer && !expr.int_negative) return expr.int_value;
    if (auto property = decl_int_property_expr(expr, input_name)) {
        switch (*property) {
            case DeclIntProperty::DeclarationCount:
                return static_cast<std::uint64_t>(input.count);
            case DeclIntProperty::GenericCount:
                return static_cast<std::uint64_t>(input.generic_count);
            case DeclIntProperty::ParamCount:
                return static_cast<std::uint64_t>(input.param_count);
            case DeclIntProperty::FieldCount:
                return static_cast<std::uint64_t>(input.field_count);
            case DeclIntProperty::CaseCount:
                return static_cast<std::uint64_t>(input.case_count);
            case DeclIntProperty::MethodCount:
                return static_cast<std::uint64_t>(input.method_count);
            case DeclIntProperty::AssociatedTypeCount:
                return static_cast<std::uint64_t>(input.associated_type_count);
        }
    }
    fail_eval(expr.loc, "internal error: unsupported ast declaration integer expression");
}

bool evaluate_decl_condition(const Expr& expr, const std::string& input_name, const MetaAstDeclInput& input) {
    switch (expr.kind) {
        case ExprKind::Bool:
            return expr.bool_value;
        case ExprKind::Unary:
            if (expr.op == TokenKind::Bang && expr_operand(expr)) {
                return !evaluate_decl_condition(*expr_operand(expr), input_name, input);
            }
            break;
        case ExprKind::Binary: {
            if (!expr_left(expr) || !expr_right(expr)) {
                fail_eval(expr.loc, "malformed ast declaration meta branch condition");
            }
            if (expr.op == TokenKind::AmpAmp) {
                return evaluate_decl_condition(*expr_left(expr), input_name, input) &&
                       evaluate_decl_condition(*expr_right(expr), input_name, input);
            }
            if (expr.op == TokenKind::PipePipe) {
                return evaluate_decl_condition(*expr_left(expr), input_name, input) ||
                       evaluate_decl_condition(*expr_right(expr), input_name, input);
            }
            if (is_decl_string_expr(*expr_left(expr), input_name) &&
                is_decl_string_expr(*expr_right(expr), input_name)) {
                bool equal =
                    evaluate_decl_string_expr(*expr_left(expr), input_name, input) ==
                    evaluate_decl_string_expr(*expr_right(expr), input_name, input);
                if (expr.op == TokenKind::EqEq) return equal;
                if (expr.op == TokenKind::BangEq) return !equal;
            }
            if (is_decl_int_expr(*expr_left(expr), input_name) &&
                is_decl_int_expr(*expr_right(expr), input_name)) {
                std::uint64_t left = evaluate_decl_int_expr(*expr_left(expr), input_name, input);
                std::uint64_t right = evaluate_decl_int_expr(*expr_right(expr), input_name, input);
                switch (expr.op) {
                    case TokenKind::EqEq: return left == right;
                    case TokenKind::BangEq: return left != right;
                    case TokenKind::Less: return left < right;
                    case TokenKind::LessEq: return left <= right;
                    case TokenKind::Greater: return left > right;
                    case TokenKind::GreaterEq: return left >= right;
                    default: break;
                }
            }
            if (expr.op == TokenKind::EqEq || expr.op == TokenKind::BangEq) {
                bool equal =
                    evaluate_decl_condition(*expr_left(expr), input_name, input) ==
                    evaluate_decl_condition(*expr_right(expr), input_name, input);
                return expr.op == TokenKind::EqEq ? equal : !equal;
            }
            break;
        }
        case ExprKind::Call:
        case ExprKind::MethodCall:
            if (is_decl_public_expr(expr, input_name)) return input.is_public;
            if (is_decl_is_expr(expr, input_name)) {
                const Expr& kind_arg =
                    expr.kind == ExprKind::Call ? *expr.args[1] : *expr.args[0];
                return input.kind == kind_arg.string_value;
            }
            break;
        default:
            break;
    }
    fail_eval(expr.loc, "internal error: unsupported ast declaration meta branch condition");
}

bool supported_decl_return_expr(const Expr& expr, const std::string& input_name, std::string& reason) {
    if (supported_decl_constructor(expr, reason)) return true;
    if (is_decl_ast_constructor(expr)) return false;
    if (expr.kind != ExprKind::If) {
        reason =
            "ast declaration returns currently support decl!(...) or expression-only if branches whose arms return decl!(...)";
        return false;
    }
    if (expr_if_condition_pattern(expr) ||
        !expr_if_then_body(expr).empty() ||
        !expr_if_else_body(expr).empty() ||
        !expr_if_condition(expr) ||
        !expr_if_then_value(expr) ||
        !expr_if_else_value(expr)) {
        reason =
            "ast declaration meta branches currently require expression-only if arms returning decl!(...)";
        return false;
    }
    if (!supported_decl_condition(*expr_if_condition(expr), input_name, reason)) return false;
    return supported_decl_return_expr(*expr_if_then_value(expr), input_name, reason) &&
           supported_decl_return_expr(*expr_if_else_value(expr), input_name, reason);
}

std::vector<Token> evaluate_decl_return_expr(const Expr& expr,
                                             const std::string& input_name,
                                             const MetaAstDeclInput& input) {
    std::string reason;
    if (supported_decl_constructor(expr, reason)) {
        return substitute_meta_input_tokens(*expr.macro_tokens, input_name, input.tokens);
    }
    if (expr.kind == ExprKind::If) {
        bool take_then = evaluate_decl_condition(*expr_if_condition(expr), input_name, input);
        return evaluate_decl_return_expr(
            take_then ? *expr_if_then_value(expr) : *expr_if_else_value(expr),
            input_name,
            input);
    }
    fail_eval(expr.loc, "internal error: unsupported ast declaration return expression");
}

struct DeclSummary {
    std::string kind;
    std::string name;
    bool is_public = false;
    std::size_t generic_count = 0;
    std::size_t param_count = 0;
    std::size_t field_count = 0;
    std::size_t case_count = 0;
    std::size_t method_count = 0;
    std::size_t associated_type_count = 0;
};

struct DeclShape {
    std::size_t generic_count = 0;
    std::size_t param_count = 0;
    std::size_t field_count = 0;
    std::size_t case_count = 0;
    std::size_t method_count = 0;
    std::size_t associated_type_count = 0;
};

void append_summary(std::vector<DeclSummary>& summaries,
                    std::string kind,
                    std::string name,
                    bool is_public,
                    DeclShape shape = {}) {
    summaries.push_back({
        std::move(kind),
        std::move(name),
        is_public,
        shape.generic_count,
        shape.param_count,
        shape.field_count,
        shape.case_count,
        shape.method_count,
        shape.associated_type_count
    });
}

void add_shape_to_input(MetaAstDeclInput& input, const DeclSummary& summary) {
    input.generic_count += summary.generic_count;
    input.param_count += summary.param_count;
    input.field_count += summary.field_count;
    input.case_count += summary.case_count;
    input.method_count += summary.method_count;
    input.associated_type_count += summary.associated_type_count;
}

} // namespace

MetaAstDeclInput summarize_meta_ast_decl_input(const std::vector<Token>& input_tokens,
                                               const Program& parsed_input) {
    std::vector<DeclSummary> summaries;
    for (const auto& decl : parsed_input.uses) {
        append_summary(summaries, "use", decl.alias.empty() ? qualified_basename(decl.path) : decl.alias, decl.is_public);
    }
    for (const auto& decl : parsed_input.module_imports) {
        append_summary(summaries, "module_import", qualified_basename(decl.name), decl.is_public);
    }
    for (const auto& decl : parsed_input.modules) {
        append_summary(summaries, "module", qualified_basename(decl.name), decl.is_public);
    }
    for (const auto& decl : parsed_input.item_macros) {
        append_summary(summaries, "item_macro", decl.name, decl.is_public);
    }
    for (const auto& decl : parsed_input.constants) {
        append_summary(summaries, "const", qualified_basename(decl.name), decl.is_public);
    }
    for (const auto& decl : parsed_input.functions) {
        append_summary(
            summaries,
            decl.meta ? "meta_fn" : "fn",
            qualified_basename(decl.name),
            decl.is_public,
            DeclShape{decl.generics.size(), decl.params.size()});
    }
    for (const auto& decl : parsed_input.structs) {
        append_summary(
            summaries,
            "struct",
            qualified_basename(decl.name),
            decl.is_public,
            DeclShape{decl.generics.size(), 0, decl.fields.size()});
    }
    for (const auto& decl : parsed_input.enums) {
        append_summary(
            summaries,
            "enum",
            qualified_basename(decl.name),
            decl.is_public,
            DeclShape{decl.generics.size(), 0, 0, decl.cases.size()});
    }
    for (const auto& decl : parsed_input.traits) {
        append_summary(
            summaries,
            "trait",
            qualified_basename(decl.name),
            decl.is_public,
            DeclShape{decl.generics.size(), 0, 0, 0, decl.methods.size(), decl.associated_types.size()});
    }
    for (const auto& decl : parsed_input.impls) {
        append_summary(
            summaries,
            "impl",
            type_ref_key(decl.for_type),
            decl.is_public,
            DeclShape{decl.generics.size(), 0, 0, 0, decl.methods.size(), decl.associated_type_witnesses.size()});
    }

    MetaAstDeclInput input;
    input.tokens = input_tokens;
    input.count = summaries.size();
    for (const auto& summary : summaries) {
        add_shape_to_input(input, summary);
    }
    if (summaries.empty()) {
        input.kind = "empty";
        return input;
    }
    if (summaries.size() == 1) {
        input.kind = summaries.front().kind;
        input.name = summaries.front().name;
        input.is_public = summaries.front().is_public;
        return input;
    }
    input.kind = "mixed";
    return input;
}

bool is_supported_meta_decl_return_expr(const Expr& expr,
                                        const std::string& input_name,
                                        std::string& reason) {
    return supported_decl_return_expr(expr, input_name, reason);
}

std::vector<Token> evaluate_meta_decl_return_expr(const Expr& expr,
                                                  const std::string& input_name,
                                                  const MetaAstDeclInput& input) {
    return evaluate_decl_return_expr(expr, input_name, input);
}

} // namespace ari
