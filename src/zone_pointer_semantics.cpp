#include "zone_pointer_semantics.hpp"

#include "ari_builtin.hpp"
#include "local_state.hpp"
#include "prelude_resolver.hpp"
#include "slice_semantics.hpp"
#include "std_box_semantics.hpp"
#include "std_vec_semantics.hpp"
#include "zone_return_semantics.hpp"

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

bool zone_source_name_from_arg(const IrExpr& zone_arg,
                               const ZonePointerLocalAdapter& locals,
                               std::string& out) {
    if (zone_arg.kind == IrExprKind::Borrow && ir_expr_label(zone_arg).empty()) {
        const LocalInfo* zone = locals.find_local(ir_expr_name(zone_arg));
        if (!zone || !is_zone_value_type(zone->type)) return false;
        out = ir_expr_name(zone_arg);
        return true;
    }
    if (zone_arg.kind == IrExprKind::Local) {
        const LocalInfo* zone = locals.find_local(ir_expr_name(zone_arg));
        if (!zone || !is_zone_borrow_type(zone->type)) return false;
        if (!zone->borrow_source.empty()) {
            const LocalInfo* source = locals.find_local(zone->borrow_source);
            if (source && is_zone_source_type(source->type)) {
                out = zone->borrow_source;
                return true;
            }
        }
        out = ir_expr_name(zone_arg);
        return true;
    }
    return false;
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

void clear_zone_pointer_source(LocalInfo& local) {
    local.zone_pointer = false;
    local.zone_pointer_source.clear();
    local.zone_pointer_generation = 0;
}

bool set_zone_pointer_source_from_name(LocalInfo& target,
                                       const std::string& source_name,
                                       const ZonePointerLocalAdapter& locals) {
    if (source_name == "<multiple zones>") return false;
    LocalInfo* zone = locals.find_local(source_name);
    if (!zone || !is_zone_source_type(zone->type)) return false;
    target.zone_pointer = true;
    target.zone_pointer_source = source_name;
    target.zone_pointer_generation = zone->zone_generation;
    return true;
}

bool set_zone_pointer_source_from_expr(LocalInfo& target,
                                       const IrExpr& value,
                                       const ZonePointerSourceResolver& resolver,
                                       const ZonePointerLocalAdapter& locals) {
    clear_zone_pointer_source(target);
    if (!is_zone_pointer_trackable_type(target.type)) return false;

    std::string source_name;
    if (!zone_pointer_source_name_from_expr(value, resolver, source_name)) return false;
    return set_zone_pointer_source_from_name(target, source_name, locals);
}

std::optional<std::string> zone_pointer_invalid_error(const std::string& pointer_name,
                                                      const LocalInfo& pointer,
                                                      const ZonePointerLocalAdapter& locals) {
    if (!pointer.zone_pointer) return std::nullopt;
    const LocalInfo* zone = locals.find_local(pointer.zone_pointer_source);
    if (zone &&
        is_zone_source_type(zone->type) &&
        local_is_alive(*zone) &&
        zone->zone_generation == pointer.zone_pointer_generation) {
        return std::nullopt;
    }
    return "cannot use zone pointer '" + pointer_name + "' after zone '" +
           pointer.zone_pointer_source + "' was reset or destroyed";
}

bool mark_zone_reset_call(const IrExpr& call, const ZonePointerLocalAdapter& locals) {
    if (call.kind != IrExprKind::Call || call.args.empty()) return false;
    std::optional<std::string> builtin_symbol = ari_builtin_symbol_for_source_name(ir_expr_name(call));
    if (!builtin_symbol || *builtin_symbol != "ari_builtin_zone_reset") return false;
    std::string source_name;
    if (!zone_source_name_from_arg(*call.args[0], locals, source_name)) return false;
    LocalInfo* zone = locals.find_local(source_name);
    if (!zone || !is_zone_source_type(zone->type)) return false;
    bump_local_zone_generation(*zone);
    return true;
}

} // namespace ari
