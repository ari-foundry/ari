#include "module_ir_summary_body.hpp"

#include <cstdint>
#include <map>
#include <string>
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
    append_field(out, type_name(type));
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

std::string expr_scalar_payload(const IrExpr& expr) {
    switch (expr.kind) {
        case IrExprKind::Integer:
            return signed_integer_payload(expr.int_negative, expr.int_value);
        case IrExprKind::Float:
            return std::to_string(expr.float_value);
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

} // namespace ari
