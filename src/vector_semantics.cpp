#include "vector_semantics.hpp"

#include "common.hpp"

#include <algorithm>
#include <array>
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

struct LocalVecMethodInfo {
    const char* name;
    LocalVecMethod method;
};

constexpr std::array<LocalVecMethodInfo, 19> kLocalVecMethods{{
    {"as_slice", LocalVecMethod::AsSlice},
    {"capacity", LocalVecMethod::Capacity},
    {"clear", LocalVecMethod::Clear},
    {"contains", LocalVecMethod::Contains},
    {"count", LocalVecMethod::Count},
    {"first", LocalVecMethod::First},
    {"get", LocalVecMethod::Get},
    {"index_of", LocalVecMethod::IndexOf},
    {"insert", LocalVecMethod::Insert},
    {"is_empty", LocalVecMethod::IsEmpty},
    {"last", LocalVecMethod::Last},
    {"len", LocalVecMethod::Len},
    {"pop", LocalVecMethod::Pop},
    {"push", LocalVecMethod::Push},
    {"remove", LocalVecMethod::Remove},
    {"reserve", LocalVecMethod::Reserve},
    {"set", LocalVecMethod::Set},
    {"swap", LocalVecMethod::Swap},
    {"truncate", LocalVecMethod::Truncate},
}};

const char* local_vec_method_name(LocalVecMethod method) {
    for (const auto& info : kLocalVecMethods) {
        if (info.method == method) return info.name;
    }
    return "<unknown>";
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

bool vector_literal_length(const IrExpr& expr, std::uint64_t& out) {
    if (expr.kind != IrExprKind::Vector) return false;
    out = static_cast<std::uint64_t>(expr.args.size());
    return true;
}

LocalVecMethod classify_local_vec_method(const std::string& method_name) {
    for (const auto& info : kLocalVecMethods) {
        if (method_name == info.name) return info.method;
    }
    return LocalVecMethod::Unknown;
}

void require_collection_len_function_shape(SourceLocation loc,
                                           std::size_t type_arg_count,
                                           std::size_t arg_count) {
    if (type_arg_count != 0) fail(loc, "len does not take type arguments");
    if (arg_count != 1) fail(loc, "len expects one array, Vec, or Slice value");
}

void require_collection_len_method_shape(SourceLocation loc,
                                         std::size_t type_arg_count,
                                         std::size_t arg_count) {
    if (type_arg_count != 0) fail(loc, "len does not take type arguments");
    if (arg_count != 0) fail(loc, "len expects no method arguments");
}

void require_collection_is_empty_method_shape(SourceLocation loc,
                                              std::size_t type_arg_count,
                                              std::size_t arg_count) {
    if (type_arg_count != 0) fail(loc, "is_empty does not take type arguments");
    if (arg_count != 0) fail(loc, "is_empty expects no method arguments");
}

void require_slice_view_method_shape(SourceLocation loc,
                                     std::size_t type_arg_count,
                                     std::size_t arg_count) {
    if (type_arg_count != 0) fail(loc, "as_slice does not take type arguments");
    if (arg_count != 0) fail(loc, "as_slice expects no arguments");
}

void require_local_vec_method_shape(SourceLocation loc,
                                    LocalVecMethod method,
                                    std::size_t type_arg_count,
                                    std::size_t arg_count) {
    if (method == LocalVecMethod::Unknown ||
        method == LocalVecMethod::AsSlice ||
        method == LocalVecMethod::IsEmpty ||
        method == LocalVecMethod::Len) {
        return;
    }

    std::string display = std::string("Vec.") + local_vec_method_name(method);
    if (type_arg_count != 0) {
        fail(loc, display + " does not take type arguments");
    }

    switch (method) {
        case LocalVecMethod::First:
        case LocalVecMethod::Last:
        case LocalVecMethod::Capacity:
        case LocalVecMethod::Pop:
        case LocalVecMethod::Clear:
            if (arg_count != 0) fail(loc, display + " expects no arguments");
            return;
        case LocalVecMethod::Get:
        case LocalVecMethod::Remove:
            if (arg_count != 1) fail(loc, display + " expects one index argument");
            return;
        case LocalVecMethod::Reserve:
            if (arg_count != 1) fail(loc, display + " expects one capacity argument");
            return;
        case LocalVecMethod::Truncate:
            if (arg_count != 1) fail(loc, display + " expects one length argument");
            return;
        case LocalVecMethod::Set:
        case LocalVecMethod::Insert:
            if (arg_count != 2) fail(loc, display + " expects an index and value");
            return;
        case LocalVecMethod::Swap:
            if (arg_count != 2) fail(loc, display + " expects two indexes");
            return;
        case LocalVecMethod::Contains:
        case LocalVecMethod::IndexOf:
        case LocalVecMethod::Count:
        case LocalVecMethod::Push:
            if (arg_count != 1) fail(loc, display + " expects one value argument");
            return;
        case LocalVecMethod::Unknown:
        case LocalVecMethod::AsSlice:
        case LocalVecMethod::IsEmpty:
        case LocalVecMethod::Len:
            return;
    }
}

std::string local_vec_api_freeze_message(const std::string& method_name) {
    std::string supported;
    for (std::size_t i = 0; i < kLocalVecMethods.size(); ++i) {
        if (i != 0) supported += ", ";
        supported += kLocalVecMethods[i].name;
    }
    return "local Vec method '" + method_name +
           "' is reserved for allocator-backed std collection APIs; supported temporary local Vec methods are: "
           + supported;
}

bool vector_known_length_after_truncate(std::uint64_t current_length,
                                        const StaticIntegerValue& requested_length,
                                        std::uint64_t& out) {
    if (requested_length.negative) return false;
    out = std::min(current_length, requested_length.value);
    return true;
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

IrExprPtr make_vec_reserve_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr requested_capacity) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.reserve requires local Vec storage");
    }
    if (!requested_capacity) {
        fail(loc, "Vec.reserve expects an integer capacity");
    }
    switch (requested_capacity->type.primitive) {
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
            fail(loc, "Vec.reserve expects an integer capacity");
    }

    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorReserve;
    lowered->loc = loc;
    lowered->type = void_type(loc);
    lowered->operand = std::move(vector);
    lowered->right = std::move(requested_capacity);
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
