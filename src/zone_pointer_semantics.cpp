#include "zone_pointer_semantics.hpp"

#include "prelude_resolver.hpp"
#include "slice_semantics.hpp"
#include "std_box_semantics.hpp"
#include "std_vec_semantics.hpp"

namespace ari {

const IrExpr& zone_pointer_source_expr(const IrExpr& value) {
    if (value.kind == IrExprKind::Cast && ir_expr_operand(value)) {
        return zone_pointer_source_expr(*ir_expr_operand(value));
    }
    return value;
}

std::string zone_pointer_escape_name(const IrExpr& value) {
    const IrExpr& source = zone_pointer_source_expr(value);
    if (source.kind == IrExprKind::Local) return ir_expr_name(source);
    return "value";
}

bool is_zone_pointer_trackable_type(const IrType& type) {
    IrType value_type = type;
    if (value_type.qualifier == TypeQualifier::Ref ||
        value_type.qualifier == TypeQualifier::MutRef) {
        value_type.qualifier = TypeQualifier::Value;
    }
    return type.qualifier == TypeQualifier::Ptr ||
           is_std_box_handle_type(value_type) ||
           is_std_vec_zone_handle_type(value_type) ||
           is_prelude_slice_type(value_type);
}

bool zone_pointer_source_name_from_expr(const IrExpr& value,
                                        const ZonePointerSourceResolver& resolver,
                                        std::string& out) {
    if (!is_zone_pointer_trackable_type(value.type)) return false;
    const IrExpr& source = zone_pointer_source_expr(value);
    auto merge_source = [&](const IrExprPtr& expr, bool& found_any) {
        if (!expr) return;
        std::string nested_source;
        if (!zone_pointer_source_name_from_expr(*expr, resolver, nested_source)) return;
        if (!found_any) {
            out = nested_source;
            found_any = true;
        } else if (out != nested_source) {
            out = "<multiple zones>";
        }
    };

    if (source.kind == IrExprKind::Borrow && ir_expr_label(source).empty()) {
        return resolver.local_zone_pointer_source(ir_expr_name(source), out);
    }
    if (source.kind == IrExprKind::Tuple && is_std_vec_zone_handle_type(source.type)) {
        std::optional<std::size_t> source_index = std_vec_zone_handle_source_field_index(source.type);
        if (!source_index || *source_index >= source.args.size()) return false;
        return zone_pointer_source_name_from_expr(*source.args[*source_index], resolver, out);
    }
    if (source.kind == IrExprKind::Tuple && is_std_box_handle_type(source.type)) {
        std::optional<std::size_t> source_index = std_box_zone_handle_source_field_index(source.type);
        if (!source_index || *source_index >= source.args.size()) return false;
        return zone_pointer_source_name_from_expr(*source.args[*source_index], resolver, out);
    }
    if (source.kind == IrExprKind::Tuple && is_prelude_slice_type(source.type)) {
        if (source.args.empty()) return false;
        return zone_pointer_source_name_from_expr(*source.args[0], resolver, out);
    }
    if (source.kind == IrExprKind::TupleIndex && ir_expr_operand(source)) {
        const IrExpr& operand = *ir_expr_operand(source);
        std::optional<std::size_t> box_source_index =
            std_box_zone_handle_source_field_index(operand.type);
        if (box_source_index && source.tuple_index == *box_source_index) {
            return zone_pointer_source_name_from_expr(operand, resolver, out);
        }
        std::optional<std::size_t> source_index = std_vec_zone_handle_source_field_index(operand.type);
        if (source_index && source.tuple_index == *source_index) {
            return zone_pointer_source_name_from_expr(operand, resolver, out);
        }
        if (is_prelude_slice_type(operand.type) && source.tuple_index == 0) {
            return zone_pointer_source_name_from_expr(operand, resolver, out);
        }
        return false;
    }
    if (source.kind == IrExprKind::SliceRange && ir_expr_operand(source)) {
        return zone_pointer_source_name_from_expr(*ir_expr_operand(source), resolver, out);
    }
    if ((source.kind == IrExprKind::Call && ir_expr_name(source) == "zone::alloc") ||
        (source.kind == IrExprKind::Call && ir_expr_name(source) == "zone::new")) {
        return !source.args.empty() && resolver.zone_source_from_arg(*source.args[0], out);
    }
    if (source.kind == IrExprKind::Call) {
        if (std_box_pointer_result_preserves_receiver_zone(source) &&
            zone_pointer_source_name_from_expr(*source.args[0], resolver, out)) {
            return true;
        }
        if (std_vec_pointer_result_preserves_receiver_zone(source) &&
            zone_pointer_source_name_from_expr(*source.args[0], resolver, out)) {
            return true;
        }
        if (is_prelude_slice_type(source.type) &&
            !source.args.empty() &&
            zone_pointer_source_name_from_expr(*source.args[0], resolver, out)) {
            return true;
        }
        if (is_zone_alloc_function_name(ir_expr_name(source)) ||
            is_zone_new_function_name(ir_expr_name(source))) {
            return !source.args.empty() && resolver.zone_source_from_arg(*source.args[0], out);
        }
        std::optional<std::size_t> zone_index =
            resolver.call_zone_return_param_index(ir_expr_name(source));
        if (!zone_index || *zone_index >= source.args.size()) return false;
        return resolver.zone_source_from_arg(*source.args[*zone_index], out);
    }
    if (source.kind == IrExprKind::Local) {
        return resolver.local_zone_pointer_source(ir_expr_name(source), out);
    }
    if (source.kind == IrExprKind::If) {
        bool found_any = false;
        merge_source(ir_expr_if_then_value(source), found_any);
        merge_source(ir_expr_if_else_value(source), found_any);
        return found_any;
    }
    if (source.kind == IrExprKind::Block) {
        return ir_expr_block_value(source) &&
               zone_pointer_source_name_from_expr(*ir_expr_block_value(source), resolver, out);
    }
    if (source.kind == IrExprKind::Match) {
        bool found_any = false;
        for (const auto& arm : ir_expr_match_arms(source)) {
            merge_source(arm.value, found_any);
        }
        return found_any;
    }
    return false;
}

} // namespace ari
