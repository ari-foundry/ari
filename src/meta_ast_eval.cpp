#include "meta_ast_eval.hpp"

#include "common.hpp"
#include "meta_token_eval.hpp"

#include <cstdint>
#include <optional>
#include <string>
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

bool is_string_literal(const Expr& expr) {
    return expr.kind == ExprKind::String;
}

bool is_non_negative_integer_literal(const Expr& expr) {
    return expr.kind == ExprKind::Integer && !expr.int_negative;
}

bool is_free_decl_member_call(const Expr& expr,
                              const std::string& name,
                              const std::string& input_name,
                              std::size_t arg_count) {
    return is_free_decl_call(expr, name, input_name, arg_count);
}

bool is_decl_member_method_call(const Expr& expr,
                                const std::string& name,
                                const std::string& input_name,
                                std::size_t arg_count) {
    return is_decl_method_call(expr, name, input_name, arg_count);
}

const Expr* decl_member_arg(const Expr& expr, std::size_t index) {
    if (index >= expr.args.size()) return nullptr;
    return expr.args[index].get();
}

const Expr* decl_member_payload_arg(const Expr& expr, std::size_t free_index, std::size_t method_index) {
    return expr.kind == ExprKind::Call ? decl_member_arg(expr, free_index) : decl_member_arg(expr, method_index);
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

enum class DeclNamedBoolProperty {
    Generic,
    Param,
    Field,
    Case,
    Method,
    AssociatedType
};

struct DeclNamedBoolQuery {
    DeclNamedBoolProperty property;
    std::string name;
};

bool is_decl_named_bool_property_expr(const Expr& expr,
                                      const std::string& input_name,
                                      const std::string& free_name,
                                      const std::string& method_name) {
    if (!is_free_decl_member_call(expr, free_name, input_name, 2) &&
        !is_decl_member_method_call(expr, method_name, input_name, 1)) {
        return false;
    }
    const Expr* name_arg = decl_member_payload_arg(expr, 1, 0);
    return name_arg && is_string_literal(*name_arg);
}

std::optional<DeclNamedBoolQuery> decl_named_bool_property_expr(const Expr& expr,
                                                               const std::string& input_name) {
    struct Candidate {
        DeclNamedBoolProperty property;
        const char* free_name;
        const char* method_name;
    };
    static const Candidate candidates[] = {
        {DeclNamedBoolProperty::Generic, "decl_has_generic", "has_generic"},
        {DeclNamedBoolProperty::Param, "decl_has_param", "has_param"},
        {DeclNamedBoolProperty::Field, "decl_has_field", "has_field"},
        {DeclNamedBoolProperty::Case, "decl_has_case", "has_case"},
        {DeclNamedBoolProperty::Method, "decl_has_method", "has_method"},
        {DeclNamedBoolProperty::AssociatedType, "decl_has_associated_type", "has_associated_type"}
    };
    for (const auto& candidate : candidates) {
        if (!is_decl_named_bool_property_expr(expr, input_name, candidate.free_name, candidate.method_name)) {
            continue;
        }
        const Expr* name_arg = decl_member_payload_arg(expr, 1, 0);
        return DeclNamedBoolQuery{candidate.property, name_arg->string_value};
    }
    return std::nullopt;
}

enum class DeclStringProperty {
    ReturnType,
    TraitType,
    ParamType,
    FieldType,
    CasePayloadType,
    MethodReturnType,
    MethodParamType,
    AssociatedTypeType
};

struct DeclStringQuery {
    DeclStringProperty property;
    std::string name;
    std::string nested_name;
    std::size_t index = 0;
};

bool is_decl_zero_arg_string_property_expr(const Expr& expr,
                                           const std::string& input_name,
                                           const std::string& free_name,
                                           const std::string& method_name) {
    return is_free_decl_member_call(expr, free_name, input_name, 1) ||
           is_decl_member_method_call(expr, method_name, input_name, 0);
}

bool is_decl_named_string_property_expr(const Expr& expr,
                                        const std::string& input_name,
                                        const std::string& free_name,
                                        const std::string& method_name) {
    if (!is_free_decl_member_call(expr, free_name, input_name, 2) &&
        !is_decl_member_method_call(expr, method_name, input_name, 1)) {
        return false;
    }
    const Expr* name_arg = decl_member_payload_arg(expr, 1, 0);
    return name_arg && is_string_literal(*name_arg);
}

bool is_decl_nested_string_property_expr(const Expr& expr,
                                         const std::string& input_name,
                                         const std::string& free_name,
                                         const std::string& method_name) {
    if (!is_free_decl_member_call(expr, free_name, input_name, 3) &&
        !is_decl_member_method_call(expr, method_name, input_name, 2)) {
        return false;
    }
    const Expr* name_arg = decl_member_payload_arg(expr, 1, 0);
    const Expr* nested_arg = decl_member_payload_arg(expr, 2, 1);
    return name_arg && nested_arg && is_string_literal(*name_arg) && is_string_literal(*nested_arg);
}

bool is_decl_indexed_string_property_expr(const Expr& expr,
                                          const std::string& input_name,
                                          const std::string& free_name,
                                          const std::string& method_name) {
    if (!is_free_decl_member_call(expr, free_name, input_name, 3) &&
        !is_decl_member_method_call(expr, method_name, input_name, 2)) {
        return false;
    }
    const Expr* name_arg = decl_member_payload_arg(expr, 1, 0);
    const Expr* index_arg = decl_member_payload_arg(expr, 2, 1);
    return name_arg && index_arg && is_string_literal(*name_arg) && is_non_negative_integer_literal(*index_arg);
}

std::optional<DeclStringQuery> decl_string_property_expr(const Expr& expr,
                                                        const std::string& input_name) {
    if (is_decl_zero_arg_string_property_expr(expr, input_name, "decl_return_type", "return_type")) {
        return DeclStringQuery{DeclStringProperty::ReturnType, "", "", 0};
    }
    if (is_decl_zero_arg_string_property_expr(expr, input_name, "decl_trait_type", "trait_type")) {
        return DeclStringQuery{DeclStringProperty::TraitType, "", "", 0};
    }
    struct NamedCandidate {
        DeclStringProperty property;
        const char* free_name;
        const char* method_name;
    };
    static const NamedCandidate named_candidates[] = {
        {DeclStringProperty::ParamType, "decl_param_type", "param_type"},
        {DeclStringProperty::FieldType, "decl_field_type", "field_type"},
        {DeclStringProperty::MethodReturnType, "decl_method_return_type", "method_return_type"},
        {DeclStringProperty::AssociatedTypeType, "decl_associated_type_type", "associated_type_type"}
    };
    for (const auto& candidate : named_candidates) {
        if (!is_decl_named_string_property_expr(expr, input_name, candidate.free_name, candidate.method_name)) {
            continue;
        }
        const Expr* name_arg = decl_member_payload_arg(expr, 1, 0);
        return DeclStringQuery{candidate.property, name_arg->string_value, "", 0};
    }
    if (is_decl_indexed_string_property_expr(expr, input_name, "decl_case_payload_type", "case_payload_type")) {
        const Expr* name_arg = decl_member_payload_arg(expr, 1, 0);
        const Expr* index_arg = decl_member_payload_arg(expr, 2, 1);
        return DeclStringQuery{
            DeclStringProperty::CasePayloadType,
            name_arg->string_value,
            "",
            static_cast<std::size_t>(index_arg->int_value)
        };
    }
    if (is_decl_nested_string_property_expr(expr, input_name, "decl_method_param_type", "method_param_type")) {
        const Expr* name_arg = decl_member_payload_arg(expr, 1, 0);
        const Expr* nested_arg = decl_member_payload_arg(expr, 2, 1);
        return DeclStringQuery{
            DeclStringProperty::MethodParamType,
            name_arg->string_value,
            nested_arg->string_value,
            0
        };
    }
    return std::nullopt;
}

enum class DeclNamedIntProperty {
    CasePayloadCount,
    MethodGenericCount,
    MethodParamCount
};

struct DeclNamedIntQuery {
    DeclNamedIntProperty property;
    std::string name;
};

bool is_decl_named_int_property_expr(const Expr& expr,
                                     const std::string& input_name,
                                     const std::string& free_name,
                                     const std::string& method_name) {
    if (!is_free_decl_member_call(expr, free_name, input_name, 2) &&
        !is_decl_member_method_call(expr, method_name, input_name, 1)) {
        return false;
    }
    const Expr* name_arg = decl_member_payload_arg(expr, 1, 0);
    return name_arg && is_string_literal(*name_arg);
}

std::optional<DeclNamedIntQuery> decl_named_int_property_expr(const Expr& expr,
                                                             const std::string& input_name) {
    struct Candidate {
        DeclNamedIntProperty property;
        const char* free_name;
        const char* method_name;
    };
    static const Candidate candidates[] = {
        {DeclNamedIntProperty::CasePayloadCount, "decl_case_payload_count", "case_payload_count"},
        {DeclNamedIntProperty::MethodGenericCount, "decl_method_generic_count", "method_generic_count"},
        {DeclNamedIntProperty::MethodParamCount, "decl_method_param_count", "method_param_count"}
    };
    for (const auto& candidate : candidates) {
        if (!is_decl_named_int_property_expr(expr, input_name, candidate.free_name, candidate.method_name)) {
            continue;
        }
        const Expr* name_arg = decl_member_payload_arg(expr, 1, 0);
        return DeclNamedIntQuery{candidate.property, name_arg->string_value};
    }
    return std::nullopt;
}

bool is_decl_string_expr(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::String ||
           is_decl_kind_expr(expr, input_name) ||
           is_decl_name_expr(expr, input_name) ||
           decl_string_property_expr(expr, input_name).has_value();
}

bool is_decl_int_expr(const Expr& expr, const std::string& input_name) {
    return (expr.kind == ExprKind::Integer && !expr.int_negative) ||
           decl_int_property_expr(expr, input_name).has_value() ||
           decl_named_int_property_expr(expr, input_name).has_value();
}

std::string decl_condition_support_message() {
    return
        "ast declaration meta branch conditions currently support bool literals, !, &&, ||, declaration kind/name/visibility/count checks, named member predicates such as input.has_field(\"name\") and decl_has_method(input, \"name\"), type-summary string comparisons such as input.field_type(\"name\") and decl_case_payload_type(input, \"Case\", 0), and member integer comparisons such as input.case_payload_count(\"Case\") and input.method_param_count(\"method\")";
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
            if (is_decl_public_expr(expr, input_name) ||
                is_decl_is_expr(expr, input_name) ||
                decl_named_bool_property_expr(expr, input_name).has_value()) {
                return true;
            }
            break;
        default:
            break;
    }
    reason = decl_condition_support_message();
    return false;
}

bool has_name(const std::vector<std::string>& names, const std::string& name) {
    for (const auto& item : names) {
        if (item == name) return true;
    }
    return false;
}

const MetaAstNameTypeSummary* find_named_type(const std::vector<MetaAstNameTypeSummary>& items,
                                              const std::string& name) {
    for (const auto& item : items) {
        if (item.name == name) return &item;
    }
    return nullptr;
}

const MetaAstCallableSummary* find_method(const MetaAstDeclInput& input, const std::string& name) {
    for (const auto& method : input.methods) {
        if (method.name == name) return &method;
    }
    return nullptr;
}

const MetaAstEnumCaseSummary* find_case(const MetaAstDeclInput& input, const std::string& name) {
    for (const auto& item : input.cases) {
        if (item.name == name) return &item;
    }
    return nullptr;
}

bool evaluate_decl_named_bool_query(const DeclNamedBoolQuery& query, const MetaAstDeclInput& input) {
    switch (query.property) {
        case DeclNamedBoolProperty::Generic:
            return has_name(input.generics, query.name);
        case DeclNamedBoolProperty::Param:
            return find_named_type(input.params, query.name) != nullptr;
        case DeclNamedBoolProperty::Field:
            return find_named_type(input.fields, query.name) != nullptr;
        case DeclNamedBoolProperty::Case:
            return find_case(input, query.name) != nullptr;
        case DeclNamedBoolProperty::Method:
            return find_method(input, query.name) != nullptr;
        case DeclNamedBoolProperty::AssociatedType:
            return find_named_type(input.associated_types, query.name) != nullptr;
    }
    return false;
}

std::string evaluate_decl_string_query(const DeclStringQuery& query, const MetaAstDeclInput& input) {
    switch (query.property) {
        case DeclStringProperty::ReturnType:
            return input.return_type;
        case DeclStringProperty::TraitType:
            return input.trait_type;
        case DeclStringProperty::ParamType:
            if (const auto* item = find_named_type(input.params, query.name)) return item->type;
            return "";
        case DeclStringProperty::FieldType:
            if (const auto* item = find_named_type(input.fields, query.name)) return item->type;
            return "";
        case DeclStringProperty::CasePayloadType: {
            const auto* item = find_case(input, query.name);
            if (!item || query.index >= item->payload_types.size()) return "";
            return item->payload_types[query.index];
        }
        case DeclStringProperty::MethodReturnType:
            if (const auto* method = find_method(input, query.name)) return method->return_type;
            return "";
        case DeclStringProperty::MethodParamType:
            if (const auto* method = find_method(input, query.name)) {
                if (const auto* param = find_named_type(method->params, query.nested_name)) return param->type;
            }
            return "";
        case DeclStringProperty::AssociatedTypeType:
            if (const auto* item = find_named_type(input.associated_types, query.name)) return item->type;
            return "";
    }
    return "";
}

std::uint64_t evaluate_decl_named_int_query(const DeclNamedIntQuery& query, const MetaAstDeclInput& input) {
    switch (query.property) {
        case DeclNamedIntProperty::CasePayloadCount:
            if (const auto* item = find_case(input, query.name)) {
                return static_cast<std::uint64_t>(item->payload_types.size());
            }
            return 0;
        case DeclNamedIntProperty::MethodGenericCount:
            if (const auto* method = find_method(input, query.name)) {
                return static_cast<std::uint64_t>(method->generic_count);
            }
            return 0;
        case DeclNamedIntProperty::MethodParamCount:
            if (const auto* method = find_method(input, query.name)) {
                return static_cast<std::uint64_t>(method->params.size());
            }
            return 0;
    }
    return 0;
}

std::string evaluate_decl_string_expr(const Expr& expr,
                                      const std::string& input_name,
                                      const MetaAstDeclInput& input) {
    if (expr.kind == ExprKind::String) return expr.string_value;
    if (is_decl_kind_expr(expr, input_name)) return input.kind;
    if (is_decl_name_expr(expr, input_name)) return input.name;
    if (auto query = decl_string_property_expr(expr, input_name)) {
        return evaluate_decl_string_query(*query, input);
    }
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
    if (auto query = decl_named_int_property_expr(expr, input_name)) {
        return evaluate_decl_named_int_query(*query, input);
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
            if (auto query = decl_named_bool_property_expr(expr, input_name)) {
                return evaluate_decl_named_bool_query(*query, input);
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

} // namespace

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
