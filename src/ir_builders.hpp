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
IrExprPtr make_tuple_index_expr(SourceLocation loc, IrExprPtr source, std::size_t index);
IrExprPtr make_vector_index_expr(SourceLocation loc,
                                 const std::string& source_name,
                                 const IrType& source_type,
                                 const std::string& index_name,
                                 const IrType& index_type);
IrExprPtr make_ir_index_expr(SourceLocation loc, IrExprPtr source, IrExprPtr index);

IrExprPtr make_integer_literal(SourceLocation loc, const IrType& type, std::uint64_t value, bool negative = false);
IrExprPtr make_integer_zero(SourceLocation loc, const IrType& type);
IrExprPtr make_bool_literal_expr(SourceLocation loc, bool value);
IrExprPtr make_float_literal_expr(SourceLocation loc, const IrType& type, double value);
IrExprPtr make_string_literal_expr(SourceLocation loc, const IrType& type, std::string value = {});
IrExprPtr make_null_literal_expr(SourceLocation loc, const IrType& type);
IrExprPtr make_function_ref_expr(SourceLocation loc, std::string name, IrType type);
IrExprPtr make_borrow_expr(SourceLocation loc,
                           std::string source_name,
                           std::string path,
                           IrExprPtr source,
                           bool mutable_borrow,
                           IrType borrowed_type);
IrExprPtr make_ir_tuple_expr(SourceLocation loc, IrType type, std::vector<IrExprPtr> elements = {});
IrExprPtr make_bool_binary_expr(SourceLocation loc, IrBinaryOp op, IrExprPtr left, IrExprPtr right);
IrExprPtr make_cast_expr(SourceLocation loc, IrExprPtr value, const IrType& target);
IrExprPtr make_trait_object_cast_expr(SourceLocation loc,
                                      IrExprPtr value,
                                      IrType target,
                                      std::string vtable_name,
                                      std::uint64_t vtable_offset,
                                      std::string drop_thunk_name = {});
IrExprPtr make_pointer_offset_expr(SourceLocation loc, IrExprPtr pointer, IrExprPtr offset);
IrExprPtr make_pointer_add_expr(SourceLocation loc, IrExprPtr pointer, IrExprPtr offset);
IrExprPtr make_pointer_load_expr(SourceLocation loc, IrExprPtr pointer, const IrType& result);
IrExprPtr make_pointer_store_expr(SourceLocation loc, IrExprPtr pointer, IrExprPtr value);
IrExprPtr make_enum_tag_expr(SourceLocation loc, IrExprPtr value);
IrExprPtr make_ir_try_expr(SourceLocation loc,
                           IrExprPtr operand,
                           IrType success_payload_type,
                           std::uint32_t success_tag,
                           bool converts_residual,
                           std::uint32_t return_residual_tag,
                           bool residual_has_payload,
                           IrType return_residual_payload_type,
                           std::vector<IrStmtPtr> residual_cleanup);
IrExprPtr make_ir_null_coalesce_expr(SourceLocation loc,
                                     IrExprPtr value,
                                     IrExprPtr fallback,
                                     IrType success_payload_type,
                                     std::uint32_t success_tag);
IrExprPtr make_ir_call_expr(SourceLocation loc,
                            std::string name,
                            IrType result,
                            std::vector<IrExprPtr> args = {});
IrExprPtr make_builtin_call(SourceLocation loc,
                            const std::string& name,
                            std::vector<IrExprPtr> args,
                            const IrType& result);
IrExprPtr make_format_print_expr(SourceLocation loc,
                                 IrType result,
                                 std::vector<std::string> format_parts,
                                 std::vector<IrFormatSpec> format_specs,
                                 std::vector<IrExprPtr> args,
                                 bool print_newline);
IrExprPtr make_trait_object_call_expr(SourceLocation loc,
                                      std::string method_name,
                                      IrExprPtr receiver,
                                      std::uint64_t slot,
                                      IrType result,
                                      std::vector<IrType> erased_params,
                                      std::vector<IrExprPtr> args);
IrExprPtr make_trait_object_drop_expr(SourceLocation loc, IrExprPtr receiver, IrType result);
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
