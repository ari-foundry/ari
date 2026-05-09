#pragma once

#include "ir.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ari {

IrStmtPtr make_ir_var_decl(SourceLocation loc,
                           std::string name,
                           IrType type,
                           IrExprPtr init,
                           bool mutable_binding);

IrExprPtr make_local_lvalue_expr(SourceLocation loc, const std::string& name, const IrType& type);
IrExprPtr make_tuple_index_expr(SourceLocation loc,
                                const std::string& source_name,
                                const IrType& source_type,
                                std::size_t index);
IrExprPtr make_vector_index_expr(SourceLocation loc,
                                 const std::string& source_name,
                                 const IrType& source_type,
                                 const std::string& index_name,
                                 const IrType& index_type);

IrExprPtr make_integer_literal(SourceLocation loc, const IrType& type, std::uint64_t value);
IrExprPtr make_integer_zero(SourceLocation loc, const IrType& type);
IrExprPtr make_bool_literal_expr(SourceLocation loc, bool value);
IrExprPtr make_bool_binary_expr(SourceLocation loc, IrBinaryOp op, IrExprPtr left, IrExprPtr right);
IrExprPtr make_cast_expr(SourceLocation loc, IrExprPtr value, const IrType& target);
IrExprPtr make_builtin_call(SourceLocation loc,
                            const std::string& name,
                            std::vector<IrExprPtr> args,
                            const IrType& result);

} // namespace ari
