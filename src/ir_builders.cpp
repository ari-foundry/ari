#include "ir_builders.hpp"

#include "common.hpp"

#include <utility>

namespace ari {
namespace {

IrType primitive_type(IrPrimitiveKind primitive, std::string name, SourceLocation loc) {
    IrType type;
    type.primitive = primitive;
    type.name = std::move(name);
    type.loc = loc;
    return type;
}

IrType bool_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::Bool, "bool", loc);
}

IrType void_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::Void, "void", loc);
}

bool same_ir_type(const IrType& left, const IrType& right) {
    if (left.qualifier != right.qualifier) return false;
    if (left.primitive != right.primitive) return false;
    if (left.name != right.name) return false;
    if (left.array_size != right.array_size) return false;
    if (left.args.size() != right.args.size()) return false;
    for (std::size_t i = 0; i < left.args.size(); ++i) {
        if (!same_ir_type(left.args[i], right.args[i])) return false;
    }
    return true;
}

const std::vector<IrType>& aggregate_field_types(const IrType& type) {
    if (type.primitive == IrPrimitiveKind::Struct ||
        type.primitive == IrPrimitiveKind::Array) {
        return type.field_types;
    }
    return type.args;
}

} // namespace

IrStmtPtr make_ir_var_decl(SourceLocation loc,
                           std::string name,
                           IrType type,
                           IrExprPtr init,
                           bool mutable_binding) {
    auto stmt = std::make_unique<IrStmt>();
    stmt->kind = IrStmtKind::VarDecl;
    stmt->loc = loc;
    stmt->binding.name = std::move(name);
    stmt->binding.loc = loc;
    stmt->binding.mutable_binding = mutable_binding;
    stmt->binding.type = std::move(type);
    stmt->binding.init = std::move(init);
    return stmt;
}

IrExprPtr make_local_lvalue_expr(SourceLocation loc, const std::string& name, const IrType& type) {
    auto base = std::make_unique<IrExpr>();
    base->kind = IrExprKind::Local;
    base->loc = loc;
    set_ir_expr_name(*base, name);
    base->type = type;
    return base;
}

IrExprPtr make_tuple_index_expr(SourceLocation loc,
                                const std::string& source_name,
                                const IrType& source_type,
                                std::size_t index) {
    return make_tuple_index_expr(loc, make_local_lvalue_expr(loc, source_name, source_type), index);
}

IrExprPtr make_tuple_index_expr(SourceLocation loc, IrExprPtr source, std::size_t index) {
    if (!source) {
        throw CompileError(where(loc) + ": internal error: tuple index expression requires a source");
    }
    const std::vector<IrType>& fields = aggregate_field_types(source->type);
    if (index >= fields.size()) {
        throw CompileError(where(loc) + ": internal error: tuple index expression out of range");
    }
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::TupleIndex;
    expr->loc = loc;
    expr->tuple_index = index;
    expr->type = fields[index];
    set_ir_expr_operand(*expr, std::move(source));
    return expr;
}

IrExprPtr make_vector_index_expr(SourceLocation loc,
                                 const std::string& source_name,
                                 const IrType& source_type,
                                 const std::string& index_name,
                                 const IrType& index_type) {
    if (source_type.primitive != IrPrimitiveKind::Vector || source_type.args.size() != 1) {
        throw CompileError(where(loc) + ": internal error: vector index expression requires a vector source");
    }
    return make_ir_index_expr(
        loc,
        make_local_lvalue_expr(loc, source_name, source_type),
        make_local_lvalue_expr(loc, index_name, index_type)
    );
}

IrExprPtr make_ir_index_expr(SourceLocation loc, IrExprPtr source, IrExprPtr index) {
    if (!source || !index || source->type.args.empty()) {
        throw CompileError(where(loc) + ": internal error: index expression requires an indexable source");
    }
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Index;
    expr->loc = loc;
    expr->type = source->type.args[0];
    set_ir_expr_operand(*expr, std::move(source));
    set_ir_expr_right(*expr, std::move(index));
    return expr;
}

IrExprPtr make_integer_literal(SourceLocation loc, const IrType& type, std::uint64_t value, bool negative) {
    auto literal = std::make_unique<IrExpr>();
    literal->kind = IrExprKind::Integer;
    literal->loc = loc;
    literal->type = type;
    literal->int_value = value;
    literal->int_negative = negative;
    return literal;
}

IrExprPtr make_integer_zero(SourceLocation loc, const IrType& type) {
    return make_integer_literal(loc, type, 0);
}

IrExprPtr make_bool_literal_expr(SourceLocation loc, bool value) {
    auto literal = std::make_unique<IrExpr>();
    literal->kind = IrExprKind::Bool;
    literal->loc = loc;
    literal->type = bool_type(loc);
    literal->bool_value = value;
    return literal;
}

IrExprPtr make_float_literal_expr(SourceLocation loc, const IrType& type, double value) {
    auto literal = std::make_unique<IrExpr>();
    literal->kind = IrExprKind::Float;
    literal->loc = loc;
    literal->type = type;
    literal->float_value = value;
    return literal;
}

IrExprPtr make_string_literal_expr(SourceLocation loc, const IrType& type, std::string value) {
    auto literal = std::make_unique<IrExpr>();
    literal->kind = IrExprKind::String;
    literal->loc = loc;
    literal->type = type;
    set_ir_expr_string_value(*literal, std::move(value));
    return literal;
}

IrExprPtr make_null_literal_expr(SourceLocation loc, const IrType& type) {
    auto literal = std::make_unique<IrExpr>();
    literal->kind = IrExprKind::Null;
    literal->loc = loc;
    literal->type = type;
    return literal;
}

IrExprPtr make_function_ref_expr(SourceLocation loc, std::string name, IrType type) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::FunctionRef;
    expr->loc = loc;
    set_ir_expr_name(*expr, std::move(name));
    expr->type = std::move(type);
    return expr;
}

IrExprPtr make_borrow_expr(SourceLocation loc,
                           std::string source_name,
                           std::string path,
                           IrExprPtr source,
                           bool mutable_borrow,
                           IrType borrowed_type) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Borrow;
    expr->loc = loc;
    set_ir_expr_name(*expr, std::move(source_name));
    set_ir_expr_label(*expr, std::move(path));
    set_ir_expr_operand(*expr, std::move(source));
    expr->mutable_borrow = mutable_borrow;
    expr->type = std::move(borrowed_type);
    expr->type.qualifier = mutable_borrow ? TypeQualifier::MutRef : TypeQualifier::Ref;
    return expr;
}

IrExprPtr make_ir_tuple_expr(SourceLocation loc, IrType type, std::vector<IrExprPtr> elements) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Tuple;
    expr->loc = loc;
    expr->type = std::move(type);
    expr->args = std::move(elements);
    return expr;
}

IrExprPtr make_bool_binary_expr(SourceLocation loc, IrBinaryOp op, IrExprPtr left, IrExprPtr right) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Binary;
    expr->loc = loc;
    expr->op = op;
    expr->type = bool_type(loc);
    set_ir_expr_left(*expr, std::move(left));
    set_ir_expr_right(*expr, std::move(right));
    return expr;
}

IrExprPtr make_cast_expr(SourceLocation loc, IrExprPtr value, const IrType& target) {
    if (same_ir_type(value->type, target)) return value;
    auto cast = std::make_unique<IrExpr>();
    cast->kind = IrExprKind::Cast;
    cast->loc = loc;
    cast->type = target;
    set_ir_expr_operand(*cast, std::move(value));
    return cast;
}

IrExprPtr make_trait_object_cast_expr(SourceLocation loc,
                                      IrExprPtr value,
                                      IrType target,
                                      std::string vtable_name,
                                      std::uint64_t vtable_offset) {
    auto cast = std::make_unique<IrExpr>();
    cast->kind = IrExprKind::Cast;
    cast->loc = loc;
    set_ir_expr_name(*cast, std::move(vtable_name));
    cast->tuple_index = vtable_offset;
    cast->type = std::move(target);
    set_ir_expr_operand(*cast, std::move(value));
    return cast;
}

IrExprPtr make_pointer_offset_expr(SourceLocation loc, IrExprPtr pointer, IrExprPtr offset) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::PointerOffset;
    expr->loc = loc;
    expr->type = pointer->type;
    set_ir_expr_operand(*expr, std::move(pointer));
    set_ir_expr_right(*expr, std::move(offset));
    return expr;
}

IrExprPtr make_pointer_add_expr(SourceLocation loc, IrExprPtr pointer, IrExprPtr offset) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::PointerAdd;
    expr->loc = loc;
    expr->type = pointer->type;
    set_ir_expr_operand(*expr, std::move(pointer));
    set_ir_expr_right(*expr, std::move(offset));
    return expr;
}

IrExprPtr make_pointer_load_expr(SourceLocation loc, IrExprPtr pointer, const IrType& result) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::PointerLoad;
    expr->loc = loc;
    expr->type = result;
    set_ir_expr_operand(*expr, std::move(pointer));
    return expr;
}

IrExprPtr make_pointer_store_expr(SourceLocation loc, IrExprPtr pointer, IrExprPtr value) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::PointerStore;
    expr->loc = loc;
    expr->type = void_type(loc);
    set_ir_expr_operand(*expr, std::move(pointer));
    set_ir_expr_right(*expr, std::move(value));
    return expr;
}

IrExprPtr make_ir_call_expr(SourceLocation loc,
                            std::string name,
                            IrType result,
                            std::vector<IrExprPtr> args) {
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::Call;
    lowered->loc = loc;
    set_ir_expr_name(*lowered, std::move(name));
    lowered->type = std::move(result);
    lowered->args = std::move(args);
    return lowered;
}

IrExprPtr make_builtin_call(SourceLocation loc,
                            const std::string& name,
                            std::vector<IrExprPtr> args,
                            const IrType& result) {
    return make_ir_call_expr(loc, name, result, std::move(args));
}

IrExprPtr make_format_print_expr(SourceLocation loc,
                                 IrType result,
                                 std::vector<std::string> format_parts,
                                 std::vector<IrExprPtr> args,
                                 bool print_newline) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::FormatPrint;
    expr->loc = loc;
    expr->type = std::move(result);
    expr->print_newline = print_newline;
    expr->format_parts = std::make_unique<std::vector<std::string>>(std::move(format_parts));
    expr->args = std::move(args);
    return expr;
}

IrExprPtr make_trait_object_call_expr(SourceLocation loc,
                                      std::string method_name,
                                      IrExprPtr receiver,
                                      std::uint64_t slot,
                                      IrType result,
                                      std::vector<IrType> erased_params,
                                      std::vector<IrExprPtr> args) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::TraitObjectCall;
    expr->loc = loc;
    set_ir_expr_name(*expr, std::move(method_name));
    expr->tuple_index = slot;
    expr->type = std::move(result);
    set_ir_expr_operand(*expr, std::move(receiver));
    set_ir_expr_call_param_types(*expr, std::move(erased_params));
    expr->args = std::move(args);
    return expr;
}

IrMatchExprArm make_match_expr_arm(IrMatchArm arm) {
    IrMatchExprArm expr_arm;
    expr_arm.wildcard = arm.wildcard;
    expr_arm.has_literal = arm.has_literal;
    expr_arm.literal_is_bool = arm.literal_is_bool;
    expr_arm.literal_negative = arm.literal_negative;
    if (arm.literal_is_bool) {
        expr_arm.literal_bool = arm.literal_bool;
    } else {
        expr_arm.literal_int = arm.literal_int;
    }
    expr_arm.has_range = arm.has_range;
    expr_arm.range_start_int = arm.range_start_int;
    expr_arm.range_start_negative = arm.range_start_negative;
    expr_arm.range_end_int = arm.range_end_int;
    expr_arm.range_end_negative = arm.range_end_negative;
    expr_arm.range_inclusive = arm.range_inclusive;
    expr_arm.range_is_unsigned = arm.range_is_unsigned;
    expr_arm.payload_literal_conditions = std::move(arm.payload_literal_conditions);
    expr_arm.payload_range_conditions = std::move(arm.payload_range_conditions);
    expr_arm.payload_enum_conditions = std::move(arm.payload_enum_conditions);
    expr_arm.case_name = std::move(arm.case_name);
    expr_arm.enum_tag = arm.enum_tag;
    expr_arm.has_value_binding = arm.has_value_binding;
    expr_arm.value_name = std::move(arm.value_name);
    expr_arm.value_type = std::move(arm.value_type);
    expr_arm.has_payload_binding = arm.has_payload_binding;
    expr_arm.payload_name = std::move(arm.payload_name);
    expr_arm.payload_type = std::move(arm.payload_type);
    expr_arm.payload_index = arm.payload_index;
    expr_arm.payload_bindings = std::move(arm.payload_bindings);
    expr_arm.loc = arm.loc;
    return expr_arm;
}

IrExprPtr make_ir_match_expr(SourceLocation loc, IrExprPtr value) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Match;
    expr->loc = loc;
    set_ir_expr_match_value(*expr, std::move(value));
    return expr;
}

IrExprPtr make_ir_block_expr(SourceLocation loc, std::string label) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Block;
    expr->loc = loc;
    set_ir_expr_block_label(*expr, std::move(label));
    return expr;
}

IrExprPtr make_ir_block_expr(SourceLocation loc,
                             std::string label,
                             IrType type,
                             std::vector<IrStmtPtr> body,
                             IrExprPtr value) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Block;
    expr->loc = loc;
    expr->type = std::move(type);
    set_ir_expr_block_payload(*expr, std::move(label), std::move(body), std::move(value));
    return expr;
}

IrExprPtr make_ir_if_expr(SourceLocation loc,
                          IrType type,
                          IrExprPtr condition,
                          std::vector<IrStmtPtr> then_body,
                          IrExprPtr then_value,
                          std::vector<IrStmtPtr> else_body,
                          IrExprPtr else_value) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::If;
    expr->loc = loc;
    expr->type = std::move(type);
    set_ir_expr_if_payload(
        *expr,
        std::move(condition),
        std::move(then_body),
        std::move(then_value),
        std::move(else_body),
        std::move(else_value));
    return expr;
}

} // namespace ari
