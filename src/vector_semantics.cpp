#include "vector_semantics.hpp"

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

IrType i64_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::I64, "i64", loc);
}

IrType bool_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::Bool, "bool", loc);
}

IrType void_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::Void, "void", loc);
}

IrExprPtr make_i64_literal(SourceLocation loc, std::uint64_t value) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Integer;
    expr->loc = loc;
    expr->type = i64_type(loc);
    expr->int_value = value;
    return expr;
}

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

} // namespace

bool is_vector_storage_type(const IrType& type) {
    return type.primitive == IrPrimitiveKind::Vector && type.args.size() == 1;
}

void specialize_vector_storage_from_init(IrType& declared, const IrExpr& init) {
    if (declared.primitive != IrPrimitiveKind::Vector ||
        init.type.primitive != IrPrimitiveKind::Vector ||
        declared.args.size() != 1 ||
        init.type.args.size() != 1 ||
        declared.array_size != 0 ||
        init.type.array_size == 0) {
        return;
    }
    declared.array_size = init.type.array_size;
}

void widen_vector_storage_type(IrType& type, std::uint64_t capacity) {
    if (!is_vector_storage_type(type) || capacity <= type.array_size) return;
    type.array_size = capacity;
}

void widen_vector_storage_literal(IrExpr& expr, std::uint64_t capacity) {
    if (expr.kind == IrExprKind::Vector && is_vector_storage_type(expr.type)) {
        widen_vector_storage_type(expr.type, capacity);
    }
}

IrExprPtr make_void_noop_expr(SourceLocation loc) {
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::Noop;
    lowered->loc = loc;
    lowered->type = void_type(loc);
    return lowered;
}

IrExprPtr make_vec_local_lvalue(SourceLocation loc, std::string name, IrType type) {
    auto local = std::make_unique<IrExpr>();
    local->kind = IrExprKind::Local;
    local->loc = loc;
    local->name = std::move(name);
    local->type = std::move(type);
    return local;
}

IrExprPtr make_vec_capacity_expr(SourceLocation loc, const IrType& type) {
    if (!is_vector_storage_type(type)) {
        fail(loc, "Vec.capacity requires local Vec storage");
    }
    return make_i64_literal(loc, type.array_size);
}

IrExprPtr make_vec_index_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec element access requires local Vec storage");
    }
    if (vector->type.args.empty()) {
        fail(loc, "Vec element access requires an element type");
    }
    if (!index) {
        fail(loc, "Vec element access expects an index");
    }
    auto out = std::make_unique<IrExpr>();
    out->kind = IrExprKind::Index;
    out->loc = loc;
    out->type = vector->type.args[0];
    out->operand = std::move(vector);
    out->right = std::move(index);
    return out;
}

IrExprPtr make_vec_pop_expr(SourceLocation loc, IrExprPtr vector) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.pop requires local Vec storage");
    }
    if (vector->type.args.empty()) {
        fail(loc, "Vec.pop requires an element type");
    }
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorPop;
    lowered->loc = loc;
    lowered->type = vector->type.args[0];
    lowered->operand = std::move(vector);
    return lowered;
}

IrExprPtr make_vec_clear_expr(SourceLocation loc, IrExprPtr vector) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.clear requires local Vec storage");
    }
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorClear;
    lowered->loc = loc;
    lowered->type = void_type(loc);
    lowered->operand = std::move(vector);
    return lowered;
}

IrExprPtr make_vec_truncate_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr new_length) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.truncate requires local Vec storage");
    }
    if (!new_length) {
        fail(loc, "Vec.truncate expects an integer length");
    }
    switch (new_length->type.primitive) {
        case IrPrimitiveKind::I8:
        case IrPrimitiveKind::I16:
        case IrPrimitiveKind::I32:
        case IrPrimitiveKind::I64:
        case IrPrimitiveKind::U8:
        case IrPrimitiveKind::U16:
        case IrPrimitiveKind::U32:
        case IrPrimitiveKind::U64:
            break;
        default:
            fail(loc, "Vec.truncate expects an integer length");
    }
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorTruncate;
    lowered->loc = loc;
    lowered->type = void_type(loc);
    lowered->operand = std::move(vector);
    lowered->right = std::move(new_length);
    return lowered;
}

IrExprPtr make_vec_set_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index, IrExprPtr value) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.set requires local Vec storage");
    }
    if (!index || !value) {
        fail(loc, "Vec.set expects an index and value");
    }
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorSet;
    lowered->loc = loc;
    lowered->type = void_type(loc);
    lowered->operand = std::move(vector);
    lowered->right = std::move(index);
    lowered->payload = std::move(value);
    return lowered;
}

IrExprPtr make_vec_swap_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr first_index, IrExprPtr second_index) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.swap requires local Vec storage");
    }
    if (!first_index || !second_index) {
        fail(loc, "Vec.swap expects two indexes");
    }
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorSwap;
    lowered->loc = loc;
    lowered->type = void_type(loc);
    lowered->operand = std::move(vector);
    lowered->right = std::move(first_index);
    lowered->payload = std::move(second_index);
    return lowered;
}

IrExprPtr make_vec_remove_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.remove requires local Vec storage");
    }
    if (vector->type.args.empty()) {
        fail(loc, "Vec.remove requires an element type");
    }
    if (!index) {
        fail(loc, "Vec.remove expects an index");
    }
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorRemove;
    lowered->loc = loc;
    lowered->type = vector->type.args[0];
    lowered->operand = std::move(vector);
    lowered->right = std::move(index);
    return lowered;
}

IrExprPtr make_vec_insert_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index, IrExprPtr value) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.insert requires local Vec storage");
    }
    if (!index || !value) {
        fail(loc, "Vec.insert expects an index and value");
    }
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorInsert;
    lowered->loc = loc;
    lowered->type = void_type(loc);
    lowered->operand = std::move(vector);
    lowered->right = std::move(index);
    lowered->payload = std::move(value);
    return lowered;
}

IrExprPtr make_vec_contains_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr value) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.contains requires local Vec storage");
    }
    if (!value) {
        fail(loc, "Vec.contains expects a value");
    }
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorContains;
    lowered->loc = loc;
    lowered->type = bool_type(loc);
    lowered->operand = std::move(vector);
    lowered->payload = std::move(value);
    return lowered;
}

IrExprPtr make_vec_index_of_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr value) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.index_of requires local Vec storage");
    }
    if (!value) {
        fail(loc, "Vec.index_of expects a value");
    }
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorIndexOf;
    lowered->loc = loc;
    lowered->type = i64_type(loc);
    lowered->operand = std::move(vector);
    lowered->payload = std::move(value);
    return lowered;
}

IrExprPtr make_vec_count_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr value) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.count requires local Vec storage");
    }
    if (!value) {
        fail(loc, "Vec.count expects a value");
    }
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorCount;
    lowered->loc = loc;
    lowered->type = i64_type(loc);
    lowered->operand = std::move(vector);
    lowered->payload = std::move(value);
    return lowered;
}

IrExprPtr make_collection_is_empty_expr(SourceLocation loc, IrExprPtr length) {
    auto empty = std::make_unique<IrExpr>();
    empty->kind = IrExprKind::Binary;
    empty->loc = loc;
    empty->op = IrBinaryOp::Eq;
    empty->type = bool_type(loc);
    empty->left = std::move(length);
    empty->right = make_i64_literal(loc, 0);
    return empty;
}

} // namespace ari
