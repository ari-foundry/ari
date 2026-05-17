#include "module_ir_layout_summary.hpp"

#include "common.hpp"
#include "module_ir_summary.hpp"
#include "module_ir_type_summary.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <utility>

namespace ari {
namespace {

constexpr const char* kVectorStorageLayout = "vector-storage";

std::string trim(std::string text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.erase(text.begin());
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.pop_back();
    }
    return text;
}

std::uint64_t parse_capacity(const std::string& text) {
    std::string value = trim(text);
    if (value.empty()) {
        throw CompileError("IR layout descriptor vector-storage capacity is empty");
    }
    std::uint64_t result = 0;
    for (char c : value) {
        if (c < '0' || c > '9') {
            throw CompileError("IR layout descriptor vector-storage capacity must be a non-negative integer");
        }
        std::uint64_t digit = static_cast<std::uint64_t>(c - '0');
        if (result > (UINT64_MAX - digit) / 10) {
            throw CompileError("IR layout descriptor vector-storage capacity is too large");
        }
        result = result * 10 + digit;
    }
    return result;
}

void add_descriptor(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                    ModuleCacheIrLayoutDescriptor descriptor) {
    std::string key = descriptor.kind + "\n" + descriptor.type;
    auto inserted = descriptors.emplace(key, std::move(descriptor));
    if (!inserted.second) return;
}

std::size_t find_matching_bracket(const std::string& text, std::size_t open) {
    std::uint64_t depth = 0;
    for (std::size_t i = open; i < text.size(); ++i) {
        if (text[i] == '[') {
            ++depth;
        } else if (text[i] == ']') {
            if (depth == 0) return std::string::npos;
            --depth;
            if (depth == 0) return i;
        }
    }
    return std::string::npos;
}

std::size_t find_top_level_semicolon(const std::string& text) {
    std::uint64_t bracket_depth = 0;
    std::uint64_t paren_depth = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (c == '[') {
            ++bracket_depth;
        } else if (c == ']') {
            if (bracket_depth > 0) --bracket_depth;
        } else if (c == '(') {
            ++paren_depth;
        } else if (c == ')') {
            if (paren_depth > 0) --paren_depth;
        } else if (c == ';' && bracket_depth == 0 && paren_depth == 0) {
            return i;
        }
    }
    return std::string::npos;
}

void collect_layout_type_text(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                              const std::string& type_text) {
    std::size_t search = 0;
    while (search < type_text.size()) {
        std::size_t vec = type_text.find("Vec[", search);
        if (vec == std::string::npos) return;

        std::size_t open = vec + 3;
        std::size_t close = find_matching_bracket(type_text, open);
        if (close == std::string::npos) return;

        std::string payload = type_text.substr(open + 1, close - open - 1);
        std::size_t semi = find_top_level_semicolon(payload);
        if (semi != std::string::npos) {
            std::string element = trim(payload.substr(0, semi));
            std::uint64_t capacity = parse_capacity(payload.substr(semi + 1));
            ModuleCacheIrLayoutDescriptor descriptor;
            descriptor.kind = kVectorStorageLayout;
            descriptor.type = "Vec[" + element + "; " + std::to_string(capacity) + "]";
            descriptor.element_type = element;
            descriptor.slot_count = capacity;
            add_descriptor(descriptors, std::move(descriptor));
            collect_layout_type_text(descriptors, element);
        } else {
            collect_layout_type_text(descriptors, payload);
        }

        search = close + 1;
    }
}

IrType value_qualified_type(IrType type) {
    type.qualifier = TypeQualifier::Value;
    return type;
}

void collect_layout_type(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                         const IrType& type) {
    IrType value_type = value_qualified_type(type);
    if (value_type.primitive == IrPrimitiveKind::Vector &&
        value_type.args.size() == 1 &&
        value_type.array_size != 0) {
        ModuleCacheIrLayoutDescriptor descriptor;
        descriptor.kind = kVectorStorageLayout;
        descriptor.type = module_cache_ir_type_name(value_type);
        descriptor.element_type = module_cache_ir_type_name(value_type.args[0]);
        descriptor.slot_count = value_type.array_size;
        add_descriptor(descriptors, std::move(descriptor));
    }

    for (const auto& arg : value_type.args) collect_layout_type(descriptors, arg);
    for (const auto& field : value_type.field_types) collect_layout_type(descriptors, field);
}

void collect_layout_expr(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                         const IrExprPtr& expr);
void collect_layout_stmts(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                          const std::vector<IrStmtPtr>& statements);

void collect_layout_exprs(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                          const std::vector<IrExprPtr>& expressions) {
    for (const auto& expr : expressions) collect_layout_expr(descriptors, expr);
}

void collect_layout_binding(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                            const IrBinding& binding) {
    collect_layout_type(descriptors, binding.type);
    collect_layout_expr(descriptors, binding.init);
}

void collect_layout_payload_bindings(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                                     const std::vector<IrPayloadBinding>& bindings) {
    for (const auto& binding : bindings) {
        collect_layout_type(descriptors, binding.type);
        collect_layout_type(descriptors, binding.compact_enum_type);
    }
}

template <typename Arm>
void collect_layout_match_arm(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                              const Arm& arm) {
    collect_layout_type(descriptors, arm.value_type);
    collect_layout_type(descriptors, arm.payload_type);
    collect_layout_payload_bindings(descriptors, arm.payload_bindings);
    for (const auto& condition : arm.payload_range_conditions) collect_layout_type(descriptors, condition.type);
    for (const auto& condition : arm.payload_enum_conditions) {
        collect_layout_type(descriptors, condition.enum_type);
        collect_layout_type(descriptors, condition.payload_type);
    }
    collect_layout_stmts(descriptors, arm.body);
}

void collect_layout_match_expr_arm(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                                   const IrMatchExprArm& arm) {
    collect_layout_match_arm(descriptors, arm);
    collect_layout_expr(descriptors, arm.value);
}

void collect_layout_expr(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                         const IrExprPtr& expr) {
    if (!expr) return;
    collect_layout_type(descriptors, expr->type);
    collect_layout_type(descriptors, ir_expr_enum_payload_type(*expr));
    for (const auto& type : ir_expr_call_param_types(*expr)) collect_layout_type(descriptors, type);
    collect_layout_type(descriptors, ir_expr_try_return_residual_payload_type(*expr));

    collect_layout_expr(descriptors, ir_expr_operand(*expr));
    collect_layout_expr(descriptors, ir_expr_left(*expr));
    collect_layout_expr(descriptors, ir_expr_right(*expr));
    collect_layout_expr(descriptors, ir_expr_payload(*expr));
    collect_layout_exprs(descriptors, expr->args);
    collect_layout_expr(descriptors, ir_expr_if_condition(*expr));
    collect_layout_stmts(descriptors, ir_expr_if_then_body(*expr));
    collect_layout_expr(descriptors, ir_expr_if_then_value(*expr));
    collect_layout_stmts(descriptors, ir_expr_if_else_body(*expr));
    collect_layout_expr(descriptors, ir_expr_if_else_value(*expr));
    collect_layout_stmts(descriptors, ir_expr_block_body(*expr));
    collect_layout_expr(descriptors, ir_expr_block_value(*expr));
    collect_layout_expr(descriptors, ir_expr_match_value(*expr));
    for (const auto& arm : ir_expr_match_arms(*expr)) collect_layout_match_expr_arm(descriptors, arm);
    collect_layout_stmts(descriptors, ir_expr_try_residual_cleanup(*expr));
}

void collect_layout_stmts(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                          const std::vector<IrStmtPtr>& statements) {
    for (const auto& statement : statements) {
        if (!statement) continue;
        collect_layout_binding(descriptors, statement->binding);
        collect_layout_expr(descriptors, ir_stmt_assign_target(*statement));
        collect_layout_expr(descriptors, ir_stmt_assign_rhs(*statement));
        collect_layout_expr(descriptors, statement->expr);
        collect_layout_expr(descriptors, statement->condition);
        collect_layout_stmts(descriptors, ir_stmt_statements(*statement));
        collect_layout_stmts(descriptors, ir_stmt_then_body(*statement));
        collect_layout_stmts(descriptors, ir_stmt_else_body(*statement));
        collect_layout_stmts(descriptors, ir_stmt_loop_body(*statement));
        collect_layout_type(descriptors, ir_stmt_for_binding_type(*statement));
        collect_layout_expr(descriptors, ir_stmt_for_start(*statement));
        collect_layout_expr(descriptors, ir_stmt_for_end(*statement));
        collect_layout_exprs(descriptors, ir_stmt_for_values(*statement));
        collect_layout_expr(descriptors, statement->match_value);
        for (const auto& binding : statement->init_bindings) collect_layout_binding(descriptors, binding);
        collect_layout_exprs(descriptors, statement->updates);
        for (const auto& arm : ir_stmt_match_arms(*statement)) collect_layout_match_arm(descriptors, arm);
        collect_layout_expr(descriptors, ir_stmt_break_value(*statement));
    }
}

void collect_layout_summary_expr(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                                 const ModuleCacheIrExprSummaryPtr& expr);
void collect_layout_summary_stmts(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                                  const std::vector<ModuleCacheIrStmtSummaryPtr>& statements);

void collect_layout_summary_type(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                                 const std::string& type) {
    collect_layout_type_text(descriptors, type);
}

void collect_layout_summary_exprs(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                                  const std::vector<ModuleCacheIrExprSummaryPtr>& expressions) {
    for (const auto& expr : expressions) collect_layout_summary_expr(descriptors, expr);
}

void collect_layout_summary_binding(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                                    const ModuleCacheIrBindingSummary& binding) {
    collect_layout_summary_type(descriptors, binding.type);
    collect_layout_summary_expr(descriptors, binding.init);
}

void collect_layout_summary_payload_bindings(
    std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
    const std::vector<ModuleCacheIrPayloadBindingSummary>& bindings) {
    for (const auto& binding : bindings) {
        collect_layout_summary_type(descriptors, binding.type);
        collect_layout_summary_type(descriptors, binding.compact_enum_type);
    }
}

void collect_layout_summary_match_pattern(
    std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
    const ModuleCacheIrMatchArmPatternSummary& pattern) {
    collect_layout_summary_type(descriptors, pattern.value_type);
    collect_layout_summary_type(descriptors, pattern.payload_type);
    collect_layout_summary_payload_bindings(descriptors, pattern.payload_bindings);
    for (const auto& condition : pattern.payload_range_conditions) {
        collect_layout_summary_type(descriptors, condition.type);
    }
    for (const auto& condition : pattern.payload_enum_conditions) {
        collect_layout_summary_type(descriptors, condition.enum_type);
        collect_layout_summary_type(descriptors, condition.payload_type);
    }
}

void collect_layout_summary_expr(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                                 const ModuleCacheIrExprSummaryPtr& expr) {
    if (!expr) return;
    collect_layout_summary_type(descriptors, expr->type);
    collect_layout_summary_type(descriptors, expr->enum_payload_type);
    for (const auto& type : expr->call_param_types) collect_layout_summary_type(descriptors, type);
    collect_layout_summary_type(descriptors, expr->try_return_residual_payload_type);

    collect_layout_summary_expr(descriptors, expr->operand);
    collect_layout_summary_expr(descriptors, expr->left);
    collect_layout_summary_expr(descriptors, expr->right);
    collect_layout_summary_expr(descriptors, expr->payload);
    collect_layout_summary_exprs(descriptors, expr->args);
    collect_layout_summary_expr(descriptors, expr->if_condition);
    collect_layout_summary_stmts(descriptors, expr->if_then_body);
    collect_layout_summary_expr(descriptors, expr->if_then_value);
    collect_layout_summary_stmts(descriptors, expr->if_else_body);
    collect_layout_summary_expr(descriptors, expr->if_else_value);
    collect_layout_summary_stmts(descriptors, expr->block_body);
    collect_layout_summary_expr(descriptors, expr->block_value);
    collect_layout_summary_expr(descriptors, expr->match_value);
    for (const auto& arm : expr->match_arms) {
        collect_layout_summary_match_pattern(descriptors, arm.pattern);
        collect_layout_summary_stmts(descriptors, arm.body);
        collect_layout_summary_expr(descriptors, arm.value);
    }
    collect_layout_summary_stmts(descriptors, expr->try_residual_cleanup);
}

void collect_layout_summary_stmts(std::map<std::string, ModuleCacheIrLayoutDescriptor>& descriptors,
                                  const std::vector<ModuleCacheIrStmtSummaryPtr>& statements) {
    for (const auto& statement : statements) {
        if (!statement) continue;
        collect_layout_summary_binding(descriptors, statement->binding);
        collect_layout_summary_expr(descriptors, statement->assign_target);
        collect_layout_summary_expr(descriptors, statement->assign_rhs);
        collect_layout_summary_expr(descriptors, statement->expr);
        collect_layout_summary_expr(descriptors, statement->condition);
        collect_layout_summary_stmts(descriptors, statement->statements);
        collect_layout_summary_stmts(descriptors, statement->then_body);
        collect_layout_summary_stmts(descriptors, statement->else_body);
        collect_layout_summary_stmts(descriptors, statement->loop_body);
        collect_layout_summary_type(descriptors, statement->for_binding_type);
        collect_layout_summary_expr(descriptors, statement->for_start);
        collect_layout_summary_expr(descriptors, statement->for_end);
        collect_layout_summary_exprs(descriptors, statement->for_values);
        collect_layout_summary_expr(descriptors, statement->match_value);
        for (const auto& binding : statement->init_bindings) collect_layout_summary_binding(descriptors, binding);
        collect_layout_summary_exprs(descriptors, statement->updates);
        for (const auto& arm : statement->match_arms) {
            collect_layout_summary_match_pattern(descriptors, arm.pattern);
            collect_layout_summary_stmts(descriptors, arm.body);
        }
        collect_layout_summary_expr(descriptors, statement->break_value);
    }
}

std::vector<ModuleCacheIrLayoutDescriptor>
ordered_descriptors(std::map<std::string, ModuleCacheIrLayoutDescriptor> descriptors) {
    std::vector<ModuleCacheIrLayoutDescriptor> result;
    result.reserve(descriptors.size());
    for (auto& item : descriptors) result.push_back(std::move(item.second));
    return result;
}

std::string descriptor_key(const ModuleCacheIrLayoutDescriptor& descriptor) {
    return descriptor.kind + "\n" + descriptor.type;
}

void require_valid_descriptor_shape(const ModuleCacheIrLayoutDescriptor& descriptor) {
    if (descriptor.kind != kVectorStorageLayout) {
        throw CompileError("IR layout descriptor has unknown kind '" + descriptor.kind + "'");
    }
    if (descriptor.type.empty()) {
        throw CompileError("IR layout descriptor vector-storage type is empty");
    }
    if (descriptor.element_type.empty()) {
        throw CompileError("IR layout descriptor vector-storage element type is empty");
    }

    std::map<std::string, ModuleCacheIrLayoutDescriptor> parsed;
    collect_layout_type_text(parsed, descriptor.type);
    auto found = parsed.find(descriptor_key(descriptor));
    if (found == parsed.end() ||
        found->second.element_type != descriptor.element_type ||
        found->second.slot_count != descriptor.slot_count) {
        throw CompileError("IR layout descriptor mismatch for " + descriptor.type);
    }
}

} // namespace

std::vector<ModuleCacheIrLayoutDescriptor>
module_cache_ir_layout_descriptors(const std::vector<const IrFunction*>& functions) {
    std::map<std::string, ModuleCacheIrLayoutDescriptor> descriptors;
    for (const IrFunction* fn : functions) {
        if (!fn) continue;
        collect_layout_type(descriptors, fn->return_type);
        for (const auto& param : fn->params) collect_layout_type(descriptors, param.type);
        collect_layout_stmts(descriptors, fn->body);
    }
    return ordered_descriptors(std::move(descriptors));
}

std::vector<ModuleCacheIrLayoutDescriptor>
module_cache_ir_layout_descriptors(const std::vector<ModuleCacheIrFunctionSummary>& functions) {
    std::map<std::string, ModuleCacheIrLayoutDescriptor> descriptors;
    for (const auto& fn : functions) {
        collect_layout_summary_type(descriptors, fn.return_type);
        for (const auto& param : fn.params) collect_layout_summary_type(descriptors, param.type);
        collect_layout_summary_stmts(descriptors, fn.body.statements);
    }
    return ordered_descriptors(std::move(descriptors));
}

void validate_module_cache_ir_layout_descriptors(
    const std::vector<ModuleCacheIrLayoutDescriptor>& descriptors,
    const std::vector<ModuleCacheIrFunctionSummary>& functions) {
    std::map<std::string, ModuleCacheIrLayoutDescriptor> actual;
    for (const auto& descriptor : descriptors) {
        require_valid_descriptor_shape(descriptor);
        std::string key = descriptor_key(descriptor);
        if (!actual.emplace(key, descriptor).second) {
            throw CompileError("duplicate IR layout descriptor for " + descriptor.type);
        }
    }

    std::vector<ModuleCacheIrLayoutDescriptor> expected =
        module_cache_ir_layout_descriptors(functions);
    for (const auto& descriptor : expected) {
        auto found = actual.find(descriptor_key(descriptor));
        if (found == actual.end()) {
            throw CompileError("missing IR layout descriptor for " + descriptor.type);
        }
        if (found->second.element_type != descriptor.element_type ||
            found->second.slot_count != descriptor.slot_count) {
            throw CompileError("IR layout descriptor mismatch for " + descriptor.type);
        }
    }
    for (const auto& item : actual) {
        auto expected_found = std::find_if(
            expected.begin(),
            expected.end(),
            [&](const ModuleCacheIrLayoutDescriptor& expected_descriptor) {
                return descriptor_key(expected_descriptor) == item.first;
            });
        if (expected_found == expected.end()) {
            throw CompileError("unused IR layout descriptor for " + item.second.type);
        }
    }
}

} // namespace ari
