#include "pointer_memory_semantics.hpp"

#include "common.hpp"
#include "ir_builders.hpp"
#include "layout.hpp"
#include "type_semantics.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(loc, message);
}

IrType primitive_type(IrPrimitiveKind primitive, std::string name, SourceLocation loc) {
    IrType type;
    type.primitive = primitive;
    type.name = std::move(name);
    type.loc = loc;
    return type;
}

IrType void_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::Void, "void", loc);
}

IrType i64_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::I64, "i64", loc);
}

std::optional<IrType> resolve_optional_pointer_helper_type_arg(
    const Expr& expr,
    const std::string& operation,
    PointerMemorySemanticContext& context) {
    if (expr_type_args(expr).empty()) return std::nullopt;
    if (expr_type_args(expr).size() != 1) {
        fail(expr.loc, operation + " expects at most one type argument");
    }
    IrType element_type = context.resolve_executable_type(expr_type_args(expr)[0]);
    if (element_type.qualifier != TypeQualifier::Value) {
        fail(expr_type_args(expr)[0].loc, operation + "<T> expects a value type, got " + type_name(element_type));
    }
    element_type.qualifier = TypeQualifier::Ptr;
    return element_type;
}

IrExprPtr check_pointer_helper_pointer_arg(const Expr& arg,
                                           const std::optional<IrType>& expected_pointer,
                                           PointerMemorySemanticContext& context) {
    IrExprPtr pointer = context.check_expr(arg);
    if (expected_pointer) {
        context.coerce_pointer_memory_expr_to_expected(*pointer, *expected_pointer);
        require_assignable(arg.loc, *expected_pointer, pointer->type);
    }
    return pointer;
}

IrType resolve_optional_mem_place_type_arg(const Expr& expr,
                                           const std::string& operation,
                                           PointerMemorySemanticContext& context) {
    if (expr_type_args(expr).size() > 1) {
        fail(expr.loc, operation + " expects at most one type argument");
    }
    if (expr_type_args(expr).empty()) return {};
    IrType explicit_type = context.resolve_executable_type(expr_type_args(expr)[0]);
    if (explicit_type.qualifier != TypeQualifier::Value) {
        fail(expr_type_args(expr)[0].loc, operation + "<T> expects a value type, got " + type_name(explicit_type));
    }
    return explicit_type;
}

IrType require_mem_place_mut_borrow(SourceLocation loc,
                                    const IrExpr& place,
                                    const std::string& operation,
                                    const std::string& argument_name) {
    if (place.type.qualifier != TypeQualifier::MutRef) {
        fail(loc, operation + " " + argument_name + " must be a mutable borrow, got " + type_name(place.type));
    }
    IrType element_type = place.type;
    element_type.qualifier = TypeQualifier::Value;
    return element_type;
}

void require_mem_place_type(SourceLocation loc,
                            const IrType& inferred,
                            const IrType& explicit_type,
                            bool has_explicit_type,
                            const std::string& operation,
                            const std::string& argument_name) {
    if (has_explicit_type && !same_type(inferred, explicit_type)) {
        fail(loc,
             operation + " " + argument_name + " type mismatch: expected " +
                 type_name(explicit_type) + ", got " + type_name(inferred));
    }
    IrType pointer_type = has_explicit_type ? explicit_type : inferred;
    pointer_type.qualifier = TypeQualifier::Ptr;
    (void)require_raw_pointer_materializable_type(loc, pointer_type, operation);
}

IrStmtPtr make_ir_expr_stmt(SourceLocation loc, IrExprPtr expr) {
    auto stmt = std::make_unique<IrStmt>();
    stmt->kind = IrStmtKind::ExprStmt;
    stmt->loc = loc;
    stmt->expr = std::move(expr);
    return stmt;
}

} // namespace

IrExprPtr lower_pointer_offset_call(const Expr& expr, PointerMemorySemanticContext& context) {
    std::optional<IrType> expected_pointer = resolve_optional_pointer_helper_type_arg(expr, "ptr_offset", context);
    if (expr.args.size() != 2) fail(expr.loc, "ptr_offset expects a pointer and a byte offset");

    std::size_t borrow_mark = context.temporary_borrow_mark();
    IrExprPtr pointer = check_pointer_helper_pointer_arg(*expr.args[0], expected_pointer, context);
    IrExprPtr offset = context.check_expr(*expr.args[1]);
    context.release_temporary_borrows(borrow_mark);

    if (!is_raw_pointer_type(pointer->type)) {
        fail(expr.args[0]->loc, "ptr_offset first argument must be a raw pointer, got " + type_name(pointer->type));
    }
    if (!is_value_integer_type(offset->type)) {
        fail(expr.args[1]->loc, "ptr_offset byte offset must be an integer, got " + type_name(offset->type));
    }

    return make_pointer_offset_expr(expr.loc, std::move(pointer), std::move(offset));
}

IrExprPtr lower_pointer_add_call(const Expr& expr, PointerMemorySemanticContext& context) {
    std::optional<IrType> expected_pointer = resolve_optional_pointer_helper_type_arg(expr, "ptr_add", context);
    if (expr.args.size() != 2) fail(expr.loc, "ptr_add expects a pointer and an element offset");

    std::size_t borrow_mark = context.temporary_borrow_mark();
    IrExprPtr pointer = check_pointer_helper_pointer_arg(*expr.args[0], expected_pointer, context);
    IrExprPtr offset = context.check_expr(*expr.args[1]);
    context.release_temporary_borrows(borrow_mark);

    (void)require_raw_pointer_add_type(expr.args[0]->loc, pointer->type, "ptr_add");
    if (!is_value_integer_type(offset->type)) {
        fail(expr.args[1]->loc, "ptr_add element offset must be an integer, got " + type_name(offset->type));
    }

    return make_pointer_add_expr(expr.loc, std::move(pointer), std::move(offset));
}

IrExprPtr lower_pointer_load_call(const Expr& expr, PointerMemorySemanticContext& context) {
    std::optional<IrType> expected_pointer = resolve_optional_pointer_helper_type_arg(expr, "ptr_load", context);
    if (expr.args.size() != 1) fail(expr.loc, "ptr_load expects one pointer");

    std::size_t borrow_mark = context.temporary_borrow_mark();
    IrExprPtr pointer = check_pointer_helper_pointer_arg(*expr.args[0], expected_pointer, context);
    context.release_temporary_borrows(borrow_mark);

    IrType element_type = require_raw_pointer_materializable_type(expr.args[0]->loc, pointer->type, "ptr_load");
    return make_pointer_load_expr(expr.loc, std::move(pointer), element_type);
}

IrExprPtr lower_pointer_store_call(const Expr& expr, PointerMemorySemanticContext& context) {
    std::optional<IrType> expected_pointer = resolve_optional_pointer_helper_type_arg(expr, "ptr_store", context);
    if (expr.args.size() != 2) fail(expr.loc, "ptr_store expects a pointer and a value");

    std::size_t borrow_mark = context.temporary_borrow_mark();
    IrExprPtr pointer = check_pointer_helper_pointer_arg(*expr.args[0], expected_pointer, context);
    IrType element_type = require_raw_pointer_materializable_type(expr.args[0]->loc, pointer->type, "ptr_store");
    IrExprPtr value = context.check_expr(*expr.args[1]);
    context.coerce_pointer_memory_expr_to_expected(*value, element_type);
    require_assignable(expr.args[1]->loc, element_type, value->type);
    context.release_temporary_borrows(borrow_mark);

    return make_pointer_store_expr(expr.loc, std::move(pointer), std::move(value));
}

IrExprPtr lower_mem_replace_call(const Expr& expr, PointerMemorySemanticContext& context) {
    const std::string operation = "std::mem::replace";
    if (expr.args.size() != 2) fail(expr.loc, operation + " expects a mutable place and a value");

    IrType explicit_type = resolve_optional_mem_place_type_arg(expr, operation, context);
    bool has_explicit_type = !expr_type_args(expr).empty();

    std::size_t borrow_mark = context.temporary_borrow_mark();
    IrExprPtr target = context.check_expr(*expr.args[0]);
    IrType element_type = require_mem_place_mut_borrow(
        expr.args[0]->loc,
        *target,
        operation,
        "target");
    require_mem_place_type(
        expr.args[0]->loc,
        element_type,
        explicit_type,
        has_explicit_type,
        operation,
        "target");
    if (has_explicit_type) element_type = explicit_type;

    IrExprPtr value = context.check_expr_maybe_expected(*expr.args[1], &element_type);
    context.coerce_pointer_memory_expr_to_expected(*value, element_type);
    require_assignable(expr.args[1]->loc, element_type, value->type);
    context.release_temporary_borrows(borrow_mark);

    IrType pointer_type = element_type;
    pointer_type.qualifier = TypeQualifier::Ptr;
    std::string raw_name = context.make_hidden_local("$mem_replace_raw");
    std::string previous_name = context.make_hidden_local("$mem_replace_previous");
    context.declare_local(expr.loc, raw_name, pointer_type, false);
    context.declare_local(expr.loc, previous_name, element_type, false);

    std::vector<IrStmtPtr> body;
    body.push_back(make_ir_var_decl(
        expr.loc,
        raw_name,
        pointer_type,
        make_cast_expr(expr.args[0]->loc, std::move(target), pointer_type),
        false));
    body.push_back(make_ir_var_decl(
        expr.loc,
        previous_name,
        element_type,
        make_pointer_load_expr(
            expr.loc,
            make_local_lvalue_expr(expr.loc, raw_name, pointer_type),
            element_type),
        false));
    body.push_back(make_ir_expr_stmt(
        expr.loc,
        make_pointer_store_expr(
            expr.loc,
            make_local_lvalue_expr(expr.loc, raw_name, pointer_type),
            std::move(value))));

    return make_ir_block_expr(
        expr.loc,
        {},
        element_type,
        std::move(body),
        make_local_lvalue_expr(expr.loc, previous_name, element_type));
}

IrExprPtr lower_mem_swap_call(const Expr& expr, PointerMemorySemanticContext& context) {
    const std::string operation = "std::mem::swap";
    if (expr.args.size() != 2) fail(expr.loc, operation + " expects two mutable places");

    IrType explicit_type = resolve_optional_mem_place_type_arg(expr, operation, context);
    bool has_explicit_type = !expr_type_args(expr).empty();

    std::size_t borrow_mark = context.temporary_borrow_mark();
    IrExprPtr left = context.check_expr(*expr.args[0]);
    IrType element_type = require_mem_place_mut_borrow(
        expr.args[0]->loc,
        *left,
        operation,
        "left argument");
    require_mem_place_type(
        expr.args[0]->loc,
        element_type,
        explicit_type,
        has_explicit_type,
        operation,
        "left argument");
    if (has_explicit_type) element_type = explicit_type;

    IrExprPtr right = context.check_expr(*expr.args[1]);
    IrType right_type = require_mem_place_mut_borrow(
        expr.args[1]->loc,
        *right,
        operation,
        "right argument");
    if (!same_type(right_type, element_type)) {
        fail(expr.args[1]->loc,
             operation + " right argument type mismatch: expected " +
                 type_name(element_type) + ", got " + type_name(right_type));
    }
    context.release_temporary_borrows(borrow_mark);

    IrType pointer_type = element_type;
    pointer_type.qualifier = TypeQualifier::Ptr;
    std::string left_raw_name = context.make_hidden_local("$mem_swap_left");
    std::string right_raw_name = context.make_hidden_local("$mem_swap_right");
    std::string temporary_name = context.make_hidden_local("$mem_swap_temporary");
    context.declare_local(expr.loc, left_raw_name, pointer_type, false);
    context.declare_local(expr.loc, right_raw_name, pointer_type, false);
    context.declare_local(expr.loc, temporary_name, element_type, false);

    std::vector<IrStmtPtr> body;
    body.push_back(make_ir_var_decl(
        expr.loc,
        left_raw_name,
        pointer_type,
        make_cast_expr(expr.args[0]->loc, std::move(left), pointer_type),
        false));
    body.push_back(make_ir_var_decl(
        expr.loc,
        right_raw_name,
        pointer_type,
        make_cast_expr(expr.args[1]->loc, std::move(right), pointer_type),
        false));
    body.push_back(make_ir_var_decl(
        expr.loc,
        temporary_name,
        element_type,
        make_pointer_load_expr(
            expr.loc,
            make_local_lvalue_expr(expr.loc, left_raw_name, pointer_type),
            element_type),
        false));
    body.push_back(make_ir_expr_stmt(
        expr.loc,
        make_pointer_store_expr(
            expr.loc,
            make_local_lvalue_expr(expr.loc, left_raw_name, pointer_type),
            make_pointer_load_expr(
                expr.loc,
                make_local_lvalue_expr(expr.loc, right_raw_name, pointer_type),
                element_type))));

    IrExprPtr final_store = make_pointer_store_expr(
        expr.loc,
        make_local_lvalue_expr(expr.loc, right_raw_name, pointer_type),
        make_local_lvalue_expr(expr.loc, temporary_name, element_type));
    return make_ir_block_expr(
        expr.loc,
        {},
        void_type(expr.loc),
        std::move(body),
        std::move(final_store));
}

IrExprPtr lower_layout_query_call(const Expr& expr, bool align_query, PointerMemorySemanticContext& context) {
    const std::string operation = align_query ? "align_of" : "size_of";
    if (expr_type_args(expr).size() != 1) {
        fail(expr.loc, operation + " expects exactly one type argument");
    }
    if (!expr.args.empty()) {
        fail(expr.loc, operation + " does not take value arguments");
    }

    IrType queried = context.resolve_executable_type(expr_type_args(expr)[0]);
    std::uint64_t bytes = 0;
    bool supported = align_query
        ? ari_layout_align_bytes(queried, bytes)
        : ari_layout_size_bytes(queried, bytes);
    if (!supported) {
        fail(expr_type_args(expr)[0].loc, operation + " does not support " + type_name(queried));
    }
    if (bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        fail(expr_type_args(expr)[0].loc, operation + " result is too large for i64");
    }
    return make_integer_literal(expr.loc, i64_type(expr.loc), bytes);
}

} // namespace ari
