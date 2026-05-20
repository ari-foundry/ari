#include "zone_pointer_semantics.hpp"

#include "ari_builtin.hpp"
#include "ir_builders.hpp"
#include "local_state.hpp"
#include "prelude_resolver.hpp"
#include "slice_semantics.hpp"
#include "std_box_semantics.hpp"
#include "std_collections_semantics.hpp"
#include "std_fs_semantics.hpp"
#include "std_source_semantics.hpp"
#include "std_string_semantics.hpp"
#include "std_vec_semantics.hpp"
#include "type_semantics.hpp"
#include "zone_return_semantics.hpp"

#include <algorithm>
#include <map>
#include <utility>

namespace ari {

bool is_auto_destroy_zone(const LocalInfo& local) {
    return local.auto_destroy_zone &&
           local.type.qualifier == TypeQualifier::Own &&
           local.type.primitive == IrPrimitiveKind::Zone;
}

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
    if (type.qualifier == TypeQualifier::Own &&
        type.primitive == IrPrimitiveKind::TraitObject) {
        return true;
    }
    return type.qualifier == TypeQualifier::Ptr ||
           is_std_box_handle_type(value_type) ||
           is_std_collections_zone_handle_type(value_type) ||
           is_std_fs_dir_entry_zone_handle_type(value_type) ||
           is_std_source_line_map_handle_type(value_type) ||
           is_std_string_zone_handle_type(value_type) ||
           is_std_vec_zone_handle_type(value_type) ||
           is_prelude_slice_type(value_type) ||
           std::any_of(value_type.args.begin(), value_type.args.end(), is_zone_pointer_trackable_type) ||
           std::any_of(value_type.field_types.begin(), value_type.field_types.end(), is_zone_pointer_trackable_type);
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
    if (source.kind == IrExprKind::Tuple && is_std_collections_zone_handle_type(source.type)) {
        std::optional<std::size_t> source_index =
            std_collections_set_zone_handle_source_field_index(source.type);
        if (!source_index || *source_index >= source.args.size()) return false;
        return zone_pointer_source_name_from_expr(*source.args[*source_index], resolver, out);
    }
    if (source.kind == IrExprKind::Tuple && is_std_fs_dir_entry_zone_handle_type(source.type)) {
        bool found_any = false;
        for (const auto& arg : source.args) {
            merge_source(arg, found_any);
        }
        return found_any;
    }
    if (source.kind == IrExprKind::Tuple && is_std_source_line_map_handle_type(source.type)) {
        std::optional<std::size_t> source_index =
            std_source_line_map_zone_handle_source_field_index(source.type);
        if (!source_index || *source_index >= source.args.size()) return false;
        return zone_pointer_source_name_from_expr(*source.args[*source_index], resolver, out);
    }
    if (source.kind == IrExprKind::Tuple && is_std_string_zone_handle_type(source.type)) {
        std::optional<std::size_t> source_index = std_string_zone_handle_source_field_index(source.type);
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
    if (source.kind == IrExprKind::EnumConstruct) {
        bool found_any = false;
        merge_source(ir_expr_payload(source), found_any);
        for (const auto& arg : source.args) {
            merge_source(arg, found_any);
        }
        return found_any;
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
        std::optional<std::size_t> collections_source_index =
            std_collections_set_zone_handle_source_field_index(operand.type);
        if (collections_source_index && source.tuple_index == *collections_source_index) {
            return zone_pointer_source_name_from_expr(operand, resolver, out);
        }
        if (is_std_fs_dir_entry_zone_handle_type(operand.type) &&
            source.tuple_index < operand.type.field_types.size()) {
            return zone_pointer_source_name_from_expr(operand, resolver, out);
        }
        std::optional<std::size_t> source_line_map_index =
            std_source_line_map_zone_handle_source_field_index(operand.type);
        if (source_line_map_index && source.tuple_index == *source_line_map_index) {
            return zone_pointer_source_name_from_expr(operand, resolver, out);
        }
        std::optional<std::size_t> string_source_index =
            std_string_zone_handle_source_field_index(operand.type);
        if (string_source_index && source.tuple_index == *string_source_index) {
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
        if (slice_pointer_result_preserves_receiver_zone(source) &&
            zone_pointer_source_name_from_expr(*source.args[0], resolver, out)) {
            return true;
        }
        if (std_vec_result_preserves_receiver_zone(source) &&
            zone_pointer_source_name_from_expr(*source.args[0], resolver, out)) {
            return true;
        }
        if (std_collections_result_preserves_receiver_zone(source) &&
            zone_pointer_source_name_from_expr(*source.args[0], resolver, out)) {
            return true;
        }
        if (std_string_pointer_result_preserves_receiver_zone(source) &&
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

bool zone_metadata_extern_builtin_allows_zone_pointer_argument(const std::string& function_name,
                                                               std::size_t arg_index) {
    if (arg_index != 0) return false;
    std::optional<std::string> builtin_symbol = ari_builtin_symbol_for_source_name(function_name);
    return builtin_symbol && *builtin_symbol == "ari_builtin_zone_allocation_zone";
}

bool memory_extern_builtin_allows_zone_pointer_argument(const std::string& function_name,
                                                        std::size_t arg_index) {
    std::optional<std::string> builtin_symbol = ari_builtin_symbol_for_source_name(function_name);
    if (!builtin_symbol) return false;
    if (*builtin_symbol == "ari_builtin_mem_copy_bytes" ||
        *builtin_symbol == "ari_builtin_mem_move_bytes") {
        return arg_index == 0 || arg_index == 1;
    }
    if (*builtin_symbol == "ari_builtin_mem_set_bytes") {
        return arg_index == 0;
    }
    return false;
}

bool temporary_zone_source_from_expr(const IrExpr& value,
                                     const ZonePointerSourceResolver& resolver,
                                     const ZonePointerLocalAdapter& locals,
                                     const LocalScopeStack& scopes,
                                     std::string& source_name,
                                     std::size_t& source_scope_index) {
    if (!zone_pointer_source_name_from_expr(value, resolver, source_name)) return false;
    if (source_name == "<multiple zones>") return false;
    LocalInfo* zone = locals.find_local(source_name);
    if (!zone || !is_auto_destroy_zone(*zone)) return false;
    return scopes.scope_index(source_name, source_scope_index);
}

static std::string temporary_zone_pointer_escape_message(const IrExpr& value,
                                                         const std::string& source_name,
                                                         const std::string& context) {
    return "zone pointer '" + zone_pointer_escape_name(value) +
           "' from temporary zone '" + source_name +
           "' cannot escape through " + context;
}

static IrType zone_cleanup_void_type(SourceLocation loc) {
    IrType type;
    type.primitive = IrPrimitiveKind::Void;
    type.name = "void";
    type.loc = loc;
    return type;
}

std::optional<std::string> temporary_zone_pointer_escape_error(const IrExpr& value,
                                                               std::size_t first_scope_index,
                                                               const std::string& context,
                                                               const ZonePointerSourceResolver& resolver,
                                                               const ZonePointerLocalAdapter& locals,
                                                               const LocalScopeStack& scopes) {
    std::string source_name;
    std::size_t source_scope_index = 0;
    if (!temporary_zone_source_from_expr(value, resolver, locals, scopes, source_name, source_scope_index)) {
        return std::nullopt;
    }
    if (source_scope_index < first_scope_index) return std::nullopt;
    return temporary_zone_pointer_escape_message(value, source_name, context);
}

std::optional<std::string> zone_pointer_escape_error(const IrExpr& value,
                                                     const std::string& context,
                                                     const ZonePointerSourceResolver& resolver,
                                                     const ZonePointerLocalAdapter& locals,
                                                     const LocalScopeStack& scopes) {
    std::string source_name;
    if (!zone_pointer_source_name_from_expr(value, resolver, source_name)) return std::nullopt;
    std::size_t source_scope_index = 0;
    if (temporary_zone_source_from_expr(value, resolver, locals, scopes, source_name, source_scope_index)) {
        return temporary_zone_pointer_escape_message(value, source_name, context);
    }
    return "zone pointer cannot escape into " + context + "; keep it in a local ptr binding";
}

std::optional<std::string> outer_temporary_zone_pointer_escape_error(std::size_t first_scope_index,
                                                                     const LocalScopeStack& scopes) {
    if (first_scope_index == 0 || !scopes.contains_scope(first_scope_index)) return std::nullopt;
    std::map<std::string, bool> temporary_zones;
    scopes.for_each_local_from(
        first_scope_index,
        [&](const std::string& name, const LocalInfo& local) {
            if (is_auto_destroy_zone(local) && local_is_alive(local)) {
                temporary_zones[name] = true;
            }
        });
    if (temporary_zones.empty()) return std::nullopt;

    std::optional<std::string> error;
    scopes.for_each_local_before(
        first_scope_index,
        [&](const std::string& name, const LocalInfo& local) {
            if (error) return;
            if (!local.zone_pointer || !local_is_alive(local)) return;
            if (temporary_zones.find(local.zone_pointer_source) == temporary_zones.end()) return;
            error = "zone pointer '" + name +
                    "' from temporary zone '" + local.zone_pointer_source +
                    "' cannot outlive the temporary zone scope";
        });
    return error;
}

IrStmtPtr make_zone_destroy_stmt(SourceLocation loc, const std::string& name, const IrType& type) {
    std::vector<IrExprPtr> args;
    args.push_back(make_local_lvalue_expr(loc, name, type));
    auto stmt = std::make_unique<IrStmt>();
    stmt->kind = IrStmtKind::ExprStmt;
    stmt->loc = loc;
    stmt->expr = make_builtin_call(loc, "zone::destroy", std::move(args), zone_cleanup_void_type(loc));
    return stmt;
}

bool has_auto_destroy_zone_cleanup(const LocalScopeStack& scopes, std::size_t first_scope_index) {
    return scopes.any_local_from(
        first_scope_index,
        [](const std::string&, const LocalInfo& local) {
            return is_auto_destroy_zone(local) && local_is_alive(local);
        });
}

std::optional<std::string> append_auto_destroy_zone_cleanup(SourceLocation loc,
                                                            std::vector<IrStmtPtr>& statements,
                                                            LocalScopeStack& scopes,
                                                            std::size_t first_scope_index) {
    if (!scopes.contains_scope(first_scope_index)) return std::nullopt;
    if (auto error = outer_temporary_zone_pointer_escape_error(first_scope_index, scopes)) {
        return error;
    }
    scopes.for_each_local_from_inner_to_outer(
        first_scope_index,
        [&](const std::string& name, LocalInfo& local) {
            if (!is_auto_destroy_zone(local) || !local_is_alive(local)) return;
            statements.push_back(make_zone_destroy_stmt(loc, name, local.type));
            mark_local_zone_destroyed(local);
        });
    return std::nullopt;
}

std::optional<std::string> require_no_temporary_zone_pointer_escape(
    const IrExpr& value,
    std::size_t first_scope_index,
    const std::string& context,
    const AutoDestroyZoneCleanupContext& cleanup
) {
    return temporary_zone_pointer_escape_error(
        value,
        first_scope_index,
        context,
        cleanup.source_resolver,
        cleanup.locals,
        cleanup.scopes);
}

bool has_auto_destroy_zone_cleanup(const AutoDestroyZoneCleanupContext& cleanup,
                                   std::size_t first_scope_index) {
    return has_auto_destroy_zone_cleanup(cleanup.scopes, first_scope_index);
}

std::optional<std::string> append_auto_destroy_zone_cleanup(
    SourceLocation loc,
    std::vector<IrStmtPtr>& statements,
    AutoDestroyZoneCleanupContext& cleanup,
    std::size_t first_scope_index
) {
    return append_auto_destroy_zone_cleanup(loc, statements, cleanup.scopes, first_scope_index);
}

AutoDestroyZoneMaterialization materialize_value_before_auto_destroy_cleanup(
    SourceLocation loc,
    IrExprPtr value,
    std::vector<IrStmtPtr>& statements,
    AutoDestroyZoneCleanupContext& cleanup,
    std::size_t first_scope_index,
    const std::string& hidden_prefix,
    const std::string& escape_context
) {
    if (value) {
        if (auto error = require_no_temporary_zone_pointer_escape(
                *value,
                first_scope_index,
                escape_context,
                cleanup)) {
            return {std::move(value), error};
        }
    }
    if (!has_auto_destroy_zone_cleanup(cleanup, first_scope_index)) {
        return {std::move(value), std::nullopt};
    }
    if (value && !is_void_value_type(value->type)) {
        IrType saved_type = value->type;
        std::string saved_name = cleanup.make_hidden_local(hidden_prefix);
        statements.push_back(make_ir_var_decl(loc, saved_name, saved_type, std::move(value), false));
        value = make_local_lvalue_expr(loc, saved_name, saved_type);
    }
    if (auto error = append_auto_destroy_zone_cleanup(loc, statements, cleanup, first_scope_index)) {
        return {std::move(value), error};
    }
    return {std::move(value), std::nullopt};
}

std::optional<std::string> materialize_values_before_auto_destroy_cleanup(
    SourceLocation loc,
    std::vector<IrExprPtr>& values,
    std::vector<IrStmtPtr>& statements,
    AutoDestroyZoneCleanupContext& cleanup,
    std::size_t first_scope_index,
    const std::string& hidden_prefix,
    const std::string& escape_context
) {
    if (!has_auto_destroy_zone_cleanup(cleanup, first_scope_index)) return std::nullopt;
    for (auto& value : values) {
        if (!value) continue;
        if (auto error = require_no_temporary_zone_pointer_escape(
                *value,
                first_scope_index,
                escape_context,
                cleanup)) {
            return error;
        }
        if (is_void_value_type(value->type)) continue;
        IrType saved_type = value->type;
        std::string saved_name = cleanup.make_hidden_local(hidden_prefix);
        statements.push_back(make_ir_var_decl(loc, saved_name, saved_type, std::move(value), false));
        value = make_local_lvalue_expr(loc, saved_name, saved_type);
    }
    return append_auto_destroy_zone_cleanup(loc, statements, cleanup, first_scope_index);
}

} // namespace ari
