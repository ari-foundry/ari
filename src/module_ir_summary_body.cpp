#include "module_ir_summary_body.hpp"

#include "common.hpp"
#include "module_ir_type_summary.hpp"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ari {

namespace {

struct IrBodyShape {
    std::map<std::string, std::uint64_t> statements;
    std::map<std::string, std::uint64_t> expressions;
};

std::string bool_key(bool value) {
    return value ? "1" : "0";
}

void append_count(std::string& out, std::uint64_t value) {
    out += std::to_string(value);
    out.push_back(';');
}

void append_field(std::string& out, const std::string& value) {
    out += std::to_string(value.size());
    out.push_back(':');
    out += value;
    out.push_back(';');
}

void append_type(std::string& out, const IrType& type) {
    append_field(out, module_cache_ir_type_name(type));
}

std::string stmt_kind_name(IrStmtKind kind) {
    switch (kind) {
        case IrStmtKind::Block: return "block";
        case IrStmtKind::VarDecl: return "var";
        case IrStmtKind::Assign: return "assign";
        case IrStmtKind::ExprStmt: return "expr";
        case IrStmtKind::Return: return "return";
        case IrStmtKind::If: return "if";
        case IrStmtKind::While: return "while";
        case IrStmtKind::WhileLet: return "while-let";
        case IrStmtKind::ForRange: return "for-range";
        case IrStmtKind::ForVector: return "for-vector";
        case IrStmtKind::InitWhile: return "init-while";
        case IrStmtKind::Continue: return "continue";
        case IrStmtKind::Break: return "break";
        case IrStmtKind::Match: return "match";
        case IrStmtKind::Drop: return "drop";
    }
    return "unknown";
}

std::string expr_kind_name(IrExprKind kind) {
    switch (kind) {
        case IrExprKind::Integer: return "integer";
        case IrExprKind::Float: return "float";
        case IrExprKind::String: return "string";
        case IrExprKind::Bool: return "bool";
        case IrExprKind::Null: return "null";
        case IrExprKind::FunctionRef: return "function-ref";
        case IrExprKind::Local: return "local";
        case IrExprKind::Borrow: return "borrow";
        case IrExprKind::Unary: return "unary";
        case IrExprKind::Cast: return "cast";
        case IrExprKind::PointerOffset: return "pointer-offset";
        case IrExprKind::PointerAdd: return "pointer-add";
        case IrExprKind::PointerLoad: return "pointer-load";
        case IrExprKind::PointerStore: return "pointer-store";
        case IrExprKind::Try: return "try";
        case IrExprKind::NullCoalesce: return "null-coalesce";
        case IrExprKind::EnumTag: return "enum-tag";
        case IrExprKind::EnumConstruct: return "enum-construct";
        case IrExprKind::Tuple: return "tuple";
        case IrExprKind::TupleIndex: return "tuple-index";
        case IrExprKind::Index: return "index";
        case IrExprKind::SliceRange: return "slice-range";
        case IrExprKind::Vector: return "vector";
        case IrExprKind::VectorPush: return "vector-push";
        case IrExprKind::VectorPop: return "vector-pop";
        case IrExprKind::VectorReserve: return "vector-reserve";
        case IrExprKind::VectorClear: return "vector-clear";
        case IrExprKind::VectorTruncate: return "vector-truncate";
        case IrExprKind::VectorSet: return "vector-set";
        case IrExprKind::VectorSwap: return "vector-swap";
        case IrExprKind::VectorRemove: return "vector-remove";
        case IrExprKind::VectorInsert: return "vector-insert";
        case IrExprKind::VectorContains: return "vector-contains";
        case IrExprKind::VectorIndexOf: return "vector-index-of";
        case IrExprKind::VectorCount: return "vector-count";
        case IrExprKind::Noop: return "noop";
        case IrExprKind::FormatPrint: return "format-print";
        case IrExprKind::Match: return "match";
        case IrExprKind::If: return "if";
        case IrExprKind::Block: return "block";
        case IrExprKind::IndirectCall: return "indirect-call";
        case IrExprKind::TraitObjectCall: return "trait-object-call";
        case IrExprKind::Binary: return "binary";
        case IrExprKind::Call: return "call";
    }
    return "unknown";
}

std::string binary_op_name(IrBinaryOp op) {
    switch (op) {
        case IrBinaryOp::LogicalOr: return "logical-or";
        case IrBinaryOp::LogicalAnd: return "logical-and";
        case IrBinaryOp::Add: return "add";
        case IrBinaryOp::Sub: return "sub";
        case IrBinaryOp::Mul: return "mul";
        case IrBinaryOp::Div: return "div";
        case IrBinaryOp::Mod: return "mod";
        case IrBinaryOp::BitAnd: return "bit-and";
        case IrBinaryOp::BitOr: return "bit-or";
        case IrBinaryOp::BitXor: return "bit-xor";
        case IrBinaryOp::Shl: return "shl";
        case IrBinaryOp::Shr: return "shr";
        case IrBinaryOp::Eq: return "eq";
        case IrBinaryOp::Ne: return "ne";
        case IrBinaryOp::Lt: return "lt";
        case IrBinaryOp::Le: return "le";
        case IrBinaryOp::Gt: return "gt";
        case IrBinaryOp::Ge: return "ge";
    }
    return "unknown";
}

std::string unary_op_name(IrUnaryOp op) {
    switch (op) {
        case IrUnaryOp::Not: return "not";
        case IrUnaryOp::BitNot: return "bit-not";
    }
    return "unknown";
}

std::string signed_integer_payload(bool negative, std::uint64_t value) {
    std::string text;
    if (negative) text.push_back('-');
    text += std::to_string(value);
    return text;
}

std::string float_payload(double value) {
    std::ostringstream out;
    out << std::scientific << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
    return out.str();
}

std::string expr_scalar_payload(const IrExpr& expr) {
    switch (expr.kind) {
        case IrExprKind::Integer:
            return signed_integer_payload(expr.int_negative, expr.int_value);
        case IrExprKind::Float:
            return float_payload(expr.float_value);
        case IrExprKind::Bool:
            return bool_key(expr.bool_value);
        case IrExprKind::TupleIndex:
            return std::to_string(expr.tuple_index);
        default:
            return "";
    }
}

void collect_body_shape_expr(IrBodyShape& shape, const IrExprPtr& expr);
void collect_body_shape_stmts(IrBodyShape& shape, const std::vector<IrStmtPtr>& statements);

void collect_body_shape_exprs(IrBodyShape& shape, const std::vector<IrExprPtr>& expressions) {
    for (const auto& expr : expressions) collect_body_shape_expr(shape, expr);
}

void collect_body_shape_stmts(IrBodyShape& shape, const std::vector<IrStmtPtr>& statements) {
    for (const auto& statement : statements) {
        if (!statement) continue;
        ++shape.statements[stmt_kind_name(statement->kind)];
        collect_body_shape_expr(shape, statement->binding.init);
        collect_body_shape_expr(shape, ir_stmt_assign_target(*statement));
        collect_body_shape_expr(shape, ir_stmt_assign_rhs(*statement));
        collect_body_shape_expr(shape, statement->expr);
        collect_body_shape_expr(shape, statement->condition);
        collect_body_shape_stmts(shape, ir_stmt_statements(*statement));
        collect_body_shape_stmts(shape, ir_stmt_then_body(*statement));
        collect_body_shape_stmts(shape, ir_stmt_else_body(*statement));
        collect_body_shape_stmts(shape, ir_stmt_loop_body(*statement));
        collect_body_shape_expr(shape, ir_stmt_for_start(*statement));
        collect_body_shape_expr(shape, ir_stmt_for_end(*statement));
        collect_body_shape_exprs(shape, ir_stmt_for_values(*statement));
        collect_body_shape_expr(shape, statement->match_value);
        for (const auto& binding : statement->init_bindings) collect_body_shape_expr(shape, binding.init);
        collect_body_shape_exprs(shape, statement->updates);
        for (const auto& arm : ir_stmt_match_arms(*statement)) collect_body_shape_stmts(shape, arm.body);
        collect_body_shape_expr(shape, ir_stmt_break_value(*statement));
    }
}

void collect_body_shape_expr(IrBodyShape& shape, const IrExprPtr& expr) {
    if (!expr) return;
    ++shape.expressions[expr_kind_name(expr->kind)];
    collect_body_shape_expr(shape, ir_expr_operand(*expr));
    collect_body_shape_expr(shape, ir_expr_left(*expr));
    collect_body_shape_expr(shape, ir_expr_right(*expr));
    collect_body_shape_expr(shape, ir_expr_payload(*expr));
    for (const auto& arg : expr->args) collect_body_shape_expr(shape, arg);
    collect_body_shape_expr(shape, ir_expr_if_condition(*expr));
    collect_body_shape_stmts(shape, ir_expr_if_then_body(*expr));
    collect_body_shape_expr(shape, ir_expr_if_then_value(*expr));
    collect_body_shape_stmts(shape, ir_expr_if_else_body(*expr));
    collect_body_shape_expr(shape, ir_expr_if_else_value(*expr));
    collect_body_shape_expr(shape, ir_expr_block_value(*expr));
    collect_body_shape_stmts(shape, ir_expr_block_body(*expr));
    collect_body_shape_expr(shape, ir_expr_match_value(*expr));
    for (const auto& arm : ir_expr_match_arms(*expr)) {
        collect_body_shape_stmts(shape, arm.body);
        collect_body_shape_expr(shape, arm.value);
    }
    collect_body_shape_stmts(shape, ir_expr_try_residual_cleanup(*expr));
}

void append_count_map(std::string& out, const std::map<std::string, std::uint64_t>& counts) {
    append_count(out, counts.size());
    for (const auto& item : counts) {
        append_field(out, item.first);
        append_count(out, item.second);
    }
}

void append_expr_tree(std::string& out, const IrExprPtr& expr);
void append_stmt_tree(std::string& out, const IrStmtPtr& stmt);

void append_expr_tree_list(std::string& out, const std::vector<IrExprPtr>& expressions) {
    append_count(out, expressions.size());
    for (const auto& expr : expressions) append_expr_tree(out, expr);
}

void append_stmt_tree_list(std::string& out, const std::vector<IrStmtPtr>& statements) {
    append_count(out, statements.size());
    for (const auto& stmt : statements) append_stmt_tree(out, stmt);
}

void append_type_list(std::string& out, const std::vector<IrType>& types) {
    append_count(out, types.size());
    for (const auto& type : types) append_type(out, type);
}

void append_binding_snapshot(std::string& out, const IrBinding& binding) {
    append_field(out, binding.name);
    append_type(out, binding.type);
    append_field(out, bool_key(binding.mutable_binding));
    append_expr_tree(out, binding.init);
}

void append_payload_binding(std::string& out, const IrPayloadBinding& binding) {
    append_count(out, binding.index);
    append_field(out, binding.name);
    append_type(out, binding.type);
    append_field(out, bool_key(binding.compact_enum_payload));
    append_type(out, binding.compact_enum_type);
    append_count(out, binding.compact_enum_payload_index);
}

void append_payload_bindings(std::string& out, const std::vector<IrPayloadBinding>& bindings) {
    append_count(out, bindings.size());
    for (const auto& binding : bindings) append_payload_binding(out, binding);
}

void append_payload_literal_condition(std::string& out, const IrPayloadLiteralCondition& condition) {
    append_count(out, condition.index);
    append_field(out, bool_key(condition.is_bool));
    append_field(out, condition.is_bool ? bool_key(condition.bool_literal())
                                        : std::to_string(condition.literal.integer));
}

void append_payload_range_condition(std::string& out, const IrPayloadRangeCondition& condition) {
    append_count(out, condition.index);
    append_field(out, signed_integer_payload(condition.start_negative, condition.start_int));
    append_field(out, signed_integer_payload(condition.end_negative, condition.end_int));
    append_field(out, bool_key(condition.inclusive));
    append_field(out, bool_key(condition.is_unsigned));
    append_type(out, condition.type);
    append_field(out, bool_key(condition.compact_enum_payload));
}

void append_payload_enum_condition(std::string& out, const IrPayloadEnumCondition& condition) {
    append_count(out, condition.index);
    append_type(out, condition.enum_type);
    append_count(out, condition.tag);
    append_count(out, condition.nested_payload_index);
    append_field(out, bool_key(condition.has_payload_literal));
    append_field(out, bool_key(condition.payload_literal_is_bool));
    append_field(out, condition.payload_literal_is_bool ? bool_key(condition.payload_literal.boolean)
                                                        : std::to_string(condition.payload_literal.integer));
    append_field(out, bool_key(condition.payload_literal_negative));
    append_field(out, bool_key(condition.has_payload_range));
    append_field(out, signed_integer_payload(condition.range_start_negative, condition.range_start_int));
    append_field(out, signed_integer_payload(condition.range_end_negative, condition.range_end_int));
    append_field(out, bool_key(condition.range_inclusive));
    append_field(out, bool_key(condition.range_is_unsigned));
    append_type(out, condition.payload_type);
}

template <typename Arm>
void append_match_arm_pattern(std::string& out, const Arm& arm) {
    append_field(out, bool_key(arm.wildcard));
    append_field(out, bool_key(arm.has_literal));
    append_field(out, bool_key(arm.literal_is_bool));
    std::string literal;
    if (arm.has_literal) {
        literal = arm.literal_is_bool ? bool_key(arm.literal_bool)
                                      : signed_integer_payload(arm.literal_negative, arm.literal_int);
    }
    append_field(out, literal);
    append_field(out, bool_key(arm.literal_negative));
    append_field(out, bool_key(arm.has_range));
    append_field(out, signed_integer_payload(arm.range_start_negative, arm.range_start_int));
    append_field(out, signed_integer_payload(arm.range_end_negative, arm.range_end_int));
    append_field(out, bool_key(arm.range_inclusive));
    append_field(out, bool_key(arm.range_is_unsigned));
    append_field(out, arm.case_name);
    append_count(out, arm.enum_tag);
    append_field(out, bool_key(arm.has_value_binding));
    append_field(out, arm.value_name);
    append_type(out, arm.value_type);
    append_field(out, bool_key(arm.has_payload_binding));
    append_field(out, arm.payload_name);
    append_type(out, arm.payload_type);
    append_count(out, arm.payload_index);
    append_payload_bindings(out, arm.payload_bindings);
    append_count(out, arm.payload_literal_conditions.size());
    for (const auto& condition : arm.payload_literal_conditions) {
        append_payload_literal_condition(out, condition);
    }
    append_count(out, arm.payload_range_conditions.size());
    for (const auto& condition : arm.payload_range_conditions) {
        append_payload_range_condition(out, condition);
    }
    append_count(out, arm.payload_enum_conditions.size());
    for (const auto& condition : arm.payload_enum_conditions) {
        append_payload_enum_condition(out, condition);
    }
}

void append_stmt_match_arm_tree(std::string& out, const IrMatchArm& arm) {
    append_match_arm_pattern(out, arm);
    append_stmt_tree_list(out, arm.body);
}

void append_expr_match_arm_tree(std::string& out, const IrMatchExprArm& arm) {
    append_match_arm_pattern(out, arm);
    append_stmt_tree_list(out, arm.body);
    append_expr_tree(out, arm.value);
}

void append_format_print_payload(std::string& out, const IrExpr& expr) {
    const std::vector<std::string>& parts = ir_expr_format_parts(expr);
    const std::vector<IrFormatSpec>& specs = ir_expr_format_specs(expr);
    append_count(out, parts.size());
    for (const auto& part : parts) append_field(out, part);
    append_count(out, specs.size());
    for (const auto& spec : specs) append_field(out, std::to_string(spec.precision));
    append_field(out, bool_key(ir_expr_format_print_newline(expr)));
}

void append_expr_tree(std::string& out, const IrExprPtr& expr) {
    if (!expr) {
        out += "N;";
        return;
    }
    out += "E;";
    append_field(out, expr_kind_name(expr->kind));
    append_type(out, expr->type);
    append_field(out, ir_expr_name(*expr));
    append_field(out, ir_expr_label(*expr));
    append_field(out, ir_expr_string_value(*expr));
    append_field(out, ir_expr_borrow_source_name(*expr));
    append_field(out, ir_expr_borrow_source_path(*expr));
    append_field(out, ir_expr_enum_name(*expr));
    append_field(out, ir_expr_case_name(*expr));
    append_field(out, expr_scalar_payload(*expr));
    append_field(out, unary_op_name(expr->unary_op));
    append_field(out, binary_op_name(expr->op));
    append_field(out, bool_key(expr->mutable_borrow));
    append_count(out, ir_expr_enum_tag(*expr));
    append_field(out, bool_key(ir_expr_has_enum_payload(*expr)));
    append_type(out, ir_expr_enum_payload_type(*expr));
    append_expr_tree(out, ir_expr_operand(*expr));
    append_expr_tree(out, ir_expr_left(*expr));
    append_expr_tree(out, ir_expr_right(*expr));
    append_expr_tree(out, ir_expr_payload(*expr));
    append_expr_tree_list(out, expr->args);
    append_type_list(out, ir_expr_call_param_types(*expr));
    append_expr_tree(out, ir_expr_if_condition(*expr));
    append_stmt_tree_list(out, ir_expr_if_then_body(*expr));
    append_expr_tree(out, ir_expr_if_then_value(*expr));
    append_stmt_tree_list(out, ir_expr_if_else_body(*expr));
    append_expr_tree(out, ir_expr_if_else_value(*expr));
    append_field(out, ir_expr_block_label(*expr));
    append_stmt_tree_list(out, ir_expr_block_body(*expr));
    append_expr_tree(out, ir_expr_block_value(*expr));
    append_expr_tree(out, ir_expr_match_value(*expr));
    append_count(out, ir_expr_match_arms(*expr).size());
    for (const auto& arm : ir_expr_match_arms(*expr)) append_expr_match_arm_tree(out, arm);
    append_format_print_payload(out, *expr);
    append_field(out, bool_key(ir_expr_try_converts_residual(*expr)));
    append_field(out, bool_key(ir_expr_try_residual_has_payload(*expr)));
    append_type(out, ir_expr_try_return_residual_payload_type(*expr));
    append_count(out, ir_expr_try_return_residual_tag(*expr));
    append_stmt_tree_list(out, ir_expr_try_residual_cleanup(*expr));
}

void append_stmt_tree(std::string& out, const IrStmtPtr& stmt) {
    if (!stmt) {
        out += "N;";
        return;
    }
    out += "S;";
    append_field(out, stmt_kind_name(stmt->kind));
    append_field(out, ir_stmt_label(*stmt));
    append_field(out, ir_stmt_drop_name(*stmt));
    append_field(out, ir_stmt_assign_name(*stmt));
    append_binding_snapshot(out, stmt->binding);
    append_expr_tree(out, ir_stmt_assign_target(*stmt));
    append_expr_tree(out, ir_stmt_assign_rhs(*stmt));
    append_expr_tree(out, stmt->expr);
    append_expr_tree(out, stmt->condition);
    append_stmt_tree_list(out, ir_stmt_statements(*stmt));
    append_stmt_tree_list(out, ir_stmt_then_body(*stmt));
    append_stmt_tree_list(out, ir_stmt_else_body(*stmt));
    append_stmt_tree_list(out, ir_stmt_loop_body(*stmt));
    append_field(out, ir_stmt_for_binding_name(*stmt));
    append_field(out, ir_stmt_for_index_name(*stmt));
    append_field(out, ir_stmt_for_end_name(*stmt));
    append_type(out, ir_stmt_for_binding_type(*stmt));
    append_field(out, bool_key(ir_stmt_for_inclusive(*stmt)));
    append_expr_tree(out, ir_stmt_for_start(*stmt));
    append_expr_tree(out, ir_stmt_for_end(*stmt));
    append_expr_tree_list(out, ir_stmt_for_values(*stmt));
    append_expr_tree(out, stmt->match_value);
    append_count(out, stmt->init_bindings.size());
    for (const auto& binding : stmt->init_bindings) append_binding_snapshot(out, binding);
    append_expr_tree_list(out, stmt->updates);
    append_count(out, ir_stmt_match_arms(*stmt).size());
    for (const auto& arm : ir_stmt_match_arms(*stmt)) append_stmt_match_arm_tree(out, arm);
    append_field(out, ir_stmt_break_label(*stmt));
    append_expr_tree(out, ir_stmt_break_value(*stmt));
}

void collect_summary_body_shape_expr(IrBodyShape& shape, const ModuleCacheIrExprSummaryPtr& expr);
void collect_summary_body_shape_stmts(IrBodyShape& shape,
                                      const std::vector<ModuleCacheIrStmtSummaryPtr>& statements);

void collect_summary_body_shape_exprs(IrBodyShape& shape,
                                      const std::vector<ModuleCacheIrExprSummaryPtr>& expressions) {
    for (const auto& expr : expressions) collect_summary_body_shape_expr(shape, expr);
}

void collect_summary_body_shape_stmts(IrBodyShape& shape,
                                      const std::vector<ModuleCacheIrStmtSummaryPtr>& statements) {
    for (const auto& statement : statements) {
        if (!statement) continue;
        ++shape.statements[statement->kind];
        collect_summary_body_shape_expr(shape, statement->binding.init);
        collect_summary_body_shape_expr(shape, statement->assign_target);
        collect_summary_body_shape_expr(shape, statement->assign_rhs);
        collect_summary_body_shape_expr(shape, statement->expr);
        collect_summary_body_shape_expr(shape, statement->condition);
        collect_summary_body_shape_stmts(shape, statement->statements);
        collect_summary_body_shape_stmts(shape, statement->then_body);
        collect_summary_body_shape_stmts(shape, statement->else_body);
        collect_summary_body_shape_stmts(shape, statement->loop_body);
        collect_summary_body_shape_expr(shape, statement->for_start);
        collect_summary_body_shape_expr(shape, statement->for_end);
        collect_summary_body_shape_exprs(shape, statement->for_values);
        collect_summary_body_shape_expr(shape, statement->match_value);
        for (const auto& binding : statement->init_bindings) {
            collect_summary_body_shape_expr(shape, binding.init);
        }
        collect_summary_body_shape_exprs(shape, statement->updates);
        for (const auto& arm : statement->match_arms) {
            collect_summary_body_shape_stmts(shape, arm.body);
        }
        collect_summary_body_shape_expr(shape, statement->break_value);
    }
}

void collect_summary_body_shape_expr(IrBodyShape& shape, const ModuleCacheIrExprSummaryPtr& expr) {
    if (!expr) return;
    ++shape.expressions[expr->kind];
    collect_summary_body_shape_expr(shape, expr->operand);
    collect_summary_body_shape_expr(shape, expr->left);
    collect_summary_body_shape_expr(shape, expr->right);
    collect_summary_body_shape_expr(shape, expr->payload);
    collect_summary_body_shape_exprs(shape, expr->args);
    collect_summary_body_shape_expr(shape, expr->if_condition);
    collect_summary_body_shape_stmts(shape, expr->if_then_body);
    collect_summary_body_shape_expr(shape, expr->if_then_value);
    collect_summary_body_shape_stmts(shape, expr->if_else_body);
    collect_summary_body_shape_expr(shape, expr->if_else_value);
    collect_summary_body_shape_expr(shape, expr->block_value);
    collect_summary_body_shape_stmts(shape, expr->block_body);
    collect_summary_body_shape_expr(shape, expr->match_value);
    for (const auto& arm : expr->match_arms) {
        collect_summary_body_shape_stmts(shape, arm.body);
        collect_summary_body_shape_expr(shape, arm.value);
    }
    collect_summary_body_shape_stmts(shape, expr->try_residual_cleanup);
}

void require_summary_body_shape_matches_tree(const ModuleCacheIrBodySummary& body) {
    IrBodyShape shape;
    collect_summary_body_shape_stmts(shape, body.statements);
    if (shape.statements != body.statement_shape) {
        throw CompileError("IR summary body statement shape does not match body tree");
    }
    if (shape.expressions != body.expression_shape) {
        throw CompileError("IR summary body expression shape does not match body tree");
    }
}

class IrSummaryBodyReader {
public:
    IrSummaryBodyReader(const std::string& text, std::size_t& pos)
        : text_(text), pos_(pos) {}

    ModuleCacheIrBodySummary read_body() {
        ModuleCacheIrBodySummary body;
        expect("B;");
        body.statement_shape = read_count_map("statement");
        body.expression_shape = read_count_map("expression");
        expect("T;");
        body.statements = read_stmt_tree_list();
        require_summary_body_shape_matches_tree(body);
        return body;
    }

private:
    const std::string& text_;
    std::size_t& pos_;

    [[noreturn]] void fail(const std::string& message) const {
        throw CompileError("IR summary " + message);
    }

    void expect(const std::string& expected) {
        if (text_.compare(pos_, expected.size(), expected) != 0) {
            fail("expected '" + expected + "' header");
        }
        pos_ += expected.size();
    }

    bool peek(const std::string& expected) const {
        return text_.compare(pos_, expected.size(), expected) == 0;
    }

    void expect_char(char expected) {
        if (pos_ >= text_.size() || text_[pos_] != expected) {
            fail(std::string("expected '") + expected + "'");
        }
        ++pos_;
    }

    std::uint64_t read_unsigned_until(char delimiter, const std::string& label) {
        if (pos_ >= text_.size() || !std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
            fail("expected " + label);
        }
        std::uint64_t value = 0;
        while (pos_ < text_.size() && text_[pos_] != delimiter) {
            char c = text_[pos_++];
            if (!std::isdigit(static_cast<unsigned char>(c))) fail("expected " + label);
            std::uint64_t digit = static_cast<std::uint64_t>(c - '0');
            if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10) {
                fail(label + " is too large");
            }
            value = value * 10 + digit;
        }
        if (pos_ >= text_.size()) fail("unterminated " + label);
        expect_char(delimiter);
        return value;
    }

    std::uint64_t read_count() {
        return read_unsigned_until(';', "count");
    }

    std::string read_field() {
        std::uint64_t length = read_unsigned_until(':', "field length");
        if (length > text_.size() - pos_) fail("field extends past end of summary");
        std::size_t start = pos_;
        pos_ += static_cast<std::size_t>(length);
        expect_char(';');
        return text_.substr(start, static_cast<std::size_t>(length));
    }

    bool read_bool_field(const std::string& label) {
        std::string value = read_field();
        if (value != "0" && value != "1") fail("expected " + label + " boolean 0 or 1");
        return value == "1";
    }

    std::string read_non_empty_field(const std::string& label) {
        std::string value = read_field();
        if (value.empty()) fail(label + " cannot be empty");
        return value;
    }

    std::map<std::string, std::uint64_t> read_count_map(const std::string& label) {
        std::map<std::string, std::uint64_t> counts;
        std::uint64_t entry_count = read_count();
        for (std::uint64_t i = 0; i < entry_count; ++i) {
            std::string name = read_field();
            if (name.empty()) fail(label + " kind cannot be empty");
            counts.emplace(std::move(name), read_count());
        }
        return counts;
    }

    bool read_null_tree() {
        if (!peek("N;")) return false;
        expect("N;");
        return true;
    }

    std::vector<std::string> read_type_list() {
        std::vector<std::string> types;
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) types.push_back(read_field());
        return types;
    }

    std::vector<ModuleCacheIrExprSummaryPtr> read_expr_tree_list() {
        std::vector<ModuleCacheIrExprSummaryPtr> expressions;
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) expressions.push_back(read_expr_tree());
        return expressions;
    }

    std::vector<ModuleCacheIrStmtSummaryPtr> read_stmt_tree_list() {
        std::vector<ModuleCacheIrStmtSummaryPtr> statements;
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) statements.push_back(read_stmt_tree());
        return statements;
    }

    ModuleCacheIrBindingSummary read_binding_snapshot() {
        ModuleCacheIrBindingSummary binding;
        binding.name = read_field();
        binding.type = read_field();
        binding.mutable_binding = read_bool_field("binding mutability");
        binding.init = read_expr_tree();
        return binding;
    }

    ModuleCacheIrPayloadBindingSummary read_payload_binding() {
        ModuleCacheIrPayloadBindingSummary binding;
        binding.index = read_count();
        binding.name = read_field();
        binding.type = read_field();
        binding.compact_enum_payload = read_bool_field("compact enum payload");
        binding.compact_enum_type = read_field();
        binding.compact_enum_payload_index = read_count();
        return binding;
    }

    std::vector<ModuleCacheIrPayloadBindingSummary> read_payload_bindings() {
        std::vector<ModuleCacheIrPayloadBindingSummary> bindings;
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) bindings.push_back(read_payload_binding());
        return bindings;
    }

    std::vector<ModuleCacheIrPayloadLiteralConditionSummary> read_payload_literal_conditions() {
        std::vector<ModuleCacheIrPayloadLiteralConditionSummary> conditions;
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) {
            ModuleCacheIrPayloadLiteralConditionSummary condition;
            condition.index = read_count();
            condition.is_bool = read_bool_field("payload literal is-bool");
            condition.literal = read_field();
            conditions.push_back(std::move(condition));
        }
        return conditions;
    }

    std::vector<ModuleCacheIrPayloadRangeConditionSummary> read_payload_range_conditions() {
        std::vector<ModuleCacheIrPayloadRangeConditionSummary> conditions;
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) {
            ModuleCacheIrPayloadRangeConditionSummary condition;
            condition.index = read_count();
            condition.start = read_field();
            condition.end = read_field();
            condition.inclusive = read_bool_field("payload range inclusive");
            condition.is_unsigned = read_bool_field("payload range unsigned");
            condition.type = read_field();
            condition.compact_enum_payload = read_bool_field("compact enum payload");
            conditions.push_back(std::move(condition));
        }
        return conditions;
    }

    std::vector<ModuleCacheIrPayloadEnumConditionSummary> read_payload_enum_conditions() {
        std::vector<ModuleCacheIrPayloadEnumConditionSummary> conditions;
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) {
            ModuleCacheIrPayloadEnumConditionSummary condition;
            condition.index = read_count();
            condition.enum_type = read_field();
            condition.tag = read_count();
            condition.nested_payload_index = read_count();
            condition.has_payload_literal = read_bool_field("payload enum literal presence");
            condition.payload_literal_is_bool = read_bool_field("payload enum literal is-bool");
            condition.payload_literal = read_field();
            condition.payload_literal_negative = read_bool_field("payload enum literal negative");
            condition.has_payload_range = read_bool_field("payload enum range presence");
            condition.range_start = read_field();
            condition.range_end = read_field();
            condition.range_inclusive = read_bool_field("payload enum range inclusive");
            condition.range_is_unsigned = read_bool_field("payload enum range unsigned");
            condition.payload_type = read_field();
            conditions.push_back(std::move(condition));
        }
        return conditions;
    }

    ModuleCacheIrMatchArmPatternSummary read_match_arm_pattern() {
        ModuleCacheIrMatchArmPatternSummary pattern;
        pattern.wildcard = read_bool_field("match-arm wildcard");
        pattern.has_literal = read_bool_field("match-arm literal presence");
        pattern.literal_is_bool = read_bool_field("match-arm literal is-bool");
        pattern.literal = read_field();
        pattern.literal_negative = read_bool_field("match-arm literal negative");
        pattern.has_range = read_bool_field("match-arm range presence");
        pattern.range_start = read_field();
        pattern.range_end = read_field();
        pattern.range_inclusive = read_bool_field("match-arm range inclusive");
        pattern.range_is_unsigned = read_bool_field("match-arm range unsigned");
        pattern.case_name = read_field();
        pattern.enum_tag = read_count();
        pattern.has_value_binding = read_bool_field("match-arm value binding presence");
        pattern.value_name = read_field();
        pattern.value_type = read_field();
        pattern.has_payload_binding = read_bool_field("match-arm payload binding presence");
        pattern.payload_name = read_field();
        pattern.payload_type = read_field();
        pattern.payload_index = read_count();
        pattern.payload_bindings = read_payload_bindings();
        pattern.payload_literal_conditions = read_payload_literal_conditions();
        pattern.payload_range_conditions = read_payload_range_conditions();
        pattern.payload_enum_conditions = read_payload_enum_conditions();
        return pattern;
    }

    ModuleCacheIrStmtMatchArmSummary read_stmt_match_arm_tree() {
        ModuleCacheIrStmtMatchArmSummary arm;
        arm.pattern = read_match_arm_pattern();
        arm.body = read_stmt_tree_list();
        return arm;
    }

    ModuleCacheIrExprMatchArmSummary read_expr_match_arm_tree() {
        ModuleCacheIrExprMatchArmSummary arm;
        arm.pattern = read_match_arm_pattern();
        arm.body = read_stmt_tree_list();
        arm.value = read_expr_tree();
        return arm;
    }

    void read_format_print_payload(ModuleCacheIrExprSummary& expr) {
        std::uint64_t part_count = read_count();
        for (std::uint64_t i = 0; i < part_count; ++i) expr.format_parts.push_back(read_field());
        std::uint64_t spec_count = read_count();
        for (std::uint64_t i = 0; i < spec_count; ++i) expr.format_specs.push_back(read_field());
        expr.format_print_newline = read_bool_field("format print newline");
    }

    ModuleCacheIrExprSummaryPtr read_expr_tree() {
        if (read_null_tree()) return nullptr;
        expect("E;");
        auto expr = std::make_unique<ModuleCacheIrExprSummary>();
        expr->kind = read_non_empty_field("expression kind");
        expr->type = read_field();
        expr->name = read_field();
        expr->label = read_field();
        expr->string_value = read_field();
        expr->borrow_source_name = read_field();
        expr->borrow_source_path = read_field();
        expr->enum_name = read_field();
        expr->case_name = read_field();
        expr->scalar_payload = read_field();
        expr->unary_op = read_field();
        expr->binary_op = read_field();
        expr->mutable_borrow = read_bool_field("mutable borrow");
        expr->enum_tag = read_count();
        expr->has_enum_payload = read_bool_field("enum payload presence");
        expr->enum_payload_type = read_field();
        expr->operand = read_expr_tree();
        expr->left = read_expr_tree();
        expr->right = read_expr_tree();
        expr->payload = read_expr_tree();
        expr->args = read_expr_tree_list();
        expr->call_param_types = read_type_list();
        expr->if_condition = read_expr_tree();
        expr->if_then_body = read_stmt_tree_list();
        expr->if_then_value = read_expr_tree();
        expr->if_else_body = read_stmt_tree_list();
        expr->if_else_value = read_expr_tree();
        expr->block_label = read_field();
        expr->block_body = read_stmt_tree_list();
        expr->block_value = read_expr_tree();
        expr->match_value = read_expr_tree();
        std::uint64_t arm_count = read_count();
        for (std::uint64_t i = 0; i < arm_count; ++i) {
            expr->match_arms.push_back(read_expr_match_arm_tree());
        }
        read_format_print_payload(*expr);
        expr->try_converts_residual = read_bool_field("try residual conversion");
        expr->try_residual_has_payload = read_bool_field("try residual payload presence");
        expr->try_return_residual_payload_type = read_field();
        expr->try_return_residual_tag = read_count();
        expr->try_residual_cleanup = read_stmt_tree_list();
        return expr;
    }

    ModuleCacheIrStmtSummaryPtr read_stmt_tree() {
        if (read_null_tree()) return nullptr;
        expect("S;");
        auto stmt = std::make_unique<ModuleCacheIrStmtSummary>();
        stmt->kind = read_non_empty_field("statement kind");
        stmt->label = read_field();
        stmt->drop_name = read_field();
        stmt->assign_name = read_field();
        stmt->binding = read_binding_snapshot();
        stmt->assign_target = read_expr_tree();
        stmt->assign_rhs = read_expr_tree();
        stmt->expr = read_expr_tree();
        stmt->condition = read_expr_tree();
        stmt->statements = read_stmt_tree_list();
        stmt->then_body = read_stmt_tree_list();
        stmt->else_body = read_stmt_tree_list();
        stmt->loop_body = read_stmt_tree_list();
        stmt->for_binding_name = read_field();
        stmt->for_index_name = read_field();
        stmt->for_end_name = read_field();
        stmt->for_binding_type = read_field();
        stmt->for_inclusive = read_bool_field("for inclusivity");
        stmt->for_start = read_expr_tree();
        stmt->for_end = read_expr_tree();
        stmt->for_values = read_expr_tree_list();
        stmt->match_value = read_expr_tree();
        std::uint64_t init_binding_count = read_count();
        for (std::uint64_t i = 0; i < init_binding_count; ++i) {
            stmt->init_bindings.push_back(read_binding_snapshot());
        }
        stmt->updates = read_expr_tree_list();
        std::uint64_t arm_count = read_count();
        for (std::uint64_t i = 0; i < arm_count; ++i) {
            stmt->match_arms.push_back(read_stmt_match_arm_tree());
        }
        stmt->break_label = read_field();
        stmt->break_value = read_expr_tree();
        return stmt;
    }
};

} // namespace

void append_ir_summary_body_shape(std::string& out, const std::vector<IrStmtPtr>& body) {
    IrBodyShape shape;
    collect_body_shape_stmts(shape, body);
    out += "B;";
    append_count_map(out, shape.statements);
    append_count_map(out, shape.expressions);
}

void append_ir_summary_body_tree(std::string& out, const std::vector<IrStmtPtr>& body) {
    out += "T;";
    append_stmt_tree_list(out, body);
}

ModuleCacheIrBodySummary read_module_cache_ir_summary_body_payload(const std::string& text,
                                                                   std::size_t& pos) {
    IrSummaryBodyReader reader(text, pos);
    return reader.read_body();
}

} // namespace ari
