#include "module_ast_summary.hpp"

#include "common.hpp"
#include "module_cache_format.hpp"
#include "module_metadata.hpp"
#include "module_path.hpp"

#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace ari {
namespace {

[[noreturn]] void fail_invalid_module_cache_ast_summary(const std::string& display_path,
                                                        const std::string& detail) {
    CompileError error("invalid module cache '" + display_path + "': " + detail);
    error.add_note(DiagnosticNote{
        std::nullopt,
        "cached AST summaries are compiler-owned declaration payloads used to replay module surfaces safely",
        DiagnosticNoteKind::Note});
    error.add_note(DiagnosticNote{
        std::nullopt,
        "regenerate the module cache with --emit-module-cache after changing declarations or compiler cache format",
        DiagnosticNoteKind::Help});
    throw error;
}

void append_field(std::ostringstream& out, const std::string& value) {
    out << value.size() << ':' << value << ';';
}

void append_bool(std::ostringstream& out, bool value) {
    out << (value ? "1;" : "0;");
}

void append_count(std::ostringstream& out, std::uint64_t value) {
    out << value << ';';
}

std::uint64_t double_bits(double value) {
    static_assert(sizeof(double) == sizeof(std::uint64_t), "double payload must fit in a u64");
    std::uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

double double_from_bits(std::uint64_t bits) {
    static_assert(sizeof(double) == sizeof(std::uint64_t), "double payload must fit in a u64");
    double value = 0.0;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

void append_qualifier(std::ostringstream& out, TypeQualifier qualifier) {
    switch (qualifier) {
        case TypeQualifier::Value: append_field(out, "value"); return;
        case TypeQualifier::Own: append_field(out, "own"); return;
        case TypeQualifier::Ref: append_field(out, "ref"); return;
        case TypeQualifier::MutRef: append_field(out, "ref mut"); return;
        case TypeQualifier::Ptr: append_field(out, "ptr"); return;
    }
}

void append_binding_mode(std::ostringstream& out, BindingMode mode) {
    switch (mode) {
        case BindingMode::Value: append_field(out, "value"); return;
        case BindingMode::Ref: append_field(out, "ref"); return;
        case BindingMode::RefMut: append_field(out, "ref mut"); return;
    }
}

void append_token_payload(std::ostringstream& out, const Token& token);

void append_type(std::ostringstream& out, const TypeRef& type) {
    append_qualifier(out, type.qualifier);
    append_field(out, type.name);
    append_bool(out, type.is_dyn_object);
    append_bool(out, type.nullable);
    append_count(out, type.array_size);
    append_bool(out, type.is_macro_invocation);
    if (type.is_macro_invocation) {
        append_count(out, type.macro_tokens.size());
        for (const auto& token : type.macro_tokens) append_token_payload(out, token);
    }
    append_bool(out, type.has_associated_projection);
    if (type.has_associated_projection) append_field(out, type.associated_projection);
    append_bool(out, type.is_union_by);
    if (type.is_union_by) {
        append_count(out, type.union_by_selector.size());
        for (const auto& part : type.union_by_selector) append_field(out, part);
        append_count(out, type.union_by_arm_names.size());
        for (std::size_t i = 0; i < type.union_by_arm_names.size(); ++i) {
            append_field(out, type.union_by_arm_names[i]);
            if (i < type.union_by_arm_types.size()) append_type(out, type.union_by_arm_types[i]);
        }
    }
    append_count(out, type.args.size());
    for (const auto& arg : type.args) append_type(out, arg);
}

void append_generics(std::ostringstream& out, const std::vector<GenericParam>& generics) {
    append_count(out, generics.size());
    for (const auto& generic : generics) {
        append_field(out, generic.name);
        append_bool(out, generic.has_constraint);
        if (generic.has_constraint) append_type(out, generic.constraint);
    }
}

void append_attributes(std::ostringstream& out, const std::vector<Attribute>& attributes) {
    append_count(out, attributes.size());
    for (const auto& attr : attributes) {
        append_field(out, attr.name);
        append_bool(out, attr.has_args);
        append_count(out, attr.args.size());
        for (const auto& token : attr.args) append_token_payload(out, token);
    }
}

void append_item_macro_invocation(std::ostringstream& out, const ItemMacroInvocation& invocation) {
    append_field(out, invocation.module_name);
    append_field(out, invocation.name);
    append_bool(out, invocation.is_public);
    append_attributes(out, invocation.attributes);
    append_count(out, invocation.tokens.size());
    for (const auto& token : invocation.tokens) append_token_payload(out, token);
}

void append_token_payload(std::ostringstream& out, const Token& token) {
    append_count(out, static_cast<std::uint64_t>(token.kind));
    append_field(out, token.text);
    append_count(out, token.int_value);
    append_count(out, double_bits(token.float_value));
    append_field(out, token.literal_suffix);
}

void append_pattern_macro_payload(std::ostringstream& out, const Pattern& pattern) {
    append_bool(out, pattern.is_macro_invocation);
    if (!pattern.is_macro_invocation) return;
    append_count(out, pattern.macro_tokens.size());
    for (const auto& token : pattern.macro_tokens) append_token_payload(out, token);
}

void append_function_body_summary(std::ostringstream& out, const FunctionDecl& fn);
bool append_pattern_payload(std::ostringstream& out, const Pattern& pattern);

void append_function_signature(std::ostringstream& out, const FunctionDecl& fn) {
    append_field(out, fn.module_name);
    append_field(out, fn.name);
    append_bool(out, fn.meta);
    append_bool(out, fn.is_extern);
    append_bool(out, fn.is_public);
    append_bool(out, fn.is_variadic);
    append_field(out, fn.extern_abi);
    append_field(out, fn.extern_link_name);
    append_bool(out, fn.has_return_type);
    append_bool(out, fn.has_body);
    append_generics(out, fn.generics);
    append_attributes(out, fn.attributes);
    append_count(out, fn.params.size());
    for (const auto& param : fn.params) {
        append_field(out, param.name);
        append_bool(out, param.has_pattern);
        if (param.has_pattern) append_pattern_payload(out, param.pattern);
        append_type(out, param.type);
    }
    if (fn.has_return_type) append_type(out, fn.return_type);
    append_function_body_summary(out, fn);
}

bool is_summary_const_unary_op(TokenKind op) {
    return op == TokenKind::Bang || op == TokenKind::Minus || op == TokenKind::Star || op == TokenKind::Tilde;
}

bool is_summary_const_binary_op(TokenKind op) {
    switch (op) {
        case TokenKind::AmpAmp:
        case TokenKind::PipePipe:
        case TokenKind::EqEq:
        case TokenKind::BangEq:
        case TokenKind::Less:
        case TokenKind::LessEq:
        case TokenKind::Greater:
        case TokenKind::GreaterEq:
        case TokenKind::Plus:
        case TokenKind::Minus:
        case TokenKind::Star:
        case TokenKind::Slash:
        case TokenKind::Percent:
        case TokenKind::Amp:
        case TokenKind::Pipe:
        case TokenKind::Caret:
        case TokenKind::LessLess:
        case TokenKind::GreaterGreater:
            return true;
        default:
            return false;
    }
}

bool append_const_expr_list(std::ostringstream& out, const std::vector<ExprPtr>& args);
bool append_body_stmt_list(std::ostringstream& out, const std::vector<StmtPtr>& statements);

void append_type_arguments(std::ostringstream& out, const std::vector<TypeRef>& type_args) {
    append_count(out, type_args.size());
    for (const auto& type_arg : type_args) append_type(out, type_arg);
}

bool append_const_expr_payload(std::ostringstream& out, const Expr& expr) {
    switch (expr.kind) {
        case ExprKind::Integer:
            append_field(out, "integer");
            append_bool(out, expr.int_negative);
            append_count(out, expr.int_value);
            append_field(out, expr.literal_suffix);
            return true;
        case ExprKind::Float:
            append_field(out, "float");
            append_count(out, double_bits(expr.float_value));
            append_field(out, expr.literal_suffix);
            return true;
        case ExprKind::String:
            append_field(out, "string");
            append_field(out, expr.string_value);
            return true;
        case ExprKind::Bool:
            append_field(out, "bool");
            append_bool(out, expr.bool_value);
            return true;
        case ExprKind::Null:
            append_field(out, "null");
            return true;
        case ExprKind::Name:
            append_field(out, "name");
            append_field(out, expr.name);
            return true;
        case ExprKind::Borrow: {
            if (!expr_operand(expr)) return false;
            std::ostringstream operand;
            if (!append_const_expr_payload(operand, *expr_operand(expr))) return false;
            append_field(out, "borrow");
            append_bool(out, expr.mutable_borrow);
            out << operand.str();
            return true;
        }
        case ExprKind::Unary: {
            if (!expr_operand(expr) || !is_summary_const_unary_op(expr.op)) return false;
            std::ostringstream operand;
            if (!append_const_expr_payload(operand, *expr_operand(expr))) return false;
            append_field(out, "unary");
            append_count(out, static_cast<std::uint64_t>(expr.op));
            out << operand.str();
            return true;
        }
        case ExprKind::Try: {
            if (!expr_operand(expr)) return false;
            std::ostringstream operand;
            if (!append_const_expr_payload(operand, *expr_operand(expr))) return false;
            append_field(out, "try");
            out << operand.str();
            return true;
        }
        case ExprKind::NullCoalesce: {
            if (!expr_left(expr) || !expr_right(expr)) return false;
            std::ostringstream left;
            std::ostringstream right;
            if (!append_const_expr_payload(left, *expr_left(expr))) return false;
            if (!append_const_expr_payload(right, *expr_right(expr))) return false;
            append_field(out, "null-coalesce");
            out << left.str();
            out << right.str();
            return true;
        }
        case ExprKind::Binary: {
            if (!expr_left(expr) || !expr_right(expr) || !is_summary_const_binary_op(expr.op)) return false;
            std::ostringstream left;
            std::ostringstream right;
            if (!append_const_expr_payload(left, *expr_left(expr))) return false;
            if (!append_const_expr_payload(right, *expr_right(expr))) return false;
            append_field(out, "binary");
            append_count(out, static_cast<std::uint64_t>(expr.op));
            out << left.str();
            out << right.str();
            return true;
        }
        case ExprKind::Cast: {
            if (!expr_operand(expr)) return false;
            std::ostringstream operand;
            if (!append_const_expr_payload(operand, *expr_operand(expr))) return false;
            append_field(out, "cast");
            append_type(out, expr.cast_type);
            out << operand.str();
            return true;
        }
        case ExprKind::FieldAccess: {
            if (!expr_operand(expr)) return false;
            std::ostringstream operand;
            if (!append_const_expr_payload(operand, *expr_operand(expr))) return false;
            append_field(out, "field");
            out << operand.str();
            append_field(out, expr.name);
            return true;
        }
        case ExprKind::TupleIndex: {
            if (!expr_operand(expr)) return false;
            std::ostringstream operand;
            if (!append_const_expr_payload(operand, *expr_operand(expr))) return false;
            append_field(out, "tuple-index");
            out << operand.str();
            append_count(out, expr.tuple_index);
            return true;
        }
        case ExprKind::Index: {
            if (!expr_operand(expr) || !expr_right(expr)) return false;
            std::ostringstream operand;
            std::ostringstream index;
            if (!append_const_expr_payload(operand, *expr_operand(expr))) return false;
            if (!append_const_expr_payload(index, *expr_right(expr))) return false;
            append_field(out, "index");
            out << operand.str();
            out << index.str();
            return true;
        }
        case ExprKind::Tuple: {
            std::ostringstream args;
            if (!append_const_expr_list(args, expr.args)) return false;
            append_field(out, "tuple");
            out << args.str();
            return true;
        }
        case ExprKind::Vector: {
            std::ostringstream args;
            if (!append_const_expr_list(args, expr.args)) return false;
            append_field(out, "vector");
            out << args.str();
            return true;
        }
        case ExprKind::StructLiteral: {
            const ExprFieldNames& field_names = expr_field_names(expr);
            if (field_names.size() != expr.args.size()) return false;
            std::vector<std::ostringstream> values(expr.args.size());
            for (std::size_t i = 0; i < expr.args.size(); ++i) {
                if (!append_const_expr_payload(values[i], *expr.args[i])) return false;
            }
            const bool has_receiver_args = !expr_receiver_type_args(expr).empty();
            append_field(out, has_receiver_args ? "qualified-struct" : "struct");
            append_field(out, expr.name);
            if (has_receiver_args) append_type_arguments(out, expr_receiver_type_args(expr));
            append_type_arguments(out, expr_type_args(expr));
            append_count(out, expr.args.size());
            for (std::size_t i = 0; i < expr.args.size(); ++i) {
                append_field(out, field_names[i]);
                out << values[i].str();
            }
            return true;
        }
        case ExprKind::Call: {
            if (expr_operand(expr)) {
                if (!expr_receiver_type_args(expr).empty() || !expr_type_args(expr).empty()) return false;
                std::ostringstream operand;
                std::ostringstream args;
                if (!append_const_expr_payload(operand, *expr_operand(expr))) return false;
                if (!append_const_expr_list(args, expr.args)) return false;
                append_field(out, "indirect-call");
                out << operand.str();
                out << args.str();
                return true;
            }
            std::ostringstream args;
            if (!append_const_expr_list(args, expr.args)) return false;
            const bool has_receiver_args = !expr_receiver_type_args(expr).empty();
            append_field(out, has_receiver_args ? "qualified-call" : "call");
            append_field(out, expr.name);
            if (has_receiver_args) append_type_arguments(out, expr_receiver_type_args(expr));
            append_type_arguments(out, expr_type_args(expr));
            out << args.str();
            return true;
        }
        case ExprKind::MethodCall: {
            if (!expr_operand(expr)) return false;
            std::ostringstream operand;
            std::ostringstream args;
            if (!append_const_expr_payload(operand, *expr_operand(expr))) return false;
            if (!append_const_expr_list(args, expr.args)) return false;
            const bool has_receiver_args = !expr_receiver_type_args(expr).empty();
            append_field(out, has_receiver_args ? "qualified-method-call" : "method-call");
            out << operand.str();
            append_field(out, expr.name);
            if (has_receiver_args) append_type_arguments(out, expr_receiver_type_args(expr));
            append_type_arguments(out, expr_type_args(expr));
            out << args.str();
            return true;
        }
        case ExprKind::MacroCall: {
            if (!expr.macro_tokens) return false;
            append_field(out, "macro-call");
            append_field(out, expr.name);
            append_count(out, expr.macro_tokens->size());
            for (const auto& token : *expr.macro_tokens) append_token_payload(out, token);
            return true;
        }
        case ExprKind::If: {
            if (!expr_if_condition(expr) || !expr_if_then_value(expr) || !expr_if_else_value(expr)) return false;
            std::ostringstream pattern;
            std::ostringstream condition;
            std::ostringstream then_body;
            std::ostringstream then_value;
            std::ostringstream else_body;
            std::ostringstream else_value;
            const bool has_pattern = expr_if_has_condition_pattern(expr);
            if (has_pattern && !append_pattern_payload(pattern, *expr_if_condition_pattern(expr))) return false;
            if (!append_const_expr_payload(condition, *expr_if_condition(expr))) return false;
            if (!append_body_stmt_list(then_body, expr_if_then_body(expr))) return false;
            if (!append_const_expr_payload(then_value, *expr_if_then_value(expr))) return false;
            if (!append_body_stmt_list(else_body, expr_if_else_body(expr))) return false;
            if (!append_const_expr_payload(else_value, *expr_if_else_value(expr))) return false;
            append_field(out, "if-expr");
            append_bool(out, has_pattern);
            out << pattern.str();
            out << condition.str();
            out << then_body.str();
            out << then_value.str();
            out << else_body.str();
            out << else_value.str();
            return true;
        }
        case ExprKind::Block: {
            if (!expr_block_value(expr)) return false;
            std::ostringstream body;
            std::ostringstream value;
            if (!append_body_stmt_list(body, expr_block_body(expr))) return false;
            if (!append_const_expr_payload(value, *expr_block_value(expr))) return false;
            append_field(out, "block-expr");
            append_field(out, expr_block_label(expr));
            out << body.str();
            out << value.str();
            return true;
        }
        case ExprKind::Lambda: {
            std::ostringstream body;
            if (!append_body_stmt_list(body, expr_lambda_body(expr))) return false;
            std::ostringstream value;
            const bool has_value = static_cast<bool>(expr_lambda_value(expr));
            if (has_value && !append_const_expr_payload(value, *expr_lambda_value(expr))) return false;
            append_field(out, "lambda");
            append_count(out, expr_lambda_params(expr).size());
            for (const Param& param : expr_lambda_params(expr)) {
                append_field(out, param.name);
                append_bool(out, param.has_pattern);
                if (param.has_pattern) append_pattern_payload(out, param.pattern);
                append_binding_mode(out, param.binding_mode);
                append_type(out, param.type);
            }
            append_bool(out, expr_lambda_has_result_type(expr));
            if (expr_lambda_has_result_type(expr)) append_type(out, expr_lambda_result_type(expr));
            out << body.str();
            append_bool(out, has_value);
            if (has_value) out << value.str();
            return true;
        }
        case ExprKind::Match: {
            if (!expr_match_value(expr)) return false;
            std::ostringstream value;
            if (!append_const_expr_payload(value, *expr_match_value(expr))) return false;
            std::vector<std::ostringstream> arm_patterns(expr_match_arms(expr).size());
            std::vector<std::ostringstream> arm_values(expr_match_arms(expr).size());
            for (std::size_t i = 0; i < expr_match_arms(expr).size(); ++i) {
                const ExprMatchArm& arm = expr_match_arms(expr)[i];
                if (!append_pattern_payload(arm_patterns[i], arm.pattern)) return false;
                if (!arm.value || !append_const_expr_payload(arm_values[i], *arm.value)) return false;
            }
            append_field(out, "match-expr");
            out << value.str();
            append_count(out, expr_match_arms(expr).size());
            for (std::size_t i = 0; i < expr_match_arms(expr).size(); ++i) {
                out << arm_patterns[i].str();
                out << arm_values[i].str();
            }
            return true;
        }
        default:
            return false;
    }
}

bool append_const_expr_list(std::ostringstream& out, const std::vector<ExprPtr>& args) {
    std::vector<std::ostringstream> payloads(args.size());
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (!args[i] || !append_const_expr_payload(payloads[i], *args[i])) return false;
    }
    append_count(out, args.size());
    for (const auto& payload : payloads) out << payload.str();
    return true;
}

bool append_pattern_payload(std::ostringstream& out, const Pattern& pattern) {
    switch (pattern.kind) {
        case PatternKind::Wildcard:
            append_field(out, "wildcard");
            append_pattern_macro_payload(out, pattern);
            append_binding_mode(out, pattern.binding_mode);
            return true;
        case PatternKind::Binding:
            append_field(out, "binding");
            append_pattern_macro_payload(out, pattern);
            append_binding_mode(out, pattern.binding_mode);
            append_field(out, pattern.payload_name);
            return true;
        case PatternKind::IntegerLiteral:
            append_field(out, "integer");
            append_pattern_macro_payload(out, pattern);
            append_binding_mode(out, pattern.binding_mode);
            append_bool(out, pattern.int_negative);
            append_count(out, pattern.int_value);
            append_field(out, pattern.literal_suffix);
            return true;
        case PatternKind::BoolLiteral:
            append_field(out, "bool");
            append_pattern_macro_payload(out, pattern);
            append_binding_mode(out, pattern.binding_mode);
            append_bool(out, pattern.bool_value);
            return true;
        case PatternKind::Range:
            append_field(out, "range");
            append_pattern_macro_payload(out, pattern);
            append_binding_mode(out, pattern.binding_mode);
            append_bool(out, pattern.int_negative);
            append_count(out, pattern.int_value);
            append_field(out, pattern.literal_suffix);
            append_bool(out, pattern.range_inclusive);
            append_bool(out, pattern.range_end_negative);
            append_count(out, pattern.range_end_value);
            append_field(out, pattern.range_end_suffix);
            return true;
        case PatternKind::EnumCase: {
            append_field(out, "enum");
            append_pattern_macro_payload(out, pattern);
            append_binding_mode(out, pattern.binding_mode);
            append_field(out, pattern.case_name);
            append_bool(out, pattern.has_payload_pattern);
            append_bool(out, pattern.has_payload_binding);
            append_field(out, pattern.payload_name);
            if (pattern.has_payload_pattern) {
                if (!pattern.payload_pattern) return false;
                if (!append_pattern_payload(out, *pattern.payload_pattern)) return false;
            }
            return true;
        }
        case PatternKind::Or:
            append_field(out, "or");
            append_pattern_macro_payload(out, pattern);
            append_binding_mode(out, pattern.binding_mode);
            append_count(out, pattern.alternatives.size());
            for (const auto& alternative : pattern.alternatives) {
                if (!append_pattern_payload(out, alternative)) return false;
            }
            return true;
        case PatternKind::Alias:
            if (!pattern.alias_pattern) return false;
            append_field(out, "alias");
            append_pattern_macro_payload(out, pattern);
            append_binding_mode(out, pattern.binding_mode);
            append_field(out, pattern.alias_name);
            return append_pattern_payload(out, *pattern.alias_pattern);
        case PatternKind::Tuple:
        case PatternKind::Array: {
            if (pattern.kind == PatternKind::Array && !pattern.rest_alias_name.empty()) {
                append_field(out, "array-rest-alias");
            } else {
                append_field(out, pattern.kind == PatternKind::Tuple ? "tuple" : "array");
            }
            append_pattern_macro_payload(out, pattern);
            append_binding_mode(out, pattern.binding_mode);
            append_bool(out, pattern.has_rest);
            append_count(out, pattern.rest_index);
            if (pattern.kind == PatternKind::Array && !pattern.rest_alias_name.empty()) {
                append_field(out, pattern.rest_alias_name);
            }
            append_count(out, pattern.elements.size());
            for (const auto& element : pattern.elements) {
                if (!append_pattern_payload(out, element)) return false;
            }
            return true;
        }
        case PatternKind::Struct:
            if (pattern.field_names.size() != pattern.elements.size()) return false;
            append_field(out, "struct");
            append_pattern_macro_payload(out, pattern);
            append_binding_mode(out, pattern.binding_mode);
            append_field(out, pattern.case_name);
            append_bool(out, pattern.has_rest);
            append_count(out, pattern.rest_index);
            append_count(out, pattern.elements.size());
            for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
                append_field(out, pattern.field_names[i]);
                if (!append_pattern_payload(out, pattern.elements[i])) return false;
            }
            return true;
    }
    return false;
}

void append_const_initializer(std::ostringstream& out, const ExprPtr& init) {
    if (!init) {
        append_bool(out, false);
        return;
    }
    std::ostringstream payload;
    if (!append_const_expr_payload(payload, *init)) {
        append_bool(out, false);
        return;
    }
    append_bool(out, true);
    out << payload.str();
}

bool append_body_stmt_payload(std::ostringstream& out, const Stmt& stmt);

bool append_body_stmt_list(std::ostringstream& out, const std::vector<StmtPtr>& statements) {
    std::vector<std::ostringstream> payloads(statements.size());
    for (std::size_t i = 0; i < statements.size(); ++i) {
        if (!statements[i] || !append_body_stmt_payload(payloads[i], *statements[i])) return false;
    }
    append_count(out, statements.size());
    for (const auto& payload : payloads) out << payload.str();
    return true;
}

bool append_body_stmt_payload(std::ostringstream& out, const Stmt& stmt) {
    switch (stmt.kind) {
        case StmtKind::Block: {
            std::ostringstream body;
            if (!append_body_stmt_list(body, stmt_statements(stmt))) return false;
            append_field(out, "block");
            append_field(out, stmt_label(stmt));
            out << body.str();
            return true;
        }
        case StmtKind::VarDecl: {
            std::ostringstream pattern;
            if (stmt.binding.has_pattern && !append_pattern_payload(pattern, stmt.binding.pattern)) return false;
            std::ostringstream init;
            if (stmt.binding.init && !append_const_expr_payload(init, *stmt.binding.init)) return false;
            append_field(out, "var");
            append_field(out, stmt.binding.name);
            append_bool(out, stmt.binding.mutable_binding);
            append_binding_mode(out, stmt.binding.binding_mode);
            append_bool(out, stmt.binding.has_pattern);
            out << pattern.str();
            append_bool(out, stmt.binding.has_type);
            if (stmt.binding.has_type) append_type(out, stmt.binding.type);
            append_bool(out, static_cast<bool>(stmt.binding.init));
            out << init.str();
            return true;
        }
        case StmtKind::Assign: {
            const bool has_target = static_cast<bool>(stmt_assign_target(stmt));
            if (!stmt_assign_rhs(stmt)) return false;
            std::ostringstream target;
            std::ostringstream rhs;
            if (has_target && !append_const_expr_payload(target, *stmt_assign_target(stmt))) return false;
            if (!append_const_expr_payload(rhs, *stmt_assign_rhs(stmt))) return false;
            append_field(out, "assign");
            append_field(out, stmt_assign_name(stmt));
            append_bool(out, has_target);
            out << target.str();
            out << rhs.str();
            return true;
        }
        case StmtKind::Return: {
            std::ostringstream value;
            const bool has_value = static_cast<bool>(stmt.expr);
            if (has_value && !append_const_expr_payload(value, *stmt.expr)) return false;
            append_field(out, "return");
            append_bool(out, has_value);
            out << value.str();
            return true;
        }
        case StmtKind::ExprStmt: {
            if (!stmt.expr) return false;
            std::ostringstream value;
            if (!append_const_expr_payload(value, *stmt.expr)) return false;
            append_field(out, "expr-stmt");
            out << value.str();
            return true;
        }
        case StmtKind::If: {
            if (!stmt.condition) return false;
            if (stmt.has_condition_pattern || stmt.condition_pattern) {
                if (!stmt.has_condition_pattern || !stmt.condition_pattern) return false;
                std::ostringstream pattern;
                std::ostringstream condition;
                std::ostringstream then_body;
                std::ostringstream else_body;
                if (!append_pattern_payload(pattern, *stmt.condition_pattern)) return false;
                if (!append_const_expr_payload(condition, *stmt.condition)) return false;
                if (!append_body_stmt_list(then_body, stmt_then_body(stmt))) return false;
                if (!append_body_stmt_list(else_body, stmt_else_body(stmt))) return false;
                append_field(out, "if-let");
                out << pattern.str();
                out << condition.str();
                out << then_body.str();
                out << else_body.str();
                return true;
            }
            std::ostringstream condition;
            std::ostringstream then_body;
            std::ostringstream else_body;
            if (!append_const_expr_payload(condition, *stmt.condition)) return false;
            if (!append_body_stmt_list(then_body, stmt_then_body(stmt))) return false;
            if (!append_body_stmt_list(else_body, stmt_else_body(stmt))) return false;
            append_field(out, "if");
            out << condition.str();
            out << then_body.str();
            out << else_body.str();
            return true;
        }
        case StmtKind::Break: {
            std::ostringstream value;
            const bool has_value = static_cast<bool>(stmt_break_value(stmt));
            if (has_value && !append_const_expr_payload(value, *stmt_break_value(stmt))) return false;
            append_field(out, "break");
            append_field(out, stmt_break_label(stmt));
            append_bool(out, has_value);
            out << value.str();
            return true;
        }
        case StmtKind::While: {
            if (!stmt.condition || stmt.has_condition_pattern || stmt.condition_pattern) return false;
            std::ostringstream condition;
            std::ostringstream body;
            if (!append_const_expr_payload(condition, *stmt.condition)) return false;
            if (!append_body_stmt_list(body, stmt_loop_body(stmt))) return false;
            append_field(out, "while");
            out << condition.str();
            out << body.str();
            return true;
        }
        case StmtKind::WhileLet: {
            if (!stmt.condition || !stmt.has_condition_pattern || !stmt.condition_pattern) return false;
            std::ostringstream pattern;
            std::ostringstream condition;
            std::ostringstream body;
            if (!append_pattern_payload(pattern, *stmt.condition_pattern)) return false;
            if (!append_const_expr_payload(condition, *stmt.condition)) return false;
            if (!append_body_stmt_list(body, stmt_loop_body(stmt))) return false;
            append_field(out, "while-let");
            out << pattern.str();
            out << condition.str();
            out << body.str();
            return true;
        }
        case StmtKind::For: {
            if (!stmt.for_pattern || !stmt.for_iterable) return false;
            std::ostringstream pattern;
            std::ostringstream iterable;
            std::ostringstream body;
            if (!append_pattern_payload(pattern, *stmt.for_pattern)) return false;
            if (!append_const_expr_payload(iterable, *stmt.for_iterable)) return false;
            if (!append_body_stmt_list(body, stmt_loop_body(stmt))) return false;
            append_field(out, "for");
            append_bool(out, stmt.for_pattern_filter);
            out << pattern.str();
            out << iterable.str();
            out << body.str();
            return true;
        }
        case StmtKind::InitWhile: {
            std::vector<std::ostringstream> initializers(stmt.init_bindings.size());
            std::vector<std::ostringstream> updates(stmt.updates.size());
            for (std::size_t i = 0; i < stmt.init_bindings.size(); ++i) {
                if (!stmt.init_bindings[i].init ||
                    !append_const_expr_payload(initializers[i], *stmt.init_bindings[i].init)) {
                    return false;
                }
            }
            for (std::size_t i = 0; i < stmt.updates.size(); ++i) {
                if (!stmt.updates[i] || !append_const_expr_payload(updates[i], *stmt.updates[i])) return false;
            }
            if (!stmt.condition) return false;
            std::ostringstream condition;
            std::ostringstream body;
            if (!append_const_expr_payload(condition, *stmt.condition)) return false;
            if (!append_body_stmt_list(body, stmt_loop_body(stmt))) return false;
            append_field(out, "init-while");
            append_count(out, stmt.init_bindings.size());
            for (std::size_t i = 0; i < stmt.init_bindings.size(); ++i) {
                const Binding& binding = stmt.init_bindings[i];
                std::ostringstream pattern;
                if (binding.has_pattern && !append_pattern_payload(pattern, binding.pattern)) return false;
                append_field(out, binding.name);
                append_bool(out, binding.mutable_binding);
                append_binding_mode(out, binding.binding_mode);
                append_bool(out, binding.has_pattern);
                out << pattern.str();
                append_bool(out, binding.has_type);
                if (binding.has_type) append_type(out, binding.type);
                out << initializers[i].str();
            }
            out << condition.str();
            out << body.str();
            append_count(out, updates.size());
            for (const auto& update : updates) out << update.str();
            return true;
        }
        case StmtKind::Continue: {
            std::ostringstream updates;
            if (!append_const_expr_list(updates, stmt.updates)) return false;
            append_field(out, "continue");
            out << updates.str();
            return true;
        }
        case StmtKind::Match: {
            if (!stmt.match_value) return false;
            std::ostringstream value;
            if (!append_const_expr_payload(value, *stmt.match_value)) return false;
            std::vector<std::ostringstream> arm_patterns(stmt_match_arms(stmt).size());
            std::vector<std::ostringstream> arm_bodies(stmt_match_arms(stmt).size());
            for (std::size_t i = 0; i < stmt_match_arms(stmt).size(); ++i) {
                const MatchArm& arm = stmt_match_arms(stmt)[i];
                if (!append_pattern_payload(arm_patterns[i], arm.pattern)) return false;
                if (!append_body_stmt_list(arm_bodies[i], arm.body)) return false;
            }
            append_field(out, "match");
            out << value.str();
            append_count(out, stmt_match_arms(stmt).size());
            for (std::size_t i = 0; i < stmt_match_arms(stmt).size(); ++i) {
                out << arm_patterns[i].str();
                out << arm_bodies[i].str();
            }
            return true;
        }
        case StmtKind::Drop:
            append_field(out, "drop");
            append_field(out, stmt_drop_name(stmt));
            return true;
        case StmtKind::Forget:
            append_field(out, "forget");
            append_field(out, stmt_drop_name(stmt));
            return true;
        default:
            return false;
    }
}

void append_function_body_summary(std::ostringstream& out, const FunctionDecl& fn) {
    if (!fn.has_body || fn.is_extern) {
        append_bool(out, false);
        return;
    }

    std::ostringstream body;
    if (!append_body_stmt_list(body, fn.body)) {
        append_bool(out, false);
        return;
    }

    append_bool(out, true);
    out << body.str();
}

struct DeclarationSummaryCounts {
    std::uint64_t use_count = 0;
    std::uint64_t module_import_count = 0;
    std::uint64_t module_decl_count = 0;
    std::uint64_t item_macro_count = 0;
    std::uint64_t constant_count = 0;
    std::uint64_t type_alias_count = 0;
    std::uint64_t function_count = 0;
    std::uint64_t struct_count = 0;
    std::uint64_t enum_count = 0;
    std::uint64_t trait_count = 0;
    std::uint64_t impl_count = 0;
};

std::string summary_display(const ModuleCacheAstSummary& summary) {
    return "'" + (summary.module_name.empty() ? "<root>" : summary.module_name) +
           "' at '" + summary.path + "'";
}

class DeclarationSummaryReader {
public:
    DeclarationSummaryReader(const std::string& text, std::string display)
        : text_(text), display_(std::move(display)) {}

    DeclarationSummaryCounts parse() {
        consume_header();
        DeclarationSummaryCounts counts;

        counts.use_count = read_count("use count");
        for (std::uint64_t i = 0; i < counts.use_count; ++i) {
            read_field("use module name");
            read_field("use path");
            read_field("use alias");
            read_bool("use visibility");
            read_bool("use glob flag");
        }

        counts.module_import_count = read_count("module import count");
        for (std::uint64_t i = 0; i < counts.module_import_count; ++i) {
            read_field("module import owner");
            read_field("module import name");
            read_field("module import local name");
            read_bool("module import visibility");
        }

        counts.module_decl_count = read_count("module declaration count");
        for (std::uint64_t i = 0; i < counts.module_decl_count; ++i) {
            read_field("module declaration owner");
            read_field("module declaration name");
            read_bool("module declaration visibility");
        }

        counts.item_macro_count = read_count("item macro invocation count");
        for (std::uint64_t i = 0; i < counts.item_macro_count; ++i) {
            read_field("item macro module name");
            read_field("item macro name");
            read_bool("item macro visibility");
            skip_attributes();
            std::uint64_t token_count = read_count("item macro token count");
            for (std::uint64_t j = 0; j < token_count; ++j) {
                (void)read_token_payload("item macro token");
            }
        }

        counts.constant_count = read_count("constant count");
        for (std::uint64_t i = 0; i < counts.constant_count; ++i) {
            read_field("constant module name");
            read_field("constant name");
            read_bool("constant visibility");
            skip_type("constant type");
            skip_const_initializer("constant initializer");
        }

        counts.type_alias_count = read_count("type alias count");
        for (std::uint64_t i = 0; i < counts.type_alias_count; ++i) {
            read_field("type alias module name");
            read_field("type alias name");
            read_bool("type alias visibility");
            skip_generics();
            skip_type("type alias target");
        }

        counts.function_count = read_count("function count");
        for (std::uint64_t i = 0; i < counts.function_count; ++i) skip_function_signature();

        counts.struct_count = read_count("struct count");
        for (std::uint64_t i = 0; i < counts.struct_count; ++i) {
            read_field("struct module name");
            read_field("struct name");
            read_bool("struct visibility");
            read_bool("tuple struct flag");
            skip_generics();
            skip_attributes();
            std::uint64_t field_count = read_count("struct field count");
            for (std::uint64_t j = 0; j < field_count; ++j) {
                read_field("struct field name");
                read_bool("struct field mutability");
                skip_type("struct field type");
            }
        }

        counts.enum_count = read_count("enum count");
        for (std::uint64_t i = 0; i < counts.enum_count; ++i) {
            read_field("enum module name");
            read_field("enum name");
            read_bool("enum visibility");
            skip_generics();
            skip_attributes();
            std::uint64_t case_count = read_count("enum case count");
            for (std::uint64_t j = 0; j < case_count; ++j) {
                read_field("enum case name");
                std::uint64_t payload_count = read_count("enum payload count");
                for (std::uint64_t k = 0; k < payload_count; ++k) skip_type("enum payload type");
            }
        }

        counts.trait_count = read_count("trait count");
        for (std::uint64_t i = 0; i < counts.trait_count; ++i) {
            read_field("trait module name");
            read_field("trait name");
            read_bool("trait visibility");
            skip_generics();
            skip_attributes();
            std::uint64_t associated_type_count = read_count("trait associated type count");
            for (std::uint64_t j = 0; j < associated_type_count; ++j) {
                read_field("trait associated type name");
            }
            std::uint64_t method_count = read_count("trait method count");
            for (std::uint64_t j = 0; j < method_count; ++j) skip_function_signature();
        }

        counts.impl_count = read_count("impl count");
        for (std::uint64_t i = 0; i < counts.impl_count; ++i) {
            read_field("impl module name");
            read_bool("impl visibility");
            bool has_trait = read_bool("impl trait flag");
            skip_generics();
            skip_attributes();
            if (has_trait) skip_type("impl trait type");
            skip_type("impl target type");
            std::uint64_t associated_type_witness_count = read_count("impl associated type witness count");
            for (std::uint64_t j = 0; j < associated_type_witness_count; ++j) {
                read_field("impl associated type witness name");
                skip_type("impl associated type witness type");
            }
            std::uint64_t method_count = read_count("impl method count");
            for (std::uint64_t j = 0; j < method_count; ++j) skip_function_signature();
        }

        if (pos_ != text_.size()) fail("trailing bytes after declaration summary");
        return counts;
    }

    Program parse_program() {
        consume_header();
        Program program;

        std::uint64_t use_count = read_count("use count");
        program.uses.reserve(static_cast<std::size_t>(use_count));
        for (std::uint64_t i = 0; i < use_count; ++i) {
            UseDecl decl;
            decl.module_name = read_field("use module name");
            decl.path = read_field("use path");
            decl.alias = read_field("use alias");
            decl.is_public = read_bool("use visibility");
            decl.is_glob = read_bool("use glob flag");
            decl.loc = default_loc();
            program.uses.push_back(std::move(decl));
        }

        std::uint64_t import_count = read_count("module import count");
        program.module_imports.reserve(static_cast<std::size_t>(import_count));
        for (std::uint64_t i = 0; i < import_count; ++i) {
            ModuleImport decl;
            decl.module_name = read_field("module import owner");
            decl.name = read_field("module import name");
            decl.local_name = read_field("module import local name");
            decl.is_public = read_bool("module import visibility");
            decl.loc = default_loc();
            program.module_imports.push_back(std::move(decl));
        }

        std::uint64_t module_count = read_count("module declaration count");
        program.modules.reserve(static_cast<std::size_t>(module_count));
        for (std::uint64_t i = 0; i < module_count; ++i) {
            ModuleDecl decl;
            decl.module_name = read_field("module declaration owner");
            decl.name = read_field("module declaration name");
            decl.is_public = read_bool("module declaration visibility");
            decl.loc = default_loc();
            program.modules.push_back(std::move(decl));
        }

        std::uint64_t item_macro_count = read_count("item macro invocation count");
        program.item_macros.reserve(static_cast<std::size_t>(item_macro_count));
        for (std::uint64_t i = 0; i < item_macro_count; ++i) {
            ItemMacroInvocation invocation;
            invocation.module_name = read_field("item macro module name");
            invocation.name = read_field("item macro name");
            invocation.is_public = read_bool("item macro visibility");
            invocation.attributes = read_attributes();
            std::uint64_t token_count = read_count("item macro token count");
            invocation.tokens.reserve(static_cast<std::size_t>(token_count));
            for (std::uint64_t j = 0; j < token_count; ++j) {
                invocation.tokens.push_back(read_token_payload("item macro token"));
            }
            invocation.loc = default_loc();
            program.item_macros.push_back(std::move(invocation));
        }

        std::uint64_t constant_count = read_count("constant count");
        program.constants.reserve(static_cast<std::size_t>(constant_count));
        for (std::uint64_t i = 0; i < constant_count; ++i) {
            ConstDecl decl;
            decl.module_name = read_field("constant module name");
            decl.name = read_field("constant name");
            decl.is_public = read_bool("constant visibility");
            decl.type = read_type("constant type");
            decl.init = read_const_initializer("constant initializer");
            decl.loc = default_loc();
            program.constants.push_back(std::move(decl));
        }

        std::uint64_t type_alias_count = read_count("type alias count");
        program.type_aliases.reserve(static_cast<std::size_t>(type_alias_count));
        for (std::uint64_t i = 0; i < type_alias_count; ++i) {
            TypeAliasDecl decl;
            decl.module_name = read_field("type alias module name");
            decl.name = read_field("type alias name");
            decl.is_public = read_bool("type alias visibility");
            decl.generics = read_generics();
            decl.target = read_type("type alias target");
            decl.loc = default_loc();
            program.type_aliases.push_back(std::move(decl));
        }

        std::uint64_t function_count = read_count("function count");
        program.functions.reserve(static_cast<std::size_t>(function_count));
        for (std::uint64_t i = 0; i < function_count; ++i) {
            program.functions.push_back(read_function_signature());
        }

        std::uint64_t struct_count = read_count("struct count");
        program.structs.reserve(static_cast<std::size_t>(struct_count));
        for (std::uint64_t i = 0; i < struct_count; ++i) {
            StructDecl decl;
            decl.module_name = read_field("struct module name");
            decl.name = read_field("struct name");
            decl.is_public = read_bool("struct visibility");
            decl.tuple_struct = read_bool("tuple struct flag");
            decl.generics = read_generics();
            decl.attributes = read_attributes();
            std::uint64_t field_count = read_count("struct field count");
            decl.fields.reserve(static_cast<std::size_t>(field_count));
            for (std::uint64_t j = 0; j < field_count; ++j) {
                StructField field;
                field.name = read_field("struct field name");
                field.mutable_field = read_bool("struct field mutability");
                field.type = read_type("struct field type");
                field.loc = default_loc();
                decl.fields.push_back(std::move(field));
            }
            decl.loc = default_loc();
            program.structs.push_back(std::move(decl));
        }

        std::uint64_t enum_count = read_count("enum count");
        program.enums.reserve(static_cast<std::size_t>(enum_count));
        for (std::uint64_t i = 0; i < enum_count; ++i) {
            EnumDecl decl;
            decl.module_name = read_field("enum module name");
            decl.name = read_field("enum name");
            decl.is_public = read_bool("enum visibility");
            decl.generics = read_generics();
            decl.attributes = read_attributes();
            std::uint64_t case_count = read_count("enum case count");
            decl.cases.reserve(static_cast<std::size_t>(case_count));
            for (std::uint64_t j = 0; j < case_count; ++j) {
                EnumCase item;
                item.name = read_field("enum case name");
                std::uint64_t payload_count = read_count("enum payload count");
                item.payloads.reserve(static_cast<std::size_t>(payload_count));
                for (std::uint64_t k = 0; k < payload_count; ++k) {
                    item.payloads.push_back(read_type("enum payload type"));
                }
                item.loc = default_loc();
                decl.cases.push_back(std::move(item));
            }
            decl.loc = default_loc();
            program.enums.push_back(std::move(decl));
        }

        std::uint64_t trait_count = read_count("trait count");
        program.traits.reserve(static_cast<std::size_t>(trait_count));
        for (std::uint64_t i = 0; i < trait_count; ++i) {
            TraitDecl decl;
            decl.module_name = read_field("trait module name");
            decl.name = read_field("trait name");
            decl.is_public = read_bool("trait visibility");
            decl.generics = read_generics();
            decl.attributes = read_attributes();
            std::uint64_t associated_type_count = read_count("trait associated type count");
            decl.associated_types.reserve(static_cast<std::size_t>(associated_type_count));
            for (std::uint64_t j = 0; j < associated_type_count; ++j) {
                TraitDecl::AssociatedType associated_type;
                associated_type.name = read_field("trait associated type name");
                associated_type.loc = default_loc();
                decl.associated_types.push_back(std::move(associated_type));
            }
            std::uint64_t method_count = read_count("trait method count");
            decl.methods.reserve(static_cast<std::size_t>(method_count));
            for (std::uint64_t j = 0; j < method_count; ++j) {
                decl.methods.push_back(read_function_signature());
            }
            decl.loc = default_loc();
            program.traits.push_back(std::move(decl));
        }

        std::uint64_t impl_count = read_count("impl count");
        program.impls.reserve(static_cast<std::size_t>(impl_count));
        for (std::uint64_t i = 0; i < impl_count; ++i) {
            ImplDecl decl;
            decl.module_name = read_field("impl module name");
            decl.is_public = read_bool("impl visibility");
            decl.has_trait = read_bool("impl trait flag");
            decl.generics = read_generics();
            decl.attributes = read_attributes();
            if (decl.has_trait) decl.trait_type = read_type("impl trait type");
            decl.for_type = read_type("impl target type");
            std::uint64_t associated_type_witness_count = read_count("impl associated type witness count");
            decl.associated_type_witnesses.reserve(static_cast<std::size_t>(associated_type_witness_count));
            for (std::uint64_t j = 0; j < associated_type_witness_count; ++j) {
                ImplDecl::AssociatedTypeWitness witness;
                witness.name = read_field("impl associated type witness name");
                witness.type = read_type("impl associated type witness type");
                witness.loc = default_loc();
                decl.associated_type_witnesses.push_back(std::move(witness));
            }
            std::uint64_t method_count = read_count("impl method count");
            decl.methods.reserve(static_cast<std::size_t>(method_count));
            for (std::uint64_t j = 0; j < method_count; ++j) {
                decl.methods.push_back(read_function_signature());
            }
            program.impls.push_back(std::move(decl));
        }

        if (pos_ != text_.size()) fail("trailing bytes after declaration summary");
        return program;
    }

private:
    const std::string& text_;
    std::string display_;
    std::size_t pos_ = 0;
    [[noreturn]] void fail(const std::string& detail) const {
        throw CompileError("malformed declaration summary for " + display_ + ": " + detail);
    }

    SourceLocation default_loc() const {
        SourceLocation loc;
        loc.line = 1;
        loc.column = 1;
        return loc;
    }

    void consume_header() {
        const std::string v0 = kModuleAstDeclsPayloadHeader;
        if (text_.compare(pos_, v0.size(), v0) == 0) {
            pos_ += v0.size();
            return;
        }
        fail("expected '" + std::string(kModuleAstDeclsPayloadHeader) + "'");
    }

    void consume_char(char expected, const std::string& label) {
        if (pos_ >= text_.size() || text_[pos_] != expected) {
            fail("expected '" + std::string(1, expected) + "' after " + label);
        }
        ++pos_;
    }

    std::uint64_t read_decimal_until(char terminator, const std::string& label) {
        if (pos_ >= text_.size()) fail("expected " + label);
        std::uint64_t result = 0;
        bool saw_digit = false;
        while (pos_ < text_.size() && text_[pos_] != terminator) {
            char c = text_[pos_++];
            if (c < '0' || c > '9') fail("expected decimal " + label);
            saw_digit = true;
            std::uint64_t digit = static_cast<std::uint64_t>(c - '0');
            if (result > (std::numeric_limits<std::uint64_t>::max() - digit) / 10) {
                fail(label + " is too large");
            }
            result = result * 10 + digit;
        }
        if (!saw_digit) fail("expected decimal " + label);
        consume_char(terminator, label);
        return result;
    }

    std::uint64_t read_count(const std::string& label) {
        return read_decimal_until(';', label);
    }

    std::string read_field(const std::string& label) {
        std::uint64_t size = read_decimal_until(':', label + " length");
        if (size > text_.size() - pos_) fail(label + " length exceeds remaining payload");
        std::string value = text_.substr(pos_, static_cast<std::size_t>(size));
        pos_ += static_cast<std::size_t>(size);
        consume_char(';', label);
        return value;
    }

    bool read_bool(const std::string& label) {
        if (pos_ + 1 >= text_.size() || text_[pos_ + 1] != ';') {
            fail("expected boolean " + label);
        }
        char value = text_[pos_];
        pos_ += 2;
        if (value == '0') return false;
        if (value == '1') return true;
        fail("expected boolean " + label);
    }

    TypeQualifier read_qualifier(const std::string& label) {
        std::string value = read_field(label);
        if (value == "value") return TypeQualifier::Value;
        if (value == "own") return TypeQualifier::Own;
        if (value == "ref") return TypeQualifier::Ref;
        if (value == "ref mut") return TypeQualifier::MutRef;
        if (value == "ptr") return TypeQualifier::Ptr;
        fail("unknown type qualifier '" + value + "'");
    }

    BindingMode read_binding_mode(const std::string& label) {
        std::string value = read_field(label);
        if (value == "value") return BindingMode::Value;
        if (value == "ref") return BindingMode::Ref;
        if (value == "ref mut") return BindingMode::RefMut;
        fail("unknown binding mode '" + value + "'");
    }

    TypeRef read_type(const std::string& label) {
        TypeRef type;
        type.qualifier = read_qualifier(label + " qualifier");
        type.name = read_field(label + " name");
        type.is_dyn_object = read_bool(label + " dyn flag");
        type.nullable = read_bool(label + " nullable flag");
        type.array_size = read_count(label + " array size");
        type.is_macro_invocation = read_bool(label + " macro invocation flag");
        if (type.is_macro_invocation) {
            std::uint64_t token_count = read_count(label + " macro token count");
            type.macro_tokens.reserve(static_cast<std::size_t>(token_count));
            for (std::uint64_t i = 0; i < token_count; ++i) {
                type.macro_tokens.push_back(read_token_payload(label + " macro token"));
            }
        }
        type.has_associated_projection = read_bool(label + " associated type projection flag");
        if (type.has_associated_projection) {
            type.associated_projection = read_field(label + " associated type projection name");
        }
        type.is_union_by = read_bool(label + " union by flag");
        if (type.is_union_by) {
            std::uint64_t selector_count = read_count(label + " union by selector count");
            type.union_by_selector.reserve(static_cast<std::size_t>(selector_count));
            for (std::uint64_t i = 0; i < selector_count; ++i) {
                type.union_by_selector.push_back(read_field(label + " union by selector part"));
            }
            std::uint64_t arm_count = read_count(label + " union by arm count");
            type.union_by_arm_names.reserve(static_cast<std::size_t>(arm_count));
            type.union_by_arm_types.reserve(static_cast<std::size_t>(arm_count));
            type.union_by_arm_locs.reserve(static_cast<std::size_t>(arm_count));
            for (std::uint64_t i = 0; i < arm_count; ++i) {
                type.union_by_arm_names.push_back(read_field(label + " union by arm name"));
                type.union_by_arm_types.push_back(read_type(label + " union by arm type"));
                type.union_by_arm_locs.push_back(default_loc());
            }
        }
        std::uint64_t arg_count = read_count(label + " argument count");
        type.args.reserve(static_cast<std::size_t>(arg_count));
        for (std::uint64_t i = 0; i < arg_count; ++i) {
            type.args.push_back(read_type(label + " argument"));
        }
        type.loc = default_loc();
        return type;
    }

    void skip_type(const std::string& label) {
        (void)read_type(label);
    }

    std::vector<GenericParam> read_generics() {
        std::uint64_t count = read_count("generic parameter count");
        std::vector<GenericParam> generics;
        generics.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t i = 0; i < count; ++i) {
            GenericParam generic;
            generic.name = read_field("generic parameter name");
            generic.has_constraint = read_bool("generic constraint flag");
            if (generic.has_constraint) generic.constraint = read_type("generic constraint type");
            generic.loc = default_loc();
            generics.push_back(std::move(generic));
        }
        return generics;
    }

    void skip_generics() {
        (void)read_generics();
    }

    std::vector<Attribute> read_attributes() {
        std::uint64_t count = read_count("attribute count");
        std::vector<Attribute> attributes;
        attributes.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t i = 0; i < count; ++i) {
            Attribute attr;
            attr.name = read_field("attribute name");
            attr.has_args = read_bool("attribute args flag");
            std::uint64_t arg_count = read_count("attribute token count");
            attr.args.reserve(static_cast<std::size_t>(arg_count));
            for (std::uint64_t j = 0; j < arg_count; ++j) {
                attr.args.push_back(read_token_payload("attribute token"));
            }
            attr.loc = default_loc();
            attributes.push_back(std::move(attr));
        }
        return attributes;
    }

    void skip_attributes() {
        (void)read_attributes();
    }

    FunctionDecl read_function_signature() {
        FunctionDecl fn;
        fn.module_name = read_field("function module name");
        fn.name = read_field("function name");
        fn.meta = read_bool("function meta flag");
        fn.is_extern = read_bool("function extern flag");
        fn.is_public = read_bool("function visibility");
        fn.is_variadic = read_bool("function variadic flag");
        fn.extern_abi = read_field("function extern ABI");
        fn.extern_link_name = read_field("function extern link name");
        fn.has_return_type = read_bool("function return type flag");
        fn.has_body = read_bool("function body flag");
        fn.generics = read_generics();
        fn.attributes = read_attributes();
        std::uint64_t param_count = read_count("function parameter count");
        fn.params.reserve(static_cast<std::size_t>(param_count));
        for (std::uint64_t i = 0; i < param_count; ++i) {
            Param param;
            param.name = read_field("function parameter name");
            param.has_pattern = read_bool("function parameter pattern flag");
            if (param.has_pattern) {
                param.pattern = read_pattern("function parameter pattern");
            }
            param.type = read_type("function parameter type");
            fn.params.push_back(std::move(param));
        }
        if (fn.has_return_type) fn.return_type = read_type("function return type");
        bool has_body_summary = read_bool("function body summary flag");
        if (has_body_summary) {
            if (!fn.has_body) fail("function body summary present without body flag");
            fn.has_body_summary = true;
            fn.body = read_body_stmt_list("function body");
        }
        fn.loc = default_loc();
        fn.variadic_loc = default_loc();
        return fn;
    }

    void skip_function_signature() {
        (void)read_function_signature();
    }

    TokenKind read_token_kind(const std::string& label) {
        return static_cast<TokenKind>(read_count(label));
    }

    Token read_token_payload(const std::string& label) {
        Token token;
        token.kind = read_token_kind(label + " kind");
        token.text = read_field(label + " text");
        token.int_value = read_count(label + " integer value");
        token.float_value = double_from_bits(read_count(label + " float bits"));
        token.literal_suffix = read_field(label + " literal suffix");
        token.loc = default_loc();
        token.span = span_from_location(token.loc);
        return token;
    }

    std::vector<TypeRef> read_type_arguments(const std::string& label) {
        std::uint64_t count = read_count(label + " count");
        std::vector<TypeRef> type_args;
        type_args.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t i = 0; i < count; ++i) {
            type_args.push_back(read_type(label + " type argument"));
        }
        return type_args;
    }

    std::vector<ExprPtr> read_const_expr_list(const std::string& label) {
        std::uint64_t count = read_count(label + " count");
        std::vector<ExprPtr> args;
        args.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t i = 0; i < count; ++i) {
            args.push_back(read_const_expr(label + " item"));
        }
        return args;
    }

    ExprPtr read_const_expr(const std::string& label) {
        std::string kind = read_field(label + " kind");
        auto expr = std::make_unique<Expr>();
        expr->loc = default_loc();
        if (kind == "integer") {
            expr->kind = ExprKind::Integer;
            expr->int_negative = read_bool(label + " integer sign");
            expr->int_value = read_count(label + " integer value");
            expr->literal_suffix = read_field(label + " integer suffix");
            return expr;
        }
        if (kind == "float") {
            expr->kind = ExprKind::Float;
            expr->float_value = double_from_bits(read_count(label + " float bits"));
            expr->literal_suffix = read_field(label + " float suffix");
            return expr;
        }
        if (kind == "string") {
            expr->kind = ExprKind::String;
            expr->string_value = read_field(label + " string value");
            return expr;
        }
        if (kind == "bool") {
            expr->kind = ExprKind::Bool;
            expr->bool_value = read_bool(label + " bool value");
            return expr;
        }
        if (kind == "null") {
            expr->kind = ExprKind::Null;
            return expr;
        }
        if (kind == "name") {
            expr->kind = ExprKind::Name;
            expr->name = read_field(label + " name");
            return expr;
        }
        if (kind == "borrow") {
            expr->kind = ExprKind::Borrow;
            expr->mutable_borrow = read_bool(label + " borrow mutability");
            ExprPtr operand = read_const_expr(label + " borrow operand");
            if (operand && operand->kind == ExprKind::Name) expr->name = operand->name;
            set_expr_operand(*expr, std::move(operand));
            return expr;
        }
        if (kind == "unary") {
            expr->kind = ExprKind::Unary;
            expr->op = read_token_kind(label + " unary operator");
            if (!is_summary_const_unary_op(expr->op)) fail("unsupported constant summary unary operator");
            set_expr_operand(*expr, read_const_expr(label + " unary operand"));
            return expr;
        }
        if (kind == "try") {
            expr->kind = ExprKind::Try;
            set_expr_operand(*expr, read_const_expr(label + " try operand"));
            return expr;
        }
        if (kind == "null-coalesce") {
            expr->kind = ExprKind::NullCoalesce;
            expr->op = TokenKind::QuestionQuestion;
            set_expr_left(*expr, read_const_expr(label + " coalesce left operand"));
            set_expr_right(*expr, read_const_expr(label + " coalesce right operand"));
            return expr;
        }
        if (kind == "binary") {
            expr->kind = ExprKind::Binary;
            expr->op = read_token_kind(label + " binary operator");
            if (!is_summary_const_binary_op(expr->op)) fail("unsupported constant summary binary operator");
            set_expr_left(*expr, read_const_expr(label + " binary left operand"));
            set_expr_right(*expr, read_const_expr(label + " binary right operand"));
            return expr;
        }
        if (kind == "cast") {
            expr->kind = ExprKind::Cast;
            expr->cast_type = read_type(label + " cast type");
            set_expr_operand(*expr, read_const_expr(label + " cast operand"));
            return expr;
        }
        if (kind == "field") {
            expr->kind = ExprKind::FieldAccess;
            set_expr_operand(*expr, read_const_expr(label + " field operand"));
            expr->name = read_field(label + " field name");
            return expr;
        }
        if (kind == "tuple-index") {
            expr->kind = ExprKind::TupleIndex;
            set_expr_operand(*expr, read_const_expr(label + " tuple-index operand"));
            expr->tuple_index = read_count(label + " tuple index");
            return expr;
        }
        if (kind == "index") {
            expr->kind = ExprKind::Index;
            set_expr_operand(*expr, read_const_expr(label + " index operand"));
            set_expr_right(*expr, read_const_expr(label + " index value"));
            return expr;
        }
        if (kind == "tuple") {
            expr->kind = ExprKind::Tuple;
            expr->args = read_const_expr_list(label + " tuple values");
            return expr;
        }
        if (kind == "vector") {
            expr->kind = ExprKind::Vector;
            expr->args = read_const_expr_list(label + " vector values");
            return expr;
        }
        if (kind == "struct" || kind == "qualified-struct") {
            expr->kind = ExprKind::StructLiteral;
            expr->name = read_field(label + " struct name");
            if (kind == "qualified-struct") {
                set_expr_receiver_type_args(*expr, read_type_arguments(label + " struct receiver type arguments"));
            }
            set_expr_type_args(*expr, read_type_arguments(label + " struct type arguments"));
            std::uint64_t field_count = read_count(label + " struct field count");
            ExprFieldNames& field_names = ensure_expr_field_names(*expr);
            field_names.reserve(static_cast<std::size_t>(field_count));
            expr->args.reserve(static_cast<std::size_t>(field_count));
            for (std::uint64_t i = 0; i < field_count; ++i) {
                field_names.push_back(read_field(label + " struct field name"));
                expr->args.push_back(read_const_expr(label + " struct field value"));
            }
            return expr;
        }
        if (kind == "call" || kind == "qualified-call") {
            expr->kind = ExprKind::Call;
            expr->name = read_field(label + " call name");
            if (kind == "qualified-call") {
                set_expr_receiver_type_args(*expr, read_type_arguments(label + " call receiver type arguments"));
            }
            set_expr_type_args(*expr, read_type_arguments(label + " call type arguments"));
            expr->args = read_const_expr_list(label + " call arguments");
            return expr;
        }
        if (kind == "indirect-call") {
            expr->kind = ExprKind::Call;
            set_expr_operand(*expr, read_const_expr(label + " indirect callee"));
            expr->args = read_const_expr_list(label + " indirect arguments");
            return expr;
        }
        if (kind == "method-call" || kind == "qualified-method-call") {
            expr->kind = ExprKind::MethodCall;
            set_expr_operand(*expr, read_const_expr(label + " method receiver"));
            expr->name = read_field(label + " method name");
            if (kind == "qualified-method-call") {
                set_expr_receiver_type_args(*expr, read_type_arguments(label + " method receiver type arguments"));
            }
            set_expr_type_args(*expr, read_type_arguments(label + " method type arguments"));
            expr->args = read_const_expr_list(label + " method arguments");
            return expr;
        }
        if (kind == "macro-call") {
            expr->kind = ExprKind::MacroCall;
            expr->name = read_field(label + " macro name");
            std::uint64_t token_count = read_count(label + " macro token count");
            auto tokens = std::make_unique<std::vector<Token>>();
            tokens->reserve(static_cast<std::size_t>(token_count));
            for (std::uint64_t i = 0; i < token_count; ++i) {
                tokens->push_back(read_token_payload(label + " macro token"));
            }
            expr->macro_tokens = std::move(tokens);
            return expr;
        }
        if (kind == "if-expr") {
            expr->kind = ExprKind::If;
            std::unique_ptr<Pattern> pattern;
            if (read_bool(label + " condition pattern flag")) {
                pattern = std::make_unique<Pattern>(read_pattern(label + " condition pattern"));
            }
            ExprPtr condition = read_const_expr(label + " condition");
            std::vector<StmtPtr> then_body = read_body_stmt_list(label + " then body");
            ExprPtr then_value = read_const_expr(label + " then value");
            std::vector<StmtPtr> else_body = read_body_stmt_list(label + " else body");
            ExprPtr else_value = read_const_expr(label + " else value");
            set_expr_if_payload(*expr,
                                std::move(condition),
                                std::move(pattern),
                                std::move(then_body),
                                std::move(then_value),
                                std::move(else_body),
                                std::move(else_value));
            return expr;
        }
        if (kind == "block-expr") {
            expr->kind = ExprKind::Block;
            std::string block_label = read_field(label + " block label");
            std::vector<StmtPtr> body = read_body_stmt_list(label + " block body");
            ExprPtr value = read_const_expr(label + " block value");
            set_expr_block_payload(*expr, std::move(block_label), std::move(body), std::move(value));
            return expr;
        }
        if (kind == "lambda") {
            expr->kind = ExprKind::Lambda;
            std::uint64_t param_count = read_count(label + " lambda parameter count");
            std::vector<Param> params;
            params.reserve(static_cast<std::size_t>(param_count));
            for (std::uint64_t i = 0; i < param_count; ++i) {
                Param param;
                param.name = read_field(label + " lambda parameter name");
                param.has_pattern = read_bool(label + " lambda parameter pattern flag");
                if (param.has_pattern) param.pattern = read_pattern(label + " lambda parameter pattern");
                param.binding_mode = read_binding_mode(label + " lambda parameter binding mode");
                param.type = read_type(label + " lambda parameter type");
                params.push_back(std::move(param));
            }
            bool has_result_type = read_bool(label + " lambda result type flag");
            TypeRef result_type;
            if (has_result_type) result_type = read_type(label + " lambda result type");
            std::vector<StmtPtr> body = read_body_stmt_list(label + " lambda body");
            ExprPtr value;
            if (read_bool(label + " lambda value flag")) {
                value = read_const_expr(label + " lambda value");
            }
            set_expr_lambda_payload(
                *expr,
                std::move(params),
                has_result_type,
                std::move(result_type),
                std::move(body),
                std::move(value));
            return expr;
        }
        if (kind == "match-expr") {
            expr->kind = ExprKind::Match;
            ExprPtr value = read_const_expr(label + " match value");
            std::uint64_t arm_count = read_count(label + " match arm count");
            std::vector<ExprMatchArm> arms;
            arms.reserve(static_cast<std::size_t>(arm_count));
            for (std::uint64_t i = 0; i < arm_count; ++i) {
                ExprMatchArm arm;
                arm.pattern = read_pattern(label + " match arm pattern");
                arm.value = read_const_expr(label + " match arm value");
                arm.loc = default_loc();
                arms.push_back(std::move(arm));
            }
            set_expr_match_payload(*expr, std::move(value), std::move(arms));
            return expr;
        }
        fail("unknown constant expression summary kind '" + kind + "'");
    }

    Pattern read_pattern(const std::string& label) {
        std::string kind = read_field(label + " kind");
        Pattern pattern;
        pattern.loc = default_loc();
        pattern.is_macro_invocation = read_bool(label + " macro invocation flag");
        if (pattern.is_macro_invocation) {
            std::uint64_t token_count = read_count(label + " macro token count");
            pattern.macro_tokens.reserve(static_cast<std::size_t>(token_count));
            for (std::uint64_t i = 0; i < token_count; ++i) {
                pattern.macro_tokens.push_back(read_token_payload(label + " macro token"));
            }
        }
        pattern.binding_mode = read_binding_mode(label + " binding mode");
        if (kind == "wildcard") {
            pattern.kind = PatternKind::Wildcard;
            return pattern;
        }
        if (kind == "binding") {
            pattern.kind = PatternKind::Binding;
            pattern.payload_name = read_field(label + " binding name");
            return pattern;
        }
        if (kind == "integer") {
            pattern.kind = PatternKind::IntegerLiteral;
            pattern.int_negative = read_bool(label + " integer sign");
            pattern.int_value = read_count(label + " integer value");
            pattern.literal_suffix = read_field(label + " integer suffix");
            return pattern;
        }
        if (kind == "bool") {
            pattern.kind = PatternKind::BoolLiteral;
            pattern.bool_value = read_bool(label + " bool value");
            return pattern;
        }
        if (kind == "range") {
            pattern.kind = PatternKind::Range;
            pattern.int_negative = read_bool(label + " range start sign");
            pattern.int_value = read_count(label + " range start value");
            pattern.literal_suffix = read_field(label + " range start suffix");
            pattern.range_inclusive = read_bool(label + " range inclusivity");
            pattern.range_end_negative = read_bool(label + " range end sign");
            pattern.range_end_value = read_count(label + " range end value");
            pattern.range_end_suffix = read_field(label + " range end suffix");
            return pattern;
        }
        if (kind == "enum") {
            pattern.kind = PatternKind::EnumCase;
            pattern.case_name = read_field(label + " enum case name");
            pattern.has_payload_pattern = read_bool(label + " enum payload pattern flag");
            pattern.has_payload_binding = read_bool(label + " enum payload binding flag");
            pattern.payload_name = read_field(label + " enum payload binding name");
            if (pattern.has_payload_pattern) {
                pattern.payload_pattern = std::make_unique<Pattern>(read_pattern(label + " enum payload pattern"));
            }
            return pattern;
        }
        if (kind == "or") {
            pattern.kind = PatternKind::Or;
            std::uint64_t count = read_count(label + " alternative count");
            pattern.alternatives.reserve(static_cast<std::size_t>(count));
            for (std::uint64_t i = 0; i < count; ++i) {
                pattern.alternatives.push_back(read_pattern(label + " alternative"));
            }
            return pattern;
        }
        if (kind == "alias") {
            pattern.kind = PatternKind::Alias;
            pattern.alias_name = read_field(label + " alias name");
            pattern.alias_pattern = std::make_unique<Pattern>(read_pattern(label + " alias pattern"));
            return pattern;
        }
        if (kind == "tuple" || kind == "array" || kind == "array-rest-alias") {
            pattern.kind = kind == "tuple" ? PatternKind::Tuple : PatternKind::Array;
            pattern.has_rest = read_bool(label + " rest flag");
            pattern.rest_index = static_cast<std::size_t>(read_count(label + " rest index"));
            if (kind == "array-rest-alias") {
                pattern.rest_alias_name = read_field(label + " rest alias name");
                pattern.rest_alias_loc = default_loc();
            }
            std::uint64_t count = read_count(label + " element count");
            pattern.elements.reserve(static_cast<std::size_t>(count));
            for (std::uint64_t i = 0; i < count; ++i) {
                pattern.elements.push_back(read_pattern(label + " element"));
            }
            return pattern;
        }
        if (kind == "struct") {
            pattern.kind = PatternKind::Struct;
            pattern.case_name = read_field(label + " struct name");
            pattern.has_rest = read_bool(label + " struct rest flag");
            pattern.rest_index = static_cast<std::size_t>(read_count(label + " struct rest index"));
            std::uint64_t count = read_count(label + " struct field count");
            pattern.field_names.reserve(static_cast<std::size_t>(count));
            pattern.elements.reserve(static_cast<std::size_t>(count));
            for (std::uint64_t i = 0; i < count; ++i) {
                pattern.field_names.push_back(read_field(label + " struct field name"));
                pattern.elements.push_back(read_pattern(label + " struct field pattern"));
            }
            return pattern;
        }
        fail("unknown pattern summary kind '" + kind + "'");
    }

    ExprPtr read_const_initializer(const std::string& label) {
        if (!read_bool(label + " flag")) return nullptr;
        return read_const_expr(label);
    }

    void skip_const_initializer(const std::string& label) {
        (void)read_const_initializer(label);
    }

    std::vector<StmtPtr> read_body_stmt_list(const std::string& label) {
        std::uint64_t count = read_count(label + " statement count");
        std::vector<StmtPtr> statements;
        statements.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t i = 0; i < count; ++i) {
            statements.push_back(read_body_stmt(label + " statement"));
        }
        return statements;
    }

    StmtPtr read_body_stmt(const std::string& label) {
        std::string kind = read_field(label + " kind");
        auto stmt = std::make_unique<Stmt>();
        stmt->loc = default_loc();
        if (kind == "block") {
            stmt->kind = StmtKind::Block;
            set_stmt_label(*stmt, read_field(label + " block label"));
            set_stmt_statements(*stmt, read_body_stmt_list(label + " block body"));
            return stmt;
        }
        if (kind == "var") {
            stmt->kind = StmtKind::VarDecl;
            stmt->binding.name = read_field(label + " binding name");
            stmt->binding.mutable_binding = read_bool(label + " binding mutability");
            stmt->binding.binding_mode = read_binding_mode(label + " binding mode");
            stmt->binding.has_pattern = read_bool(label + " binding pattern flag");
            if (stmt->binding.has_pattern) {
                stmt->binding.pattern = read_pattern(label + " binding pattern");
            }
            stmt->binding.has_type = read_bool(label + " binding type flag");
            if (stmt->binding.has_type) stmt->binding.type = read_type(label + " binding type");
            if (read_bool(label + " binding initializer flag")) {
                stmt->binding.init = read_const_expr(label + " binding initializer");
            }
            stmt->binding.loc = default_loc();
            return stmt;
        }
        if (kind == "assign") {
            stmt->kind = StmtKind::Assign;
            set_stmt_assign_name(*stmt, read_field(label + " assignment name"));
            if (read_bool(label + " assignment target flag")) {
                set_stmt_assign_target(*stmt, read_const_expr(label + " assignment target"));
            }
            set_stmt_assign_rhs(*stmt, read_const_expr(label + " assignment value"));
            return stmt;
        }
        if (kind == "return") {
            stmt->kind = StmtKind::Return;
            if (read_bool(label + " return value flag")) {
                stmt->expr = read_const_expr(label + " return value");
            }
            return stmt;
        }
        if (kind == "expr-stmt") {
            stmt->kind = StmtKind::ExprStmt;
            stmt->expr = read_const_expr(label + " expression");
            return stmt;
        }
        if (kind == "if") {
            stmt->kind = StmtKind::If;
            stmt->condition = read_const_expr(label + " condition");
            set_stmt_then_body(*stmt, read_body_stmt_list(label + " then body"));
            set_stmt_else_body(*stmt, read_body_stmt_list(label + " else body"));
            return stmt;
        }
        if (kind == "if-let") {
            stmt->kind = StmtKind::If;
            stmt->has_condition_pattern = true;
            stmt->condition_pattern = std::make_unique<Pattern>(read_pattern(label + " condition pattern"));
            stmt->condition = read_const_expr(label + " condition");
            set_stmt_then_body(*stmt, read_body_stmt_list(label + " then body"));
            set_stmt_else_body(*stmt, read_body_stmt_list(label + " else body"));
            return stmt;
        }
        if (kind == "break") {
            stmt->kind = StmtKind::Break;
            set_stmt_break_label(*stmt, read_field(label + " break label"));
            if (read_bool(label + " break value flag")) {
                set_stmt_break_value(*stmt, read_const_expr(label + " break value"));
            }
            return stmt;
        }
        if (kind == "while") {
            stmt->kind = StmtKind::While;
            stmt->condition = read_const_expr(label + " condition");
            set_stmt_loop_body(*stmt, read_body_stmt_list(label + " loop body"));
            return stmt;
        }
        if (kind == "while-let") {
            stmt->kind = StmtKind::WhileLet;
            stmt->has_condition_pattern = true;
            stmt->condition_pattern = std::make_unique<Pattern>(read_pattern(label + " condition pattern"));
            stmt->condition = read_const_expr(label + " condition");
            set_stmt_loop_body(*stmt, read_body_stmt_list(label + " loop body"));
            return stmt;
        }
        if (kind == "for") {
            stmt->kind = StmtKind::For;
            stmt->for_pattern_filter = read_bool(label + " for pattern filter flag");
            stmt->for_pattern = std::make_unique<Pattern>(read_pattern(label + " for pattern"));
            stmt->for_iterable = read_const_expr(label + " for iterable");
            set_stmt_loop_body(*stmt, read_body_stmt_list(label + " loop body"));
            return stmt;
        }
        if (kind == "init-while") {
            stmt->kind = StmtKind::InitWhile;
            std::uint64_t binding_count = read_count(label + " init binding count");
            stmt->init_bindings.reserve(static_cast<std::size_t>(binding_count));
            for (std::uint64_t i = 0; i < binding_count; ++i) {
                Binding binding;
                binding.loc = default_loc();
                binding.name = read_field(label + " init binding name");
                binding.mutable_binding = read_bool(label + " init binding mutability");
                binding.binding_mode = read_binding_mode(label + " init binding mode");
                binding.has_pattern = read_bool(label + " init binding pattern flag");
                if (binding.has_pattern) {
                    binding.pattern = read_pattern(label + " init binding pattern");
                }
                binding.has_type = read_bool(label + " init binding type flag");
                if (binding.has_type) binding.type = read_type(label + " init binding type");
                binding.init = read_const_expr(label + " init binding initializer");
                stmt->init_bindings.push_back(std::move(binding));
            }
            stmt->condition = read_const_expr(label + " condition");
            set_stmt_loop_body(*stmt, read_body_stmt_list(label + " loop body"));
            stmt->updates = read_const_expr_list(label + " updates");
            return stmt;
        }
        if (kind == "continue") {
            stmt->kind = StmtKind::Continue;
            stmt->updates = read_const_expr_list(label + " updates");
            return stmt;
        }
        if (kind == "match") {
            stmt->kind = StmtKind::Match;
            stmt->match_value = read_const_expr(label + " match value");
            std::uint64_t arm_count = read_count(label + " match arm count");
            StmtMatchArms& arms = ensure_stmt_match_arms(*stmt);
            arms.reserve(static_cast<std::size_t>(arm_count));
            for (std::uint64_t i = 0; i < arm_count; ++i) {
                MatchArm arm;
                arm.pattern = read_pattern(label + " match arm pattern");
                arm.body = read_body_stmt_list(label + " match arm body");
                arm.loc = default_loc();
                arms.push_back(std::move(arm));
            }
            return stmt;
        }
        if (kind == "drop") {
            stmt->kind = StmtKind::Drop;
            set_stmt_drop_name(*stmt, read_field(label + " drop name"));
            return stmt;
        }
        if (kind == "forget") {
            stmt->kind = StmtKind::Forget;
            set_stmt_drop_name(*stmt, read_field(label + " forget name"));
            return stmt;
        }
        fail("unknown function body statement summary kind '" + kind + "'");
    }
};

std::string declaration_summary_payload(const Program& program) {
    std::ostringstream out;
    out << kModuleAstDeclsPayloadHeader;

    append_count(out, program.uses.size());
    for (const auto& decl : program.uses) {
        append_field(out, decl.module_name);
        append_field(out, decl.path);
        append_field(out, decl.alias);
        append_bool(out, decl.is_public);
        append_bool(out, decl.is_glob);
    }

    append_count(out, program.module_imports.size());
    for (const auto& decl : program.module_imports) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_field(out, decl.local_name);
        append_bool(out, decl.is_public);
    }

    append_count(out, program.modules.size());
    for (const auto& decl : program.modules) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
    }

    append_count(out, program.item_macros.size());
    for (const auto& invocation : program.item_macros) {
        append_item_macro_invocation(out, invocation);
    }

    append_count(out, program.constants.size());
    for (const auto& decl : program.constants) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_type(out, decl.type);
        append_const_initializer(out, decl.init);
    }

    append_count(out, program.type_aliases.size());
    for (const auto& decl : program.type_aliases) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_generics(out, decl.generics);
        append_type(out, decl.target);
    }

    append_count(out, program.functions.size());
    for (const auto& fn : program.functions) append_function_signature(out, fn);

    append_count(out, program.structs.size());
    for (const auto& decl : program.structs) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_bool(out, decl.tuple_struct);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        append_count(out, decl.fields.size());
        for (const auto& field : decl.fields) {
            append_field(out, field.name);
            append_bool(out, field.mutable_field);
            append_type(out, field.type);
        }
    }

    append_count(out, program.enums.size());
    for (const auto& decl : program.enums) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        append_count(out, decl.cases.size());
        for (const auto& item : decl.cases) {
            append_field(out, item.name);
            append_count(out, item.payloads.size());
            for (const auto& payload : item.payloads) append_type(out, payload);
        }
    }

    append_count(out, program.traits.size());
    for (const auto& decl : program.traits) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        append_count(out, decl.associated_types.size());
        for (const auto& associated_type : decl.associated_types) append_field(out, associated_type.name);
        append_count(out, decl.methods.size());
        for (const auto& method : decl.methods) append_function_signature(out, method);
    }

    append_count(out, program.impls.size());
    for (const auto& decl : program.impls) {
        append_field(out, decl.module_name);
        append_bool(out, decl.is_public);
        append_bool(out, decl.has_trait);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        if (decl.has_trait) append_type(out, decl.trait_type);
        append_type(out, decl.for_type);
        append_count(out, decl.associated_type_witnesses.size());
        for (const auto& witness : decl.associated_type_witnesses) {
            append_field(out, witness.name);
            append_type(out, witness.type);
        }
        append_count(out, decl.methods.size());
        for (const auto& method : decl.methods) append_function_signature(out, method);
    }

    return out.str();
}

DeclarationSummaryCounts parse_declaration_summary_payload(const ModuleCacheAstSummary& summary) {
    DeclarationSummaryReader reader(summary.declaration_summary, summary_display(summary));
    return reader.parse();
}

Program materialize_declaration_summary_payload(const ModuleCacheAstSummary& summary) {
    DeclarationSummaryReader reader(summary.declaration_summary, summary_display(summary));
    return reader.parse_program();
}

void require_count_match(std::uint64_t recorded,
                         std::uint64_t parsed,
                         const std::string& label,
                         const ModuleCacheAstSummary& summary) {
    if (recorded == parsed) return;
    throw CompileError("module cache AST summary for " + summary_display(summary) +
                       " has " + label + " count " + std::to_string(recorded) +
                       " but declaration summary contains " + std::to_string(parsed));
}

} // namespace

ModuleCacheAstSummary make_module_cache_ast_summary(const std::string& path,
                                                    const std::string& content_hash,
                                                    const std::vector<std::string>& module_path,
                                                    const Program& program,
                                                    bool is_root) {
    ModuleCacheAstSummary summary;
    summary.module_name = join_qualified_path(module_path);
    summary.path = path;
    summary.content_hash = content_hash;
    summary.declaration_summary = declaration_summary_payload(program);
    summary.declaration_hash = module_metadata_source_hash(summary.declaration_summary);
    summary.is_root = is_root;
    summary.use_count = program.uses.size();
    summary.module_import_count = program.module_imports.size();
    summary.module_decl_count = program.modules.size();
    summary.constant_count = program.constants.size();
    summary.type_alias_count = program.type_aliases.size();
    summary.function_count = program.functions.size();
    summary.struct_count = program.structs.size();
    summary.enum_count = program.enums.size();
    summary.trait_count = program.traits.size();
    summary.impl_count = program.impls.size();
    return summary;
}

void require_valid_module_cache_ast_summary_payload(const ModuleCacheAstSummary& summary,
                                                    const std::string& display_path) {
    if (summary.declaration_summary.empty()) {
        fail_invalid_module_cache_ast_summary(
            display_path,
            "AST summary for " + summary_display(summary) +
                " is missing a declaration summary");
    }
    if (summary.declaration_summary.compare(
            0,
            std::strlen(kModuleAstDeclsPayloadHeader),
            kModuleAstDeclsPayloadHeader
        ) != 0) {
        fail_invalid_module_cache_ast_summary(
            display_path,
            "AST summary for " + summary_display(summary) +
                " declaration summary expected '" +
                std::string(kModuleAstDeclsPayloadHeader) + "' header");
    }
    std::string hash = module_metadata_source_hash(summary.declaration_summary);
    if (hash != summary.declaration_hash) {
        fail_invalid_module_cache_ast_summary(
            display_path,
            "AST summary for " + summary_display(summary) +
                " declaration summary hashes to '" + hash +
                "' instead of recorded '" + summary.declaration_hash + "'");
    }
    try {
        DeclarationSummaryCounts counts = parse_declaration_summary_payload(summary);
        require_count_match(summary.use_count, counts.use_count, "use", summary);
        require_count_match(summary.module_import_count, counts.module_import_count, "module import", summary);
        require_count_match(summary.module_decl_count, counts.module_decl_count, "module declaration", summary);
        require_count_match(summary.constant_count, counts.constant_count, "constant", summary);
        require_count_match(summary.type_alias_count, counts.type_alias_count, "type alias", summary);
        require_count_match(summary.function_count, counts.function_count, "function", summary);
        require_count_match(summary.struct_count, counts.struct_count, "struct", summary);
        require_count_match(summary.enum_count, counts.enum_count, "enum", summary);
        require_count_match(summary.trait_count, counts.trait_count, "trait", summary);
        require_count_match(summary.impl_count, counts.impl_count, "impl", summary);
        Program declarations = materialize_declaration_summary_payload(summary);
        ModuleCacheAstSummary round_trip = make_module_cache_ast_summary(
            summary.path,
            summary.content_hash,
            split_qualified_path(summary.module_name),
            declarations,
            summary.is_root
        );
        if (round_trip.declaration_summary != summary.declaration_summary ||
            round_trip.declaration_hash != summary.declaration_hash) {
            throw CompileError("declaration summary does not round-trip through materialized declarations");
        }
    } catch (const CompileError& error) {
        fail_invalid_module_cache_ast_summary(display_path, error.what());
    }
}

Program materialize_module_cache_ast_summary_declarations(const ModuleCacheAstSummary& summary,
                                                          const std::string& display_path) {
    try {
        return materialize_declaration_summary_payload(summary);
    } catch (const CompileError& error) {
        fail_invalid_module_cache_ast_summary(display_path, error.what());
    }
}

bool can_load_module_cache_ast_summary_declarations(const Program& program) {
    if (!program.item_macros.empty()) return false;
    for (const auto& decl : program.constants) {
        if (!decl.init) return false;
    }
    for (const auto& fn : program.functions) {
        if (fn.has_body && !fn.is_extern && !fn.has_body_summary) return false;
    }
    for (const auto& trait : program.traits) {
        for (const auto& method : trait.methods) {
            if (method.has_body && !method.is_extern && !method.has_body_summary) return false;
        }
    }
    for (const auto& impl : program.impls) {
        for (const auto& method : impl.methods) {
            if (method.has_body && !method.is_extern && !method.has_body_summary) return false;
        }
    }
    return true;
}

} // namespace ari
