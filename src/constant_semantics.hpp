#pragma once

#include "ir.hpp"
#include "token.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ari {

enum class ConstantValueKind {
    Integer,
    Bool,
    Tuple,
    Struct,
    Array,
    Enum
};

struct ConstantValue {
    ConstantValueKind kind = ConstantValueKind::Integer;
    IrType type;
    bool is_bool = false;
    std::uint64_t int_value = 0;
    bool int_negative = false;
    bool bool_value = false;
    std::vector<ConstantValue> elements;
    std::string enum_name;
    std::string case_name;
    std::uint32_t enum_tag = 0;
};

struct StaticIntegerValue {
    std::uint64_t value = 0;
    bool negative = false;
};

class ConstantEvaluationStackGuard {
public:
    ConstantEvaluationStackGuard(std::vector<std::string>& stack, std::string name);
    ConstantEvaluationStackGuard(const ConstantEvaluationStackGuard&) = delete;
    ConstantEvaluationStackGuard& operator=(const ConstantEvaluationStackGuard&) = delete;
    ~ConstantEvaluationStackGuard();

private:
    std::vector<std::string>& stack_;
};

std::string format_constant_cycle_path(const std::vector<std::string>& stack,
                                       const std::string& repeated_name);
bool static_integer_value_to_i64(const StaticIntegerValue& value, std::int64_t& out);
StaticIntegerValue static_integer_value_from_i64(std::int64_t value);
std::int64_t constant_integer_to_i64(SourceLocation loc, const ConstantValue& value);
std::uint64_t constant_integer_to_u64(SourceLocation loc, const ConstantValue& value);
std::uint64_t integer_bit_mask(unsigned width);
std::uint64_t constant_integer_raw_bits(const ConstantValue& value, unsigned width);
std::int64_t sign_extend_integer_bits(std::uint64_t raw, unsigned width);
unsigned constant_shift_amount(SourceLocation loc, const ConstantValue& value, unsigned width);
ConstantValue make_signed_integer_constant(SourceLocation loc, const IrType& type, std::int64_t result);
ConstantValue make_unsigned_integer_constant(SourceLocation loc, const IrType& type, std::uint64_t result);
ConstantValue make_integer_literal_constant(SourceLocation loc,
                                            const IrType& expected,
                                            const std::string& literal_suffix,
                                            std::uint64_t int_value,
                                            bool int_negative);
ConstantValue make_bool_constant(SourceLocation loc, const IrType& expected, bool result);
ConstantValue make_bool_literal_constant(SourceLocation loc, const IrType& expected, bool result);
ConstantValue evaluate_constant_bool_binary(SourceLocation loc,
                                            TokenKind op,
                                            const IrType& expected,
                                            const ConstantValue& left,
                                            const ConstantValue& right);
ConstantValue evaluate_constant_bool_comparison(SourceLocation loc,
                                                TokenKind op,
                                                const IrType& expected,
                                                const ConstantValue& left,
                                                const ConstantValue& right);
ConstantValue evaluate_constant_integer_comparison(SourceLocation loc,
                                                   TokenKind op,
                                                   const IrType& expected,
                                                   const IrType& operand_type,
                                                   const ConstantValue& left,
                                                   const ConstantValue& right);
ConstantValue evaluate_constant_integer_binary(SourceLocation loc,
                                               TokenKind op,
                                               const IrType& expected,
                                               const ConstantValue& left,
                                               const ConstantValue& right);
IrExprPtr make_constant_expr(SourceLocation loc, const ConstantValue& value);
IrMatchArm make_scalar_constant_match_arm(SourceLocation loc,
                                          const ConstantValue& value,
                                          const IrType& match_type);
void set_nested_enum_payload_constant_literal(SourceLocation loc,
                                              const ConstantValue& value,
                                              const IrType& nested_payload_type,
                                              IrPayloadEnumCondition& condition);
void lower_aggregate_enum_payload_constant_pattern(SourceLocation loc,
                                                   const ConstantValue& value,
                                                   const IrType& payload_type,
                                                   IrMatchArm& lowered_arm,
                                                   std::uint32_t payload_index);
void lower_compact_enum_payload_constant_pattern(SourceLocation loc,
                                                 const ConstantValue& value,
                                                 const IrType& payload_type,
                                                 std::uint32_t enum_tag,
                                                 IrMatchArm& lowered_arm);
bool fold_static_integer_unary(TokenKind op,
                               const StaticIntegerValue& operand,
                               StaticIntegerValue& out);
bool fold_static_integer_binary(TokenKind op,
                                const StaticIntegerValue& left,
                                const StaticIntegerValue& right,
                                StaticIntegerValue& out);
bool try_fold_static_integer_value(const IrExpr& expr, StaticIntegerValue& out);

} // namespace ari
