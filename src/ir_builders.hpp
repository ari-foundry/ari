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

IrExprPtr make_integer_literal(SourceLocation loc, const IrType& type, std::uint64_t value, bool negative = false);
IrExprPtr make_integer_zero(SourceLocation loc, const IrType& type);
IrExprPtr make_bool_literal_expr(SourceLocation loc, bool value);
IrExprPtr make_float_literal_expr(SourceLocation loc, const IrType& type, double value);
IrExprPtr make_string_literal_expr(SourceLocation loc, const IrType& type, std::string value = {});
IrExprPtr make_null_literal_expr(SourceLocation loc, const IrType& type);
IrExprPtr make_ir_tuple_expr(SourceLocation loc, IrType type, std::vector<IrExprPtr> elements = {});
IrExprPtr make_bool_binary_expr(SourceLocation loc, IrBinaryOp op, IrExprPtr left, IrExprPtr right);
IrExprPtr make_cast_expr(SourceLocation loc, IrExprPtr value, const IrType& target);
IrExprPtr make_pointer_offset_expr(SourceLocation loc, IrExprPtr pointer, IrExprPtr offset);
IrExprPtr make_pointer_add_expr(SourceLocation loc, IrExprPtr pointer, IrExprPtr offset);
IrExprPtr make_pointer_load_expr(SourceLocation loc, IrExprPtr pointer, const IrType& result);
IrExprPtr make_pointer_store_expr(SourceLocation loc, IrExprPtr pointer, IrExprPtr value);
IrExprPtr make_ir_call_expr(SourceLocation loc,
                            std::string name,
                            IrType result,
                            std::vector<IrExprPtr> args = {});
IrExprPtr make_builtin_call(SourceLocation loc,
                            const std::string& name,
                            std::vector<IrExprPtr> args,
                            const IrType& result);
IrMatchExprArm make_match_expr_arm(IrMatchArm arm);
IrExprPtr make_ir_match_expr(SourceLocation loc, IrExprPtr value);
IrExprPtr make_ir_block_expr(SourceLocation loc, std::string label = {});
IrExprPtr make_ir_block_expr(SourceLocation loc,
                             std::string label,
                             IrType type,
                             std::vector<IrStmtPtr> body,
                             IrExprPtr value);
IrExprPtr make_ir_if_expr(SourceLocation loc,
                          IrType type,
                          IrExprPtr condition,
                          std::vector<IrStmtPtr> then_body,
                          IrExprPtr then_value,
                          std::vector<IrStmtPtr> else_body,
                          IrExprPtr else_value);

} // namespace ari
