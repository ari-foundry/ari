#include "vector_semantics.hpp"

#include "ast.hpp"
#include "common.hpp"
#include "slice_semantics.hpp"
#include "type_semantics.hpp"

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

IrType array_storage_type(SourceLocation loc, const IrType& element, std::uint64_t length) {
    IrType type = primitive_type(IrPrimitiveKind::Array, "Array", loc);
    type.args.push_back(element);
    type.array_size = length;
    type.field_types.reserve(static_cast<std::size_t>(length));
    type.field_mutable.reserve(static_cast<std::size_t>(length));
    for (std::uint64_t i = 0; i < length; ++i) {
        type.field_types.push_back(element);
        type.field_mutable.push_back(false);
    }
    return type;
}

void refresh_vector_storage_fields(IrType& type) {
    type.field_names.clear();
    type.field_types.clear();
    type.field_mutable.clear();
    if (type.primitive != IrPrimitiveKind::Vector || type.args.size() != 1) return;

    type.field_names.push_back("len");
    type.field_types.push_back(i64_type(type.loc));
    type.field_mutable.push_back(true);
    type.field_names.push_back("data");
    type.field_types.push_back(array_storage_type(type.loc, type.args[0], type.array_size));
    type.field_mutable.push_back(true);
}

IrExprPtr make_i64_literal(SourceLocation loc, std::uint64_t value) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Integer;
    expr->loc = loc;
    expr->type = i64_type(loc);
    expr->int_value = value;
    return expr;
}

IrExprPtr make_bool_literal(SourceLocation loc, bool value) {
    auto expr = std::make_unique<IrExpr>();
    expr->kind = IrExprKind::Bool;
    expr->loc = loc;
    expr->type = bool_type(loc);
    expr->bool_value = value;
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

bool contains_root_vector_without_runtime_abi(const IrType& type) {
    if (type.primitive == IrPrimitiveKind::Vector &&
        type.args.size() == 1 &&
        type.array_size == 0) {
        return true;
    }
    for (const auto& arg : type.args) {
        if (contains_root_vector_without_runtime_abi(arg)) return true;
    }
    for (const auto& field : type.field_types) {
        if (contains_root_vector_without_runtime_abi(field)) return true;
    }
    return false;
}

void require_root_vector_runtime_abi(SourceLocation loc,
                                     const IrType& type,
                                     const std::string& context) {
    if (!contains_root_vector_without_runtime_abi(type)) return;
    fail(loc,
         "root Vec[T] cannot be used as " + context +
             " until the runtime-capacity Vec ABI is defined; use std::vec::Vec[T] with an explicit Zone handle or pass Slice[T]");
}

IrType make_vector_storage_type(SourceLocation loc, const IrType& element, std::uint64_t length) {
    IrType type = primitive_type(IrPrimitiveKind::Vector, "Vec", loc);
    type.args.push_back(element);
    type.array_size = length;
    refresh_vector_storage_fields(type);
    return type;
}

const IrType& require_typed_empty_vector_element_type(SourceLocation loc, const IrType& expected) {
    if (expected.qualifier != TypeQualifier::Value ||
        expected.primitive != IrPrimitiveKind::Vector ||
        expected.args.size() != 1) {
        fail(loc, "empty [] literals need an explicit Vec[T] type or non-empty array elements");
    }
    return expected.args[0];
}

void specialize_vector_storage_from_init(IrType& declared, const IrExpr& init) {
    std::uint64_t capacity = vector_storage_capacity_from_expr(init);
    if (declared.primitive != IrPrimitiveKind::Vector ||
        declared.args.size() != 1 ||
        declared.array_size != 0) {
        return;
    }
    declared.array_size = capacity;
    refresh_vector_storage_fields(declared);
}

void widen_vector_storage_type(IrType& type, std::uint64_t capacity) {
    if (!is_vector_storage_type(type) || capacity <= type.array_size) return;
    type.array_size = capacity;
    refresh_vector_storage_fields(type);
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

static bool merge_vector_known_length(VectorKnownLength& merged,
                                      bool& has_merged,
                                      const IrType& storage_type,
                                      const IrExprPtr& value);

static bool merge_labeled_break_vector_known_lengths(VectorKnownLength& merged,
                                                     bool& has_merged,
                                                     const IrType& storage_type,
                                                     const std::vector<IrStmtPtr>& statements,
                                                     const std::string& label);

static bool merge_labeled_break_vector_known_lengths(VectorKnownLength& merged,
                                                     bool& has_merged,
                                                     const IrType& storage_type,
                                                     const std::vector<IrMatchArm>& arms,
                                                     const std::string& label) {
    for (const auto& arm : arms) {
        if (!merge_labeled_break_vector_known_lengths(
                merged, has_merged, storage_type, arm.body, label)) {
            return false;
        }
    }
    return true;
}

static bool merge_labeled_break_vector_known_length(VectorKnownLength& merged,
                                                    bool& has_merged,
                                                    const IrType& storage_type,
                                                    const IrStmt& statement,
                                                    const std::string& label) {
    if (statement.kind == IrStmtKind::Break && ir_stmt_break_label(statement) == label) {
        return merge_vector_known_length(merged, has_merged, storage_type, ir_stmt_break_value(statement));
    }

    switch (statement.kind) {
        case IrStmtKind::Block:
            return merge_labeled_break_vector_known_lengths(
                merged, has_merged, storage_type, ir_stmt_statements(statement), label);
        case IrStmtKind::If:
            return merge_labeled_break_vector_known_lengths(
                       merged, has_merged, storage_type, ir_stmt_then_body(statement), label) &&
                   merge_labeled_break_vector_known_lengths(
                       merged, has_merged, storage_type, ir_stmt_else_body(statement), label);
        case IrStmtKind::While:
        case IrStmtKind::WhileLet:
        case IrStmtKind::ForRange:
        case IrStmtKind::ForVector:
        case IrStmtKind::InitWhile:
            return merge_labeled_break_vector_known_lengths(
                merged, has_merged, storage_type, ir_stmt_loop_body(statement), label);
        case IrStmtKind::Match:
            return merge_labeled_break_vector_known_lengths(
                merged, has_merged, storage_type, ir_stmt_match_arms(statement), label);
        default:
            return true;
    }
}

static bool merge_labeled_break_vector_known_lengths(VectorKnownLength& merged,
                                                     bool& has_merged,
                                                     const IrType& storage_type,
                                                     const std::vector<IrStmtPtr>& statements,
                                                     const std::string& label) {
    for (const auto& statement : statements) {
        if (!merge_labeled_break_vector_known_length(
                merged, has_merged, storage_type, *statement, label)) {
            return false;
        }
    }
    return true;
}

static bool merge_vector_known_length(VectorKnownLength& merged,
                                      bool& has_merged,
                                      const IrType& storage_type,
                                      const IrExprPtr& value) {
    if (!value) return false;
    VectorKnownLength length = vector_known_length_from_expr(storage_type, *value);
    if (!length.known) return false;
    if (!has_merged) {
        merged = length;
        has_merged = true;
        return true;
    }
    return merged.length == length.length;
}

VectorKnownLength vector_known_length_from_expr(const IrType& storage_type, const IrExpr& expr) {
    if (!is_vector_storage_type(storage_type)) return {};
    std::uint64_t length = 0;
    if (vector_literal_length(expr, length)) return VectorKnownLength{true, length};
    if (expr.kind == IrExprKind::Block && ir_expr_block_value(expr)) {
        VectorKnownLength merged;
        bool has_merged = false;
        if (!merge_vector_known_length(merged, has_merged, storage_type, ir_expr_block_value(expr))) {
            return {};
        }
        if (!ir_expr_block_label(expr).empty() &&
            !merge_labeled_break_vector_known_lengths(
                merged, has_merged, storage_type, ir_expr_block_body(expr), ir_expr_block_label(expr))) {
            return {};
        }
        return has_merged ? merged : VectorKnownLength{};
    }
    if (expr.kind == IrExprKind::If) {
        VectorKnownLength merged;
        bool has_merged = false;
        if (!merge_vector_known_length(merged, has_merged, storage_type, ir_expr_if_then_value(expr))) return {};
        if (!merge_vector_known_length(merged, has_merged, storage_type, ir_expr_if_else_value(expr))) return {};
        return has_merged ? merged : VectorKnownLength{};
    }
    if (expr.kind == IrExprKind::Match) {
        VectorKnownLength merged;
        bool has_merged = false;
        for (const auto& arm : ir_expr_match_arms(expr)) {
            if (!merge_vector_known_length(merged, has_merged, storage_type, arm.value)) return {};
        }
        return has_merged ? merged : VectorKnownLength{};
    }
    return {};
}

static bool merge_source_vector_known_length(VectorKnownLength& merged,
                                             bool& has_merged,
                                             const ExprPtr& source,
                                             const VectorKnownLengthLookup& lookup);

static bool merge_labeled_break_source_vector_known_lengths(VectorKnownLength& merged,
                                                            bool& has_merged,
                                                            const VectorKnownLengthLookup& lookup,
                                                            const std::vector<StmtPtr>& statements,
                                                            const std::string& label);

static bool merge_labeled_break_source_vector_known_lengths(VectorKnownLength& merged,
                                                            bool& has_merged,
                                                            const VectorKnownLengthLookup& lookup,
                                                            const std::vector<MatchArm>& arms,
                                                            const std::string& label) {
    for (const auto& arm : arms) {
        if (!merge_labeled_break_source_vector_known_lengths(
                merged, has_merged, lookup, arm.body, label)) {
            return false;
        }
    }
    return true;
}

static bool merge_labeled_break_source_vector_known_length(VectorKnownLength& merged,
                                                           bool& has_merged,
                                                           const VectorKnownLengthLookup& lookup,
                                                           const Stmt& statement,
                                                           const std::string& label) {
    if (statement.kind == StmtKind::Break && stmt_break_label(statement) == label) {
        return merge_source_vector_known_length(merged, has_merged, stmt_break_value(statement), lookup);
    }

    switch (statement.kind) {
        case StmtKind::Block:
            return merge_labeled_break_source_vector_known_lengths(
                merged, has_merged, lookup, stmt_statements(statement), label);
        case StmtKind::If:
            return merge_labeled_break_source_vector_known_lengths(
                       merged, has_merged, lookup, stmt_then_body(statement), label) &&
                   merge_labeled_break_source_vector_known_lengths(
                       merged, has_merged, lookup, stmt_else_body(statement), label);
        case StmtKind::While:
        case StmtKind::WhileLet:
        case StmtKind::For:
        case StmtKind::InitWhile:
            return merge_labeled_break_source_vector_known_lengths(
                merged, has_merged, lookup, stmt_loop_body(statement), label);
        case StmtKind::Match:
            return merge_labeled_break_source_vector_known_lengths(
                merged, has_merged, lookup, stmt_match_arms(statement), label);
        default:
            return true;
    }
}

static bool merge_labeled_break_source_vector_known_lengths(VectorKnownLength& merged,
                                                            bool& has_merged,
                                                            const VectorKnownLengthLookup& lookup,
                                                            const std::vector<StmtPtr>& statements,
                                                            const std::string& label) {
    for (const auto& statement : statements) {
        if (!merge_labeled_break_source_vector_known_length(
                merged, has_merged, lookup, *statement, label)) {
            return false;
        }
    }
    return true;
}

static bool merge_source_vector_known_length(VectorKnownLength& merged,
                                             bool& has_merged,
                                             const ExprPtr& source,
                                             const VectorKnownLengthLookup& lookup) {
    if (!source) return false;
    VectorKnownLength length = vector_known_length_from_source_tree(*source, lookup);
    if (!length.known) return false;
    if (!has_merged) {
        merged = length;
        has_merged = true;
        return true;
    }
    return merged.length == length.length;
}

VectorKnownLength vector_known_length_from_source_tree(const Expr& source,
                                                       const VectorKnownLengthLookup& lookup) {
    if (source.kind == ExprKind::Vector) {
        return VectorKnownLength{true, static_cast<std::uint64_t>(source.args.size())};
    }
    if (source.kind == ExprKind::Name) {
        return lookup ? lookup(source.name) : VectorKnownLength{};
    }
    if (source.kind == ExprKind::Block && expr_block_value(source)) {
        VectorKnownLength merged;
        bool has_merged = false;
        if (!merge_source_vector_known_length(merged, has_merged, expr_block_value(source), lookup)) return {};
        if (!expr_block_label(source).empty() &&
            !merge_labeled_break_source_vector_known_lengths(
                merged, has_merged, lookup, expr_block_body(source), expr_block_label(source))) {
            return {};
        }
        return has_merged ? merged : VectorKnownLength{};
    }
    if (source.kind == ExprKind::If) {
        VectorKnownLength merged;
        bool has_merged = false;
        if (!merge_source_vector_known_length(merged, has_merged, expr_if_then_value(source), lookup)) return {};
        if (!merge_source_vector_known_length(merged, has_merged, expr_if_else_value(source), lookup)) return {};
        return has_merged ? merged : VectorKnownLength{};
    }
    if (source.kind == ExprKind::Match) {
        VectorKnownLength merged;
        bool has_merged = false;
        for (const auto& arm : expr_match_arms(source)) {
            if (!merge_source_vector_known_length(merged, has_merged, arm.value, lookup)) return {};
        }
        return has_merged ? merged : VectorKnownLength{};
    }
    return {};
}

static void merge_source_vector_storage_capacity(std::uint64_t& capacity,
                                                 const ExprPtr& value,
                                                 const VectorStorageCapacityLookup& lookup);

static void merge_labeled_break_source_vector_storage_capacity(std::uint64_t& capacity,
                                                               const std::vector<StmtPtr>& statements,
                                                               const std::string& label,
                                                               const VectorStorageCapacityLookup& lookup);

static void merge_labeled_break_source_vector_storage_capacity(std::uint64_t& capacity,
                                                               const std::vector<MatchArm>& arms,
                                                               const std::string& label,
                                                               const VectorStorageCapacityLookup& lookup) {
    for (const auto& arm : arms) {
        merge_labeled_break_source_vector_storage_capacity(capacity, arm.body, label, lookup);
    }
}

static void merge_labeled_break_source_vector_storage_capacity(std::uint64_t& capacity,
                                                               const Stmt& statement,
                                                               const std::string& label,
                                                               const VectorStorageCapacityLookup& lookup) {
    if (statement.kind == StmtKind::Break && stmt_break_label(statement) == label) {
        merge_source_vector_storage_capacity(capacity, stmt_break_value(statement), lookup);
        return;
    }

    switch (statement.kind) {
        case StmtKind::Block:
            merge_labeled_break_source_vector_storage_capacity(
                capacity, stmt_statements(statement), label, lookup);
            break;
        case StmtKind::If:
            merge_labeled_break_source_vector_storage_capacity(
                capacity, stmt_then_body(statement), label, lookup);
            merge_labeled_break_source_vector_storage_capacity(
                capacity, stmt_else_body(statement), label, lookup);
            break;
        case StmtKind::While:
        case StmtKind::WhileLet:
        case StmtKind::For:
        case StmtKind::InitWhile:
            merge_labeled_break_source_vector_storage_capacity(
                capacity, stmt_loop_body(statement), label, lookup);
            break;
        case StmtKind::Match:
            merge_labeled_break_source_vector_storage_capacity(
                capacity, stmt_match_arms(statement), label, lookup);
            break;
        default:
            break;
    }
}

static void merge_labeled_break_source_vector_storage_capacity(std::uint64_t& capacity,
                                                               const std::vector<StmtPtr>& statements,
                                                               const std::string& label,
                                                               const VectorStorageCapacityLookup& lookup) {
    for (const auto& statement : statements) {
        merge_labeled_break_source_vector_storage_capacity(capacity, *statement, label, lookup);
    }
}

static void merge_source_vector_storage_capacity(std::uint64_t& capacity,
                                                 const ExprPtr& value,
                                                 const VectorStorageCapacityLookup& lookup) {
    if (value) {
        capacity = std::max(capacity, vector_storage_capacity_from_source_tree(*value, lookup));
    }
}

std::uint64_t vector_storage_capacity_from_source_tree(const Expr& source,
                                                       const VectorStorageCapacityLookup& lookup) {
    if (source.kind == ExprKind::Vector) {
        return static_cast<std::uint64_t>(source.args.size());
    }
    if (source.kind == ExprKind::Name) {
        return lookup ? lookup(source.name) : 0;
    }
    if (source.kind == ExprKind::Block && expr_block_value(source)) {
        std::uint64_t capacity = 0;
        merge_source_vector_storage_capacity(capacity, expr_block_value(source), lookup);
        if (!expr_block_label(source).empty()) {
            merge_labeled_break_source_vector_storage_capacity(
                capacity, expr_block_body(source), expr_block_label(source), lookup);
        }
        return capacity;
    }
    if (source.kind == ExprKind::If) {
        std::uint64_t capacity = 0;
        if (expr_if_then_value(source)) {
            capacity = std::max(
                capacity,
                vector_storage_capacity_from_source_tree(*expr_if_then_value(source), lookup));
        }
        if (expr_if_else_value(source)) {
            capacity = std::max(
                capacity,
                vector_storage_capacity_from_source_tree(*expr_if_else_value(source), lookup));
        }
        return capacity;
    }
    if (source.kind == ExprKind::Match) {
        std::uint64_t capacity = 0;
        for (const auto& arm : expr_match_arms(source)) {
            if (arm.value) {
                capacity = std::max(
                    capacity,
                    vector_storage_capacity_from_source_tree(*arm.value, lookup));
            }
        }
        return capacity;
    }
    return 0;
}

static void merge_labeled_break_vector_storage_capacity(std::uint64_t& capacity,
                                                        const std::vector<IrStmtPtr>& statements,
                                                        const std::string& label);

static void merge_labeled_break_vector_storage_capacity(std::uint64_t& capacity,
                                                        const std::vector<IrMatchArm>& arms,
                                                        const std::string& label) {
    for (const auto& arm : arms) {
        merge_labeled_break_vector_storage_capacity(capacity, arm.body, label);
    }
}

static void merge_labeled_break_vector_storage_capacity(std::uint64_t& capacity,
                                                        const IrStmt& statement,
                                                        const std::string& label) {
    if (statement.kind == IrStmtKind::Break && ir_stmt_break_label(statement) == label) {
        const IrExprPtr& break_value = ir_stmt_break_value(statement);
        if (break_value) {
            capacity = std::max(capacity, vector_storage_capacity_from_expr(*break_value));
        }
        return;
    }

    switch (statement.kind) {
        case IrStmtKind::Block:
            merge_labeled_break_vector_storage_capacity(capacity, ir_stmt_statements(statement), label);
            break;
        case IrStmtKind::If:
            merge_labeled_break_vector_storage_capacity(capacity, ir_stmt_then_body(statement), label);
            merge_labeled_break_vector_storage_capacity(capacity, ir_stmt_else_body(statement), label);
            break;
        case IrStmtKind::While:
        case IrStmtKind::WhileLet:
        case IrStmtKind::ForRange:
        case IrStmtKind::ForVector:
        case IrStmtKind::InitWhile:
            merge_labeled_break_vector_storage_capacity(capacity, ir_stmt_loop_body(statement), label);
            break;
        case IrStmtKind::Match:
            merge_labeled_break_vector_storage_capacity(capacity, ir_stmt_match_arms(statement), label);
            break;
        default:
            break;
    }
}

static void merge_labeled_break_vector_storage_capacity(std::uint64_t& capacity,
                                                        const std::vector<IrStmtPtr>& statements,
                                                        const std::string& label) {
    for (const auto& statement : statements) {
        merge_labeled_break_vector_storage_capacity(capacity, *statement, label);
    }
}

std::uint64_t vector_storage_capacity_from_expr(const IrExpr& expr) {
    std::uint64_t capacity = is_vector_storage_type(expr.type) ? expr.type.array_size : 0;
    auto merge_capacity = [&](const IrExprPtr& value) {
        if (value) capacity = std::max(capacity, vector_storage_capacity_from_expr(*value));
    };
    if (expr.kind == IrExprKind::Vector && is_vector_storage_type(expr.type)) {
        capacity = std::max(capacity, static_cast<std::uint64_t>(expr.args.size()));
    } else if (expr.kind == IrExprKind::Block) {
        merge_capacity(ir_expr_block_value(expr));
        if (!ir_expr_block_label(expr).empty()) {
            merge_labeled_break_vector_storage_capacity(
                capacity, ir_expr_block_body(expr), ir_expr_block_label(expr));
        }
    } else if (expr.kind == IrExprKind::If) {
        merge_capacity(ir_expr_if_then_value(expr));
        merge_capacity(ir_expr_if_else_value(expr));
    } else if (expr.kind == IrExprKind::Match) {
        for (const auto& arm : ir_expr_match_arms(expr)) merge_capacity(arm.value);
    }
    return capacity;
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

void require_local_vec_integer_argument(SourceLocation loc,
                                        LocalVecMethod method,
                                        const char* role,
                                        const IrType& type) {
    if (is_value_integer_type(type)) return;
    std::string display = std::string("Vec.") + local_vec_method_name(method);
    fail(loc, display + " " + role + " must be an integer, got " + type_name(type));
}

void require_local_vec_non_negative_argument(SourceLocation loc,
                                             LocalVecMethod method,
                                             const char* role,
                                             const StaticIntegerValue& value) {
    if (!value.negative) return;
    std::string display = std::string("Vec.") + local_vec_method_name(method);
    fail(loc, display + " " + role + " must be non-negative");
}

void require_local_vec_static_index_in_known_bounds(SourceLocation loc,
                                                    LocalVecMethod method,
                                                    const char* role,
                                                    const StaticIntegerValue& value,
                                                    VectorKnownLength length,
                                                    bool allow_end) {
    require_local_vec_non_negative_argument(loc, method, role, value);
    if (!length.known) return;

    const bool out_of_bounds = allow_end ? value.value > length.length : value.value >= length.length;
    if (!out_of_bounds) return;

    std::string display = std::string("Vec.") + local_vec_method_name(method);
    fail(loc,
         display + " " + role + " " + std::to_string(value.value) +
             " is out of range for " + std::to_string(length.length) + " elements");
}

void require_vector_index_in_known_bounds(SourceLocation loc,
                                          const StaticIntegerValue& value,
                                          VectorKnownLength length) {
    if (value.negative) fail(loc, "vector index must be non-negative");
    if (!length.known || value.value < length.length) return;
    fail(loc,
         "vector index " + std::to_string(value.value) +
             " is out of range for " + std::to_string(length.length) + " elements");
}

void require_vector_index_known_non_empty(SourceLocation loc, VectorKnownLength length) {
    if (!length.known || length.length != 0) return;
    fail(loc, "vector index requires a non-empty Vec");
}

void require_local_vec_known_non_empty(SourceLocation loc,
                                       LocalVecMethod method,
                                       VectorKnownLength length) {
    if (!length.known || length.length != 0) return;
    std::string display = std::string("Vec.") + local_vec_method_name(method);
    fail(loc, display + " requires a non-empty Vec");
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

VectorKnownLength vector_known_length_after_append(VectorKnownLength current) {
    if (!current.known) return {};
    return VectorKnownLength{true, current.length + 1};
}

VectorKnownLength vector_known_length_after_remove(VectorKnownLength current) {
    if (!current.known || current.length == 0) return {};
    return VectorKnownLength{true, current.length - 1};
}

VectorKnownLength vector_known_length_after_clear() {
    return VectorKnownLength{true, 0};
}

VectorKnownLength vector_known_length_after_truncate(VectorKnownLength current,
                                                     const StaticIntegerValue* requested_length) {
    std::uint64_t updated_length = 0;
    if (!current.known ||
        !requested_length ||
        !vector_known_length_after_truncate(current.length, *requested_length, updated_length)) {
        return {};
    }
    return VectorKnownLength{true, updated_length};
}

VectorKnownLength vector_known_length_after_checked_truncate(SourceLocation loc,
                                                             VectorKnownLength current,
                                                             const StaticIntegerValue* requested_length) {
    if (requested_length) {
        require_local_vec_non_negative_argument(loc, LocalVecMethod::Truncate, "length", *requested_length);
    }
    return vector_known_length_after_truncate(current, requested_length);
}

std::uint64_t vector_required_capacity_for_append(const IrType& storage_type,
                                                  VectorKnownLength current) {
    if (current.known) return current.length + 1;
    return storage_type.array_size + 1;
}

IrExprPtr make_void_noop_expr(SourceLocation loc) {
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::Noop;
    lowered->loc = loc;
    lowered->type = void_type(loc);
    return lowered;
}

IrExprPtr make_empty_vector_literal_expr(SourceLocation loc, const IrType& element) {
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::Vector;
    lowered->loc = loc;
    lowered->type = make_vector_storage_type(loc, element, 0);
    return lowered;
}

IrExprPtr make_vec_local_lvalue(SourceLocation loc, std::string name, IrType type) {
    auto local = std::make_unique<IrExpr>();
    local->kind = IrExprKind::Local;
    local->loc = loc;
    set_ir_expr_name(*local, std::move(name));
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
    set_ir_expr_operand(*out, std::move(vector));
    set_ir_expr_right(*out, std::move(index));
    return out;
}

IrExprPtr make_vec_first_expr(SourceLocation loc,
                              SourceLocation receiver_loc,
                              const std::string& name,
                              const IrType& type) {
    return make_vec_index_expr(
        loc,
        make_vec_local_lvalue(receiver_loc, name, type),
        make_i64_literal(loc, 0)
    );
}

IrExprPtr make_vec_last_expr(SourceLocation loc,
                             SourceLocation receiver_loc,
                             const std::string& name,
                             const IrType& type) {
    auto index = std::make_unique<IrExpr>();
    index->kind = IrExprKind::Binary;
    index->loc = loc;
    index->op = IrBinaryOp::Sub;
    index->type = i64_type(loc);
    set_ir_expr_left(
        *index,
        make_collection_len_expr(loc, make_vec_local_lvalue(receiver_loc, name, type))
    );
    set_ir_expr_right(*index, make_i64_literal(loc, 1));
    return make_vec_index_expr(
        loc,
        make_vec_local_lvalue(receiver_loc, name, type),
        std::move(index)
    );
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
    set_ir_expr_operand(*lowered, std::move(vector));
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
    set_ir_expr_operand(*lowered, std::move(vector));
    set_ir_expr_right(*lowered, std::move(requested_capacity));
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
    set_ir_expr_operand(*lowered, std::move(vector));
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
    set_ir_expr_operand(*lowered, std::move(vector));
    set_ir_expr_right(*lowered, std::move(new_length));
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
    set_ir_expr_operand(*lowered, std::move(vector));
    set_ir_expr_right(*lowered, std::move(index));
    set_ir_expr_payload(*lowered, std::move(value));
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
    set_ir_expr_operand(*lowered, std::move(vector));
    set_ir_expr_right(*lowered, std::move(first_index));
    set_ir_expr_payload(*lowered, std::move(second_index));
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
    set_ir_expr_operand(*lowered, std::move(vector));
    set_ir_expr_right(*lowered, std::move(index));
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
    set_ir_expr_operand(*lowered, std::move(vector));
    set_ir_expr_right(*lowered, std::move(index));
    set_ir_expr_payload(*lowered, std::move(value));
    return lowered;
}

IrExprPtr make_vec_push_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr value) {
    if (!vector || !is_vector_storage_type(vector->type)) {
        fail(loc, "Vec.push requires local Vec storage");
    }
    if (!value) {
        fail(loc, "Vec.push expects a value");
    }
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::VectorPush;
    lowered->loc = loc;
    lowered->type = void_type(loc);
    set_ir_expr_operand(*lowered, std::move(vector));
    set_ir_expr_right(*lowered, std::move(value));
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
    set_ir_expr_operand(*lowered, std::move(vector));
    set_ir_expr_payload(*lowered, std::move(value));
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
    set_ir_expr_operand(*lowered, std::move(vector));
    set_ir_expr_payload(*lowered, std::move(value));
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
    set_ir_expr_operand(*lowered, std::move(vector));
    set_ir_expr_payload(*lowered, std::move(value));
    return lowered;
}

IrExprPtr make_local_vec_len_expr(SourceLocation loc, IrExprPtr value, VectorKnownLength length) {
    if (length.known && value && is_vector_storage_type(value->type)) {
        return make_i64_literal(loc, length.length);
    }
    return make_collection_len_expr(loc, std::move(value));
}

IrExprPtr make_local_vec_is_empty_expr(SourceLocation loc, IrExprPtr value, VectorKnownLength length) {
    if (length.known && value && is_vector_storage_type(value->type)) {
        return make_bool_literal(loc, length.length == 0);
    }
    IrExprPtr lowered_length = make_collection_len_expr(loc, std::move(value));
    return make_collection_is_empty_expr(loc, std::move(lowered_length));
}

IrExprPtr make_collection_len_expr(SourceLocation loc, IrExprPtr value) {
    if (!value ||
        (value->type.primitive != IrPrimitiveKind::Vector &&
         value->type.primitive != IrPrimitiveKind::Array &&
         !is_prelude_slice_type(value->type)) ||
        value->type.args.size() != 1) {
        const std::string actual = value ? type_name(value->type) : "<missing>";
        fail(loc, "len expects an array, Vec, or Slice value, got " + actual);
    }
    if (value->type.primitive == IrPrimitiveKind::Array) {
        if (value->kind == IrExprKind::Vector && is_owner_type(value->type)) {
            fail(loc, "len of owning array literals would discard owning values; bind the array first");
        }
        return make_i64_literal(loc, value->type.array_size);
    }
    if (value->kind == IrExprKind::Vector) {
        if (is_owner_type(value->type)) {
            fail(loc, "len of owning vector literals would discard owning values; bind the vector first");
        }
        return make_i64_literal(loc, static_cast<std::uint64_t>(value->args.size()));
    }

    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::TupleIndex;
    lowered->loc = loc;
    lowered->tuple_index = is_prelude_slice_type(value->type) ? 1 : 0;
    lowered->type = i64_type(loc);
    set_ir_expr_operand(*lowered, std::move(value));
    return lowered;
}

IrExprPtr make_collection_is_empty_expr(SourceLocation loc, IrExprPtr length) {
    auto empty = std::make_unique<IrExpr>();
    empty->kind = IrExprKind::Binary;
    empty->loc = loc;
    empty->op = IrBinaryOp::Eq;
    empty->type = bool_type(loc);
    set_ir_expr_left(*empty, std::move(length));
    set_ir_expr_right(*empty, make_i64_literal(loc, 0));
    return empty;
}

IrExprPtr make_slice_data_pointer_expr(SourceLocation loc, IrExprPtr lvalue, const IrType& element) {
    IrType pointer = element;
    pointer.qualifier = TypeQualifier::Ptr;

    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::Borrow;
    lowered->loc = loc;
    lowered->type = std::move(pointer);
    set_ir_expr_operand(*lowered, std::move(lvalue));
    return lowered;
}

IrExprPtr make_slice_view_expr(SourceLocation loc, IrExprPtr data, IrExprPtr length, IrType slice_type) {
    auto lowered = std::make_unique<IrExpr>();
    lowered->kind = IrExprKind::Tuple;
    lowered->loc = loc;
    lowered->type = std::move(slice_type);
    lowered->args.reserve(2);
    lowered->args.push_back(std::move(data));
    lowered->args.push_back(std::move(length));
    return lowered;
}

IrExprPtr make_vec_storage_lvalue_expr(SourceLocation loc, std::string name, const IrType& type) {
    if (!is_vector_storage_type(type)) {
        fail(loc, "Vec storage view requires local Vec storage");
    }
    if (type.args.empty()) {
        fail(loc, "Vec storage view requires an element type");
    }

    auto storage = std::make_unique<IrExpr>();
    storage->kind = IrExprKind::TupleIndex;
    storage->loc = loc;
    storage->tuple_index = 1;
    storage->type = array_storage_type(loc, type.args[0], type.array_size);
    set_ir_expr_operand(
        *storage,
        make_vec_local_lvalue(loc, std::move(name), type)
    );
    return storage;
}

} // namespace ari
