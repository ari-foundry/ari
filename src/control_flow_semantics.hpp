#pragma once

#include "ir.hpp"

#include <functional>
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

} // namespace ari
