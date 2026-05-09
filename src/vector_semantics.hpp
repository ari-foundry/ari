#pragma once

#include "ir.hpp"

#include <cstdint>
#include <string>

namespace ari {

bool is_vector_storage_type(const IrType& type);
void specialize_vector_storage_from_init(IrType& declared, const IrExpr& init);
void widen_vector_storage_type(IrType& type, std::uint64_t capacity);
void widen_vector_storage_literal(IrExpr& expr, std::uint64_t capacity);

IrExprPtr make_void_noop_expr(SourceLocation loc);
IrExprPtr make_vec_local_lvalue(SourceLocation loc, std::string name, IrType type);
IrExprPtr make_vec_capacity_expr(SourceLocation loc, const IrType& type);
IrExprPtr make_vec_index_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index);
IrExprPtr make_vec_pop_expr(SourceLocation loc, IrExprPtr vector);
IrExprPtr make_vec_clear_expr(SourceLocation loc, IrExprPtr vector);
IrExprPtr make_vec_truncate_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr new_length);
IrExprPtr make_vec_set_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index, IrExprPtr value);
IrExprPtr make_vec_swap_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr first_index, IrExprPtr second_index);
IrExprPtr make_vec_remove_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index);
IrExprPtr make_vec_insert_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr index, IrExprPtr value);
IrExprPtr make_vec_contains_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr value);
IrExprPtr make_vec_index_of_expr(SourceLocation loc, IrExprPtr vector, IrExprPtr value);
IrExprPtr make_collection_is_empty_expr(SourceLocation loc, IrExprPtr length);

} // namespace ari
