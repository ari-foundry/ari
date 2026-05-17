#pragma once

#include "ir.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ari {

struct ModuleCacheIrExprSummary;
struct ModuleCacheIrStmtSummary;

using ModuleCacheIrExprSummaryPtr = std::unique_ptr<ModuleCacheIrExprSummary>;
using ModuleCacheIrStmtSummaryPtr = std::unique_ptr<ModuleCacheIrStmtSummary>;

struct ModuleCacheIrBindingSummary {
    std::string name;
    std::string type;
    bool mutable_binding = false;
    ModuleCacheIrExprSummaryPtr init;
};

struct ModuleCacheIrPayloadBindingSummary {
    std::uint64_t index = 0;
    std::string name;
    std::string type;
    std::vector<std::uint64_t> field_path;
    bool compact_enum_payload = false;
    std::string compact_enum_type;
    std::uint64_t compact_enum_payload_index = 0;
};

struct ModuleCacheIrPayloadLiteralConditionSummary {
    std::uint64_t index = 0;
    bool is_bool = false;
    std::string literal;
};

struct ModuleCacheIrPayloadRangeConditionSummary {
    std::uint64_t index = 0;
    std::string start;
    std::string end;
    bool inclusive = false;
    bool is_unsigned = false;
    std::string type;
    bool compact_enum_payload = false;
};

struct ModuleCacheIrPayloadVectorLengthConditionSummary {
    std::uint64_t index = 0;
    std::vector<std::uint64_t> field_path;
    std::uint64_t length = 0;
    bool at_least = false;
};

struct ModuleCacheIrPayloadEnumConditionSummary {
    std::uint64_t index = 0;
    std::string enum_type;
    std::uint64_t tag = 0;
    std::uint64_t nested_payload_index = 0;
    bool has_payload_literal = false;
    bool payload_literal_is_bool = false;
    std::string payload_literal;
    bool payload_literal_negative = false;
    bool has_payload_range = false;
    std::string range_start;
    std::string range_end;
    bool range_inclusive = false;
    bool range_is_unsigned = false;
    std::string payload_type;
};

struct ModuleCacheIrMatchArmPatternSummary {
    bool wildcard = false;
    bool has_literal = false;
    bool literal_is_bool = false;
    std::string literal;
    bool literal_negative = false;
    bool has_range = false;
    std::string range_start;
    std::string range_end;
    bool range_inclusive = false;
    bool range_is_unsigned = false;
    std::string case_name;
    std::uint64_t enum_tag = 0;
    bool has_value_binding = false;
    std::string value_name;
    std::string value_type;
    bool has_payload_binding = false;
    std::string payload_name;
    std::string payload_type;
    std::uint64_t payload_index = 0;
    std::vector<ModuleCacheIrPayloadBindingSummary> payload_bindings;
    std::vector<ModuleCacheIrPayloadLiteralConditionSummary> payload_literal_conditions;
    std::vector<ModuleCacheIrPayloadRangeConditionSummary> payload_range_conditions;
    std::vector<ModuleCacheIrPayloadVectorLengthConditionSummary> payload_vector_length_conditions;
    std::vector<ModuleCacheIrPayloadEnumConditionSummary> payload_enum_conditions;
};

struct ModuleCacheIrStmtMatchArmSummary {
    ModuleCacheIrMatchArmPatternSummary pattern;
    std::vector<ModuleCacheIrStmtSummaryPtr> body;
};

struct ModuleCacheIrExprMatchArmSummary {
    ModuleCacheIrMatchArmPatternSummary pattern;
    std::vector<ModuleCacheIrStmtSummaryPtr> body;
    ModuleCacheIrExprSummaryPtr value;
};

struct ModuleCacheIrExprSummary {
    std::string kind;
    std::string type;
    std::string name;
    std::string label;
    std::string string_value;
    std::string borrow_source_name;
    std::string borrow_source_path;
    std::string enum_name;
    std::string case_name;
    std::string scalar_payload;
    std::string unary_op;
    std::string binary_op;
    bool mutable_borrow = false;
    std::uint64_t enum_tag = 0;
    bool has_enum_payload = false;
    std::string enum_payload_type;
    ModuleCacheIrExprSummaryPtr operand;
    ModuleCacheIrExprSummaryPtr left;
    ModuleCacheIrExprSummaryPtr right;
    ModuleCacheIrExprSummaryPtr payload;
    std::vector<ModuleCacheIrExprSummaryPtr> args;
    std::vector<std::string> call_param_types;
    ModuleCacheIrExprSummaryPtr if_condition;
    std::vector<ModuleCacheIrStmtSummaryPtr> if_then_body;
    ModuleCacheIrExprSummaryPtr if_then_value;
    std::vector<ModuleCacheIrStmtSummaryPtr> if_else_body;
    ModuleCacheIrExprSummaryPtr if_else_value;
    std::string block_label;
    std::vector<ModuleCacheIrStmtSummaryPtr> block_body;
    ModuleCacheIrExprSummaryPtr block_value;
    ModuleCacheIrExprSummaryPtr match_value;
    std::vector<ModuleCacheIrExprMatchArmSummary> match_arms;
    std::vector<std::string> format_parts;
    std::vector<std::string> format_specs;
    bool format_print_newline = false;
    bool try_converts_residual = false;
    bool try_residual_has_payload = false;
    std::string try_return_residual_payload_type;
    std::uint64_t try_return_residual_tag = 0;
    std::vector<ModuleCacheIrStmtSummaryPtr> try_residual_cleanup;
};

struct ModuleCacheIrStmtSummary {
    std::string kind;
    std::string label;
    std::string drop_name;
    std::string assign_name;
    ModuleCacheIrBindingSummary binding;
    ModuleCacheIrExprSummaryPtr assign_target;
    ModuleCacheIrExprSummaryPtr assign_rhs;
    ModuleCacheIrExprSummaryPtr expr;
    ModuleCacheIrExprSummaryPtr condition;
    std::vector<ModuleCacheIrStmtSummaryPtr> statements;
    std::vector<ModuleCacheIrStmtSummaryPtr> then_body;
    std::vector<ModuleCacheIrStmtSummaryPtr> else_body;
    std::vector<ModuleCacheIrStmtSummaryPtr> loop_body;
    std::string for_binding_name;
    std::string for_index_name;
    std::string for_end_name;
    std::string for_binding_type;
    bool for_inclusive = false;
    ModuleCacheIrExprSummaryPtr for_start;
    ModuleCacheIrExprSummaryPtr for_end;
    std::vector<ModuleCacheIrExprSummaryPtr> for_values;
    ModuleCacheIrExprSummaryPtr match_value;
    std::vector<ModuleCacheIrBindingSummary> init_bindings;
    std::vector<ModuleCacheIrExprSummaryPtr> updates;
    std::vector<ModuleCacheIrStmtMatchArmSummary> match_arms;
    std::string break_label;
    ModuleCacheIrExprSummaryPtr break_value;
};

struct ModuleCacheIrBodySummary {
    std::map<std::string, std::uint64_t> statement_shape;
    std::map<std::string, std::uint64_t> expression_shape;
    std::vector<ModuleCacheIrStmtSummaryPtr> statements;
};

void append_ir_summary_body_shape(std::string& out, const std::vector<IrStmtPtr>& body);
void append_ir_summary_body_tree(std::string& out, const std::vector<IrStmtPtr>& body);
ModuleCacheIrBodySummary read_module_cache_ir_summary_body_payload(const std::string& text,
                                                                   std::size_t& pos);

} // namespace ari
