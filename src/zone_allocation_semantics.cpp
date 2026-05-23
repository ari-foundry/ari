#include "zone_allocation_semantics.hpp"

#include "common.hpp"
#include "ir_builders.hpp"
#include "layout.hpp"
#include "type_semantics.hpp"

#include <cstdint>
#include <limits>
#include <string>
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

IrType i64_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::I64, "i64", loc);
}

IrType zone_type(SourceLocation loc, TypeQualifier qualifier) {
    IrType zone = primitive_type(IrPrimitiveKind::Zone, "Zone", loc);
    zone.qualifier = qualifier;
    return zone;
}

IrType require_zone_allocated_type(const Expr& expr, const std::string& operation, ZoneAllocationSemanticContext& context) {
    if (expr_type_args(expr).size() != 1) {
        fail(expr.loc, operation + "<T> expects exactly one type argument");
    }

    IrType allocated = context.resolve_executable_type(expr_type_args(expr)[0]);
    if (allocated.qualifier != TypeQualifier::Value) {
        fail(expr_type_args(expr)[0].loc, operation + "<T> expects a value type, got " + type_name(allocated));
    }
    return allocated;
}

void require_copyable_zone_allocated_type(const Expr& expr, const IrType& allocated, const std::string& operation) {
    if (is_owner_type(allocated) || contains_borrow_type(allocated)) {
        fail(expr_type_args(expr)[0].loc, operation + "<T> cannot " +
            (operation == "zone::promote" ? "copy" : "place") +
            " ownership- or borrow-valued types yet");
    }
}

struct ZoneAllocationLayout {
    std::uint64_t size_bytes = 0;
    std::uint64_t align_bytes = 0;
};

ZoneAllocationLayout require_zone_allocation_layout(
    const Expr& expr,
    const IrType& allocated,
    const std::string& operation) {
    ZoneAllocationLayout layout;
    if (!ari_layout_size_bytes(allocated, layout.size_bytes) ||
        !ari_layout_align_bytes(allocated, layout.align_bytes)) {
        fail(expr_type_args(expr)[0].loc, operation + "<T> does not support " + type_name(allocated));
    }
    if (layout.size_bytes == 0) {
        fail(expr_type_args(expr)[0].loc, operation + "<T> requires a non-zero-sized type");
    }
    if (layout.size_bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) ||
        layout.align_bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        fail(expr_type_args(expr)[0].loc, operation + "<T> layout is too large for i64");
    }
    return layout;
}

IrExprPtr checked_zone_arg(const Expr& arg, ZoneAllocationSemanticContext& context) {
    IrType zone = zone_type(arg.loc, TypeQualifier::MutRef);
    IrExprPtr zone_arg = context.check_expr(arg);
    context.coerce_zone_allocation_expr_to_expected(*zone_arg, zone);
    require_assignable(arg.loc, zone, zone_arg->type);
    return zone_arg;
}

IrExprPtr make_i64_layout_literal(SourceLocation loc, std::uint64_t value) {
    return make_integer_literal(loc, i64_type(loc), value);
}

std::vector<IrExprPtr> zone_new_args(SourceLocation loc,
                                     IrExprPtr zone_arg,
                                     const ZoneAllocationLayout& layout,
                                     IrExprPtr value) {
    std::vector<IrExprPtr> args;
    args.reserve(4);
    args.push_back(std::move(zone_arg));
    args.push_back(make_i64_layout_literal(loc, layout.size_bytes));
    args.push_back(make_i64_layout_literal(loc, layout.align_bytes));
    args.push_back(std::move(value));
    return args;
}

} // namespace

IrExprPtr lower_typed_zone_alloc_call(const Expr& expr, ZoneAllocationSemanticContext& context) {
    if (expr_type_args(expr).empty()) return nullptr;
    if (expr.args.size() != 1) {
        fail(expr.loc, "zone::alloc<T> expects exactly one zone argument");
    }

    IrType allocated = require_zone_allocated_type(expr, "zone::alloc", context);
    ZoneAllocationLayout layout = require_zone_allocation_layout(expr, allocated, "zone::alloc");

    std::size_t borrow_mark = context.temporary_borrow_mark();
    IrExprPtr zone_arg = checked_zone_arg(*expr.args[0], context);

    IrType pointer_type = allocated;
    pointer_type.qualifier = TypeQualifier::Ptr;
    std::vector<IrExprPtr> args;
    args.reserve(3);
    args.push_back(std::move(zone_arg));
    args.push_back(make_i64_layout_literal(expr.loc, layout.size_bytes));
    args.push_back(make_i64_layout_literal(expr.loc, layout.align_bytes));
    context.release_temporary_borrows(borrow_mark);
    return make_ir_call_expr(expr.loc, "zone::alloc", std::move(pointer_type), std::move(args));
}

IrExprPtr lower_zone_new_call(const Expr& expr, ZoneAllocationSemanticContext& context) {
    if (expr.args.size() != 2) {
        fail(expr.loc, "zone::new<T> expects a zone and a value");
    }

    IrType allocated = require_zone_allocated_type(expr, "zone::new", context);
    require_copyable_zone_allocated_type(expr, allocated, "zone::new");
    ZoneAllocationLayout layout = require_zone_allocation_layout(expr, allocated, "zone::new");

    std::size_t borrow_mark = context.temporary_borrow_mark();
    IrExprPtr zone_arg = checked_zone_arg(*expr.args[0], context);

    IrExprPtr value = context.check_expr(*expr.args[1]);
    context.coerce_zone_allocation_expr_to_expected(*value, allocated);
    require_assignable(expr.args[1]->loc, allocated, value->type);

    IrType pointer_type = allocated;
    pointer_type.qualifier = TypeQualifier::Ptr;
    std::vector<IrExprPtr> args = zone_new_args(expr.loc, std::move(zone_arg), layout, std::move(value));
    context.release_temporary_borrows(borrow_mark);
    return make_ir_call_expr(expr.loc, "zone::new", std::move(pointer_type), std::move(args));
}

IrExprPtr lower_zone_promote_call(const Expr& expr, ZoneAllocationSemanticContext& context) {
    if (expr.args.size() != 2) {
        fail(expr.loc, "zone::promote<T> expects a target zone and source pointer");
    }

    IrType allocated = require_zone_allocated_type(expr, "zone::promote", context);
    require_copyable_zone_allocated_type(expr, allocated, "zone::promote");
    ZoneAllocationLayout layout = require_zone_allocation_layout(expr, allocated, "zone::promote");

    IrType source_pointer_type = allocated;
    source_pointer_type.qualifier = TypeQualifier::Ptr;

    std::size_t borrow_mark = context.temporary_borrow_mark();
    IrExprPtr zone_arg = checked_zone_arg(*expr.args[0], context);

    IrExprPtr source = context.check_expr(*expr.args[1]);
    context.coerce_zone_allocation_expr_to_expected(*source, source_pointer_type);
    require_assignable(expr.args[1]->loc, source_pointer_type, source->type);

    IrExprPtr value = make_pointer_load_expr(expr.args[1]->loc, std::move(source), allocated);
    std::vector<IrExprPtr> args = zone_new_args(expr.loc, std::move(zone_arg), layout, std::move(value));
    context.release_temporary_borrows(borrow_mark);
    return make_ir_call_expr(expr.loc, "zone::new", std::move(source_pointer_type), std::move(args));
}

IrExprPtr lower_zone_temp_call(const Expr& expr, ZoneAllocationSemanticContext& context) {
    if (!expr_type_args(expr).empty()) {
        fail(expr.loc, "zone::temp does not take type arguments");
    }
    if (expr.args.size() != 1) {
        fail(expr.loc, "zone::temp expects a capacity");
    }

    IrType capacity_type = i64_type(expr.loc);
    std::size_t borrow_mark = context.temporary_borrow_mark();
    IrExprPtr capacity = context.check_expr(*expr.args[0]);
    context.coerce_zone_allocation_expr_to_expected(*capacity, capacity_type);
    require_assignable(expr.args[0]->loc, capacity_type, capacity->type);
    context.release_temporary_borrows(borrow_mark);

    std::vector<IrExprPtr> args;
    args.push_back(std::move(capacity));
    return make_ir_call_expr(expr.loc, "zone::temp", zone_type(expr.loc, TypeQualifier::Own), std::move(args));
}

} // namespace ari
