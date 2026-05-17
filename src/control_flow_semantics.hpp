#pragma once

#include "ir.hpp"

#include <functional>
#include <string>
#include <vector>

namespace ari {

struct TupleCheckedStmtArm {
    SourceLocation loc;
    IrExprPtr condition;
    std::vector<IrStmtPtr> body;
};

struct TupleCheckedExprArm {
    SourceLocation loc;
    IrExprPtr condition;
    std::vector<IrStmtPtr> body;
    IrExprPtr value;
};

IrExprPtr make_tuple_match_block_value(SourceLocation loc,
                                       IrType result_type,
                                       std::vector<IrStmtPtr> body,
                                       IrExprPtr value);

std::vector<IrStmtPtr> build_tuple_match_if_chain(std::vector<TupleCheckedStmtArm>& arms);

IrExprPtr build_tuple_match_if_expr_chain(
    std::vector<TupleCheckedExprArm>& arms,
    const IrType& result_type,
    const std::function<IrExprPtr(SourceLocation, const IrType&)>& make_fallback
);

bool is_diverging_builtin_symbol(const std::string& symbol);
bool is_diverging_builtin_source_name(const std::string& source_name);
bool is_diverging_builtin_call(const IrExpr& expr);
bool is_diverging_control_flow_value(const IrExpr& expr);
bool enum_match_arm_has_refutable_payload_condition(const IrMatchArm& arm);
bool enum_construct_matches_arm_without_refutable_payload_conditions(
    const IrExpr& match_value,
    const std::vector<IrMatchArm>& pattern_arms
);

} // namespace ari
