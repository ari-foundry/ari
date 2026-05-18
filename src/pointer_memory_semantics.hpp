#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <cstddef>
#include <string>

namespace ari {

class PointerMemorySemanticContext {
public:
    virtual ~PointerMemorySemanticContext() = default;

    virtual IrType resolve_executable_type(const TypeRef& ast_type) = 0;
    virtual IrExprPtr check_expr(const Expr& expr) = 0;
    virtual IrExprPtr check_expr_maybe_expected(const Expr& expr, const IrType* expected) = 0;
    virtual void coerce_pointer_memory_expr_to_expected(IrExpr& expr, const IrType& expected) = 0;
    virtual std::size_t temporary_borrow_mark() const = 0;
    virtual void release_temporary_borrows(std::size_t mark) = 0;
    virtual std::string make_hidden_local(const std::string& prefix) = 0;
    virtual void declare_local(SourceLocation loc,
                               const std::string& name,
                               const IrType& type,
                               bool mutable_binding) = 0;
};

IrExprPtr lower_pointer_offset_call(const Expr& expr, PointerMemorySemanticContext& context);
IrExprPtr lower_pointer_add_call(const Expr& expr, PointerMemorySemanticContext& context);
IrExprPtr lower_pointer_load_call(const Expr& expr, PointerMemorySemanticContext& context);
IrExprPtr lower_pointer_store_call(const Expr& expr, PointerMemorySemanticContext& context);
IrExprPtr lower_mem_replace_call(const Expr& expr, PointerMemorySemanticContext& context);
IrExprPtr lower_mem_swap_call(const Expr& expr, PointerMemorySemanticContext& context);
IrExprPtr lower_layout_query_call(const Expr& expr, bool align_query, PointerMemorySemanticContext& context);

} // namespace ari
