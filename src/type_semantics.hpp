#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <cstdint>
#include <string>

namespace ari {

bool same_type(const IrType& left, const IrType& right);
bool is_copy_type(const IrType& type);
bool is_owner_type(const IrType& type);
bool is_borrow_type(const IrType& type);
bool contains_borrow_type(const IrType& type);
bool is_aggregate_type(const IrType& type);

bool is_value_integer_type(const IrType& type);
bool is_value_enum_type(const IrType& type);
bool is_value_float_type(const IrType& type);
bool is_value_trait_object_type(const IrType& type);
bool is_void_value_type(const IrType& type);

bool is_raw_pointer_type(const IrType& type);
bool is_raw_pointer_cast(const IrType& from, const IrType& to);
IrType raw_pointer_pointee_type(IrType type);
bool is_raw_memory_value_type(const IrType& type);
bool is_raw_pointer_deref_value_type(const IrType& type);

bool is_integer_literal(const IrExpr& expr);
bool is_float_literal(const IrExpr& expr);
bool is_null_literal(const IrExpr& expr);

bool is_integer_primitive(IrPrimitiveKind primitive);
bool is_owned_executable_primitive(IrPrimitiveKind primitive);
bool is_borrowable_executable_primitive(IrPrimitiveKind primitive);
bool is_legacy_enum_payload_type(const IrType& type);
bool is_aggregate_enum_payload_type(const IrType& type);
bool has_aggregate_enum_layout(const IrType& type);

bool is_float_primitive(IrPrimitiveKind primitive);
const char* primitive_name(IrPrimitiveKind primitive);
IrType integer_literal_suffix_type(const std::string& suffix, SourceLocation loc);
IrType float_literal_suffix_type(const std::string& suffix, SourceLocation loc);

bool is_signed_integer_primitive(IrPrimitiveKind primitive);
bool is_unsigned_integer_primitive(IrPrimitiveKind primitive);
unsigned integer_primitive_bit_width(IrPrimitiveKind primitive);
std::uint64_t signed_positive_max(IrPrimitiveKind primitive);
std::uint64_t signed_negative_limit(IrPrimitiveKind primitive);
std::uint64_t unsigned_max(IrPrimitiveKind primitive);

std::string integer_literal_name(const IrExpr& expr);
IrType literal_value_type_for_expected(const IrType& expected);
bool integer_literal_fits(const IrExpr& expr, const IrType& expected);
bool range_start_le_end(const Pattern& pattern, const IrType& match_type);

IrType require_raw_pointer_memory_type(SourceLocation loc, const IrType& pointer_type, const std::string& operation);
IrType require_raw_pointer_deref_type(SourceLocation loc, const IrType& pointer_type, const std::string& operation);
IrType require_raw_pointer_add_type(SourceLocation loc, const IrType& pointer_type, const std::string& operation);
IrType require_raw_pointer_materializable_type(SourceLocation loc, const IrType& pointer_type, const std::string& operation);

void require_numeric_operands(SourceLocation loc, const IrType& left, const IrType& right);
void require_integer_operands(SourceLocation loc, const IrType& left, const IrType& right);
void require_integer_shift_operands(SourceLocation loc, const IrType& left, const IrType& right);
void require_comparable_operands(SourceLocation loc, const IrType& left, const IrType& right);
void require_logical_operand(SourceLocation loc, const IrType& type);
void require_bitwise_not_operand(SourceLocation loc, const IrType& type);
void require_boolish(SourceLocation loc, const IrType& type);
void require_assignable(SourceLocation loc, const IrType& expected, const IrType& actual);

} // namespace ari
