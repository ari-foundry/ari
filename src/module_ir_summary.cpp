#include "module_ir_summary.hpp"

#include "common.hpp"
#include "module_cache_format.hpp"
#include "module_metadata.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace ari {

namespace {

struct IrSummaryCounts {
    std::uint64_t function_count = 0;
};

struct IrBodyShape {
    std::map<std::string, std::uint64_t> statements;
    std::map<std::string, std::uint64_t> expressions;
};

std::string bool_key(bool value) {
    return value ? "1" : "0";
}

std::string display_module_name(const std::string& module_name) {
    return module_name.empty() ? "<root>" : module_name;
}

std::string summary_display(const ModuleCacheIrSummary& summary) {
    return "'" + display_module_name(summary.module_name) + "' at '" + summary.path + "'";
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

void append_body_shape(std::string& out, const std::vector<IrStmtPtr>& body) {
    IrBodyShape shape;
    collect_body_shape_stmts(shape, body);
    out += "B;";
    append_count_map(out, shape.statements);
    append_count_map(out, shape.expressions);
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

void append_body_tree(std::string& out, const std::vector<IrStmtPtr>& body) {
    out += "T;";
    append_stmt_tree_list(out, body);
}

void append_function_summary(std::string& out, const IrFunction& fn) {
    out += "F;";
    append_field(out, fn.name);
    append_field(out, fn.module_name);
    append_field(out, fn.link_name);
    append_type(out, fn.return_type);
    append_count(out, fn.params.size());
    for (const auto& param : fn.params) {
        append_field(out, param.name);
        append_type(out, param.type);
    }
    append_count(out, fn.body.size());
    append_body_shape(out, fn.body);
    append_body_tree(out, fn.body);
    append_field(out, bool_key(fn.shared_export));
}

std::string ir_summary_payload(const std::vector<const IrFunction*>& functions) {
    std::string out = kModuleIrSummaryPayloadHeader;
    append_count(out, functions.size());
    for (const IrFunction* fn : functions) append_function_summary(out, *fn);
    return out;
}

class IrSummaryReader {
public:
    explicit IrSummaryReader(std::string text)
        : text_(std::move(text)) {}

    IrSummaryCounts parse() {
        expect(kModuleIrSummaryPayloadHeader);
        IrSummaryCounts counts;
        counts.function_count = read_count();
        for (std::uint64_t i = 0; i < counts.function_count; ++i) read_function();
        if (pos_ != text_.size()) fail("trailing data in IR summary");
        return counts;
    }

private:
    std::string text_;
    std::size_t pos_ = 0;

    [[noreturn]] void fail(const std::string& message) const {
        throw CompileError("IR summary " + message);
    }

    void expect(const std::string& expected) {
        if (text_.compare(pos_, expected.size(), expected) != 0) {
            fail("expected '" + expected + "' header");
        }
        pos_ += expected.size();
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

    void read_function() {
        expect("F;");
        read_field(); // name
        read_field(); // module name
        read_field(); // link name
        read_field(); // return type
        std::uint64_t param_count = read_count();
        for (std::uint64_t i = 0; i < param_count; ++i) {
            read_field(); // parameter name
            read_field(); // parameter type
        }
        read_count(); // body statement count
        if (peek("B;")) read_body_shape();
        if (peek("T;")) read_body_tree();
        std::string shared_export = read_field();
        if (shared_export != "0" && shared_export != "1") {
            fail("expected shared-export boolean 0 or 1");
        }
    }

    bool peek(const std::string& expected) const {
        return text_.compare(pos_, expected.size(), expected) == 0;
    }

    void read_count_map(const std::string& label) {
        std::uint64_t entry_count = read_count();
        for (std::uint64_t i = 0; i < entry_count; ++i) {
            std::string name = read_field();
            if (name.empty()) fail(label + " kind cannot be empty");
            read_count();
        }
    }

    void read_body_shape() {
        expect("B;");
        read_count_map("statement");
        read_count_map("expression");
    }

    bool read_null_tree() {
        if (!peek("N;")) return false;
        expect("N;");
        return true;
    }

    std::string read_non_empty_field(const std::string& label) {
        std::string value = read_field();
        if (value.empty()) fail(label + " cannot be empty");
        return value;
    }

    void read_type_list() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) read_field();
    }

    void read_expr_tree_list() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) read_expr_tree();
    }

    void read_stmt_tree_list() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) read_stmt_tree();
    }

    void read_binding_snapshot() {
        read_field(); // name
        read_field(); // type
        read_bool_field("binding mutability");
        read_expr_tree();
    }

    void read_payload_binding() {
        read_count(); // index
        read_field(); // name
        read_field(); // type
        read_bool_field("compact enum payload");
        read_field(); // compact enum type
        read_count(); // compact enum payload index
    }

    void read_payload_bindings() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) read_payload_binding();
    }

    void read_payload_literal_conditions() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) {
            read_count(); // index
            read_bool_field("payload literal is-bool");
            read_field(); // literal payload
        }
    }

    void read_payload_range_conditions() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) {
            read_count(); // index
            read_field(); // start
            read_field(); // end
            read_bool_field("payload range inclusive");
            read_bool_field("payload range unsigned");
            read_field(); // type
            read_bool_field("compact enum payload");
        }
    }

    void read_payload_enum_conditions() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) {
            read_count(); // index
            read_field(); // enum type
            read_count(); // tag
            read_count(); // nested payload index
            read_bool_field("payload enum literal presence");
            read_bool_field("payload enum literal is-bool");
            read_field(); // payload literal
            read_bool_field("payload enum literal negative");
            read_bool_field("payload enum range presence");
            read_field(); // range start
            read_field(); // range end
            read_bool_field("payload enum range inclusive");
            read_bool_field("payload enum range unsigned");
            read_field(); // payload type
        }
    }

    void read_match_arm_pattern() {
        read_bool_field("match-arm wildcard");
        read_bool_field("match-arm literal presence");
        read_bool_field("match-arm literal is-bool");
        read_field(); // literal payload
        read_bool_field("match-arm literal negative");
        read_bool_field("match-arm range presence");
        read_field(); // range start
        read_field(); // range end
        read_bool_field("match-arm range inclusive");
        read_bool_field("match-arm range unsigned");
        read_field(); // case name
        read_count(); // enum tag
        read_bool_field("match-arm value binding presence");
        read_field(); // value binding name
        read_field(); // value binding type
        read_bool_field("match-arm payload binding presence");
        read_field(); // payload binding name
        read_field(); // payload binding type
        read_count(); // payload index
        read_payload_bindings();
        read_payload_literal_conditions();
        read_payload_range_conditions();
        read_payload_enum_conditions();
    }

    void read_stmt_match_arm_tree() {
        read_match_arm_pattern();
        read_stmt_tree_list();
    }

    void read_expr_match_arm_tree() {
        read_match_arm_pattern();
        read_stmt_tree_list();
        read_expr_tree();
    }

    void read_format_print_payload() {
        std::uint64_t part_count = read_count();
        for (std::uint64_t i = 0; i < part_count; ++i) read_field();
        std::uint64_t spec_count = read_count();
        for (std::uint64_t i = 0; i < spec_count; ++i) read_field();
        read_bool_field("format print newline");
    }

    void read_expr_tree() {
        if (read_null_tree()) return;
        expect("E;");
        read_non_empty_field("expression kind");
        read_field(); // result type
        read_field(); // name
        read_field(); // label
        read_field(); // string value
        read_field(); // borrow source name
        read_field(); // borrow source path
        read_field(); // enum name
        read_field(); // case name
        read_field(); // scalar payload
        read_field(); // unary op
        read_field(); // binary op
        read_bool_field("mutable borrow");
        read_count(); // enum tag
        read_bool_field("enum payload presence");
        read_field(); // enum payload type
        read_expr_tree(); // operand
        read_expr_tree(); // left
        read_expr_tree(); // right
        read_expr_tree(); // payload
        read_expr_tree_list(); // args
        read_type_list(); // call param types
        read_expr_tree(); // if condition
        read_stmt_tree_list(); // then body
        read_expr_tree(); // then value
        read_stmt_tree_list(); // else body
        read_expr_tree(); // else value
        read_field(); // block label
        read_stmt_tree_list(); // block body
        read_expr_tree(); // block value
        read_expr_tree(); // match value
        std::uint64_t arm_count = read_count();
        for (std::uint64_t i = 0; i < arm_count; ++i) read_expr_match_arm_tree();
        read_format_print_payload();
        read_bool_field("try residual conversion");
        read_bool_field("try residual payload presence");
        read_field(); // try return residual payload type
        read_count(); // try return residual tag
        read_stmt_tree_list(); // try residual cleanup
    }

    void read_stmt_tree() {
        if (read_null_tree()) return;
        expect("S;");
        read_non_empty_field("statement kind");
        read_field(); // label
        read_field(); // drop name
        read_field(); // assign name
        read_binding_snapshot();
        read_expr_tree(); // assign target
        read_expr_tree(); // assign rhs
        read_expr_tree(); // expression
        read_expr_tree(); // condition
        read_stmt_tree_list(); // statements
        read_stmt_tree_list(); // then body
        read_stmt_tree_list(); // else body
        read_stmt_tree_list(); // loop body
        read_field(); // for binding name
        read_field(); // for index name
        read_field(); // for end name
        read_field(); // for binding type
        read_bool_field("for inclusivity");
        read_expr_tree(); // for start
        read_expr_tree(); // for end
        read_expr_tree_list(); // for values
        read_expr_tree(); // match value
        std::uint64_t init_binding_count = read_count();
        for (std::uint64_t i = 0; i < init_binding_count; ++i) read_binding_snapshot();
        read_expr_tree_list(); // updates
        std::uint64_t arm_count = read_count();
        for (std::uint64_t i = 0; i < arm_count; ++i) read_stmt_match_arm_tree();
        read_field(); // break label
        read_expr_tree(); // break value
    }

    void read_body_tree() {
        expect("T;");
        read_stmt_tree_list();
    }
};

IrSummaryCounts parse_ir_summary_payload(const ModuleCacheIrSummary& summary) {
    IrSummaryReader reader(summary.ir_summary);
    return reader.parse();
}

void require_count_match(std::uint64_t recorded,
                         std::uint64_t parsed,
                         const std::string& label,
                         const ModuleCacheIrSummary& summary) {
    if (recorded == parsed) return;
    throw CompileError("module cache IR summary for " + summary_display(summary) +
                       " has " + label + " count " + std::to_string(recorded) +
                       " but IR summary contains " + std::to_string(parsed));
}

std::vector<const IrFunction*> functions_for_module(const IrProgram& ir,
                                                    const std::string& module_name) {
    std::vector<const IrFunction*> functions;
    for (const auto& fn : ir.functions) {
        if (fn.module_name == module_name) functions.push_back(&fn);
    }
    std::sort(functions.begin(), functions.end(), [](const IrFunction* left, const IrFunction* right) {
        if (left->name != right->name) return left->name < right->name;
        return left->link_name < right->link_name;
    });
    return functions;
}

} // namespace

ModuleCacheIrSummary make_module_cache_ir_summary(const ModuleCacheSource& source,
                                                  const IrProgram& ir) {
    std::vector<const IrFunction*> functions = functions_for_module(ir, source.module_name);

    ModuleCacheIrSummary summary;
    summary.module_name = source.module_name;
    summary.path = source.path;
    summary.content_hash = source.content_hash;
    summary.ir_summary = ir_summary_payload(functions);
    summary.ir_hash = module_metadata_source_hash(summary.ir_summary);
    summary.is_root = source.is_root;
    summary.function_count = functions.size();
    return summary;
}

void attach_module_cache_ir_summaries(ModuleCache& cache, const IrProgram& ir) {
    cache.ir_summaries.clear();
    cache.ir_summaries.reserve(cache.sources.size());
    for (const auto& source : cache.sources) {
        cache.ir_summaries.push_back(make_module_cache_ir_summary(source, ir));
    }
}

void require_valid_module_cache_ir_summary_payload(const ModuleCacheIrSummary& summary,
                                                   const std::string& display_path) {
    if (summary.ir_summary.empty()) {
        throw CompileError("invalid module cache '" + display_path +
                           "': IR summary for " + summary_display(summary) +
                           " is missing an IR summary");
    }
    if (summary.ir_summary.compare(
            0,
            std::strlen(kModuleIrSummaryPayloadHeader),
            kModuleIrSummaryPayloadHeader
        ) != 0) {
        throw CompileError("invalid module cache '" + display_path +
                           "': IR summary for " + summary_display(summary) +
                           " expected '" + std::string(kModuleIrSummaryPayloadHeader) + "' header");
    }
    std::string hash = module_metadata_source_hash(summary.ir_summary);
    if (hash != summary.ir_hash) {
        throw CompileError("invalid module cache '" + display_path +
                           "': IR summary for " + summary_display(summary) +
                           " hashes to '" + hash + "' instead of recorded '" +
                           summary.ir_hash + "'");
    }
    try {
        IrSummaryCounts counts = parse_ir_summary_payload(summary);
        require_count_match(summary.function_count, counts.function_count, "function", summary);
    } catch (const CompileError& error) {
        throw CompileError("invalid module cache '" + display_path + "': " + error.what());
    }
}

} // namespace ari
