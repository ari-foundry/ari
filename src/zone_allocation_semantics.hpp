#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <cstddef>

namespace ari {

class ZoneAllocationSemanticContext {
public:
    virtual ~ZoneAllocationSemanticContext() = default;

    virtual IrType resolve_executable_type(const TypeRef& ast_type) = 0;
    virtual IrExprPtr check_expr(const Expr& expr) = 0;
    virtual void coerce_zone_allocation_expr_to_expected(IrExpr& expr, const IrType& expected) = 0;
    virtual std::size_t temporary_borrow_mark() const = 0;
    virtual void release_temporary_borrows(std::size_t mark) = 0;
};

IrExprPtr lower_typed_zone_alloc_call(const Expr& expr, ZoneAllocationSemanticContext& context);
IrExprPtr lower_zone_new_call(const Expr& expr, ZoneAllocationSemanticContext& context);
IrExprPtr lower_zone_promote_call(const Expr& expr, ZoneAllocationSemanticContext& context);
IrExprPtr lower_zone_temp_call(const Expr& expr, ZoneAllocationSemanticContext& context);

} // namespace ari
