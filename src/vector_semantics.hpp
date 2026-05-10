#pragma once

#include "constant_semantics.hpp"
#include "ir.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace ari {

enum class LocalVecMethod {
    Unknown,
    AsSlice,
    Capacity,
    Clear,
    Contains,
    Count,
    First,
    Get,
    IndexOf,
    Insert,
    IsEmpty,
    Last,
    Len,
    Pop,
    Push,
    Remove,
    Reserve,
    Set,
    Swap,
    Truncate,
};

struct VectorKnownLength {
    bool known = false;
    std::uint64_t length = 0;
};

bool is_vector_storage_type(const IrType& type);
void specialize_vector_storage_from_init(IrType& declared, const IrExpr& init);
void widen_vector_storage_type(IrType& type, std::uint64_t capacity);
void widen_vector_storage_literal(IrExpr& expr, std::uint64_t capacity);
bool vector_literal_length(const IrExpr& expr, std::uint64_t& out);
VectorKnownLength vector_known_length_from_expr(const IrType& storage_type, const IrExpr& expr);
LocalVecMethod classify_local_vec_method(const std::string& method_name);
void require_collection_len_function_shape(SourceLocation loc,
                                           std::size_t type_arg_count,
                                           std::size_t arg_count);
void require_collection_len_method_shape(SourceLocation loc,
                                         std::size_t type_arg_count,
                                         std::size_t arg_count);
void require_collection_is_empty_method_shape(SourceLocation loc,
                                              std::size_t type_arg_count,
                                              std::size_t arg_count);
void require_slice_view_method_shape(SourceLocation loc,
                                     std::size_t type_arg_count,
                                     std::size_t arg_count);
void require_local_vec_method_shape(SourceLocation loc,
                                    LocalVecMethod method,
                                    std::size_t type_arg_count,
                                    std::size_t arg_count);
void require_local_vec_integer_argument(SourceLocation loc,
                                        LocalVecMethod method,
                                        const char* role,
                                        const IrType& type);
void require_local_vec_non_negative_argument(SourceLocation loc,
                                             LocalVecMethod method,
                                             const char* role,
                                             const StaticIntegerValue& value);
std::string local_vec_api_freeze_message(const std::string& method_name);

bool vector_known_length_after_truncate(std::uint64_t current_length,
                                        const StaticIntegerValue& requested_length,
                                        std::uint64_t& out);
VectorKnownLength vector_known_length_after_append(VectorKnownLength current);
VectorKnownLength vector_known_length_after_remove(VectorKnownLength current);
VectorKnownLength vector_known_length_after_clear();
VectorKnownLength vector_known_length_after_truncate(VectorKnownLength current,
                                                     const StaticIntegerValue* requested_length);
std::uint64_t vector_required_capacity_for_append(const IrType& storage_type,
                                                  VectorKnownLength current);

IrExprPtr make_void_noop_expr(SourceLocation loc);
IrExprPtr make_vec_local_lvalue(SourceLocation loc, std::string name, IrType type);
IrExprPtr make_vec_capacity_expr(SourceLocation loc, const IrType& type);
IrExprPtr make_vec_index_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index);
IrExprPtr make_vec_pop_expr(SourceLocation loc, IrExprPtr vector);
IrExprPtr make_vec_reserve_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr requested_capacity);
IrExprPtr make_vec_clear_expr(SourceLocation loc, IrExprPtr vector);
IrExprPtr make_vec_truncate_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr new_length);
IrExprPtr make_vec_set_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index, IrExprPtr value);
IrExprPtr make_vec_swap_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr first_index, IrExprPtr second_index);
IrExprPtr make_vec_remove_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index);
IrExprPtr make_vec_insert_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index, IrExprPtr value);
IrExprPtr make_vec_push_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr value);
IrExprPtr make_vec_contains_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr value);
IrExprPtr make_vec_index_of_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr value);
IrExprPtr make_vec_count_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr value);
IrExprPtr make_collection_len_expr(SourceLocation loc, IrExprPtr value);
IrExprPtr make_collection_is_empty_expr(SourceLocation loc, IrExprPtr length);

} // namespace ari
