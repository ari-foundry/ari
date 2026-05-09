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
    base->name = name;
    base->type = type;
    return base;
}

IrExprPtr make_tuple_index_expr(SourceLocation loc,
                                const std::string& source_name,
                                const IrType& source_type,
                                std::size_t index) {
    const std::vector<IrType>& fields = aggregate_field_types(source_type);
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::TupleIndex;
    expr->loc = loc;
    expr->tuple_index = index;
    expr->type = fields[index];
    expr->operand = make_local_lvalue_expr(loc, source_name, source_type);
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
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Index;
    expr->loc = loc;
    expr->type = source_type.args[0];
    expr->operand = make_local_lvalue_expr(loc, source_name, source_type);
    expr->right = make_local_lvalue_expr(loc, index_name, index_type);
    return expr;
}

IrExprPtr make_integer_literal(SourceLocation loc, const IrType& type, std::uint64_t value) {
    auto literal = std::make_unique<IrExpr>();
    literal->kind = IrExprKind::Integer;
    literal->loc = loc;
    literal->type = type;
    literal->int_value = value;
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

IrExprPtr make_bool_binary_expr(SourceLocation loc, IrBinaryOp op, IrExprPtr left, IrExprPtr right) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Binary;
    expr->loc = loc;
    expr->op = op;
    expr->type = bool_type(loc);
    expr->left = std::move(left);
    expr->right = std::move(right);
    return expr;
}

IrExprPtr make_cast_expr(SourceLocation loc, IrExprPtr value, const IrType& target) {
    if (same_ir_type(value->type, target)) return value;
    auto cast = std::make_unique<IrExpr>();
    cast->kind = IrExprKind::Cast;
    cast->loc = loc;
    cast->type = target;
    cast->operand = std::move(value);
    return cast;
}

IrExprPtr make_builtin_call(SourceLocation loc,
                            const std::string& name,
                            std::vector<IrExprPtr> args,
                            const IrType& result) {
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::Call;
    lowered->loc = loc;
    lowered->name = name;
    lowered->type = result;
    lowered->args = std::move(args);
    return lowered;
}

} // namespace ari
