#pragma once

#include "ir.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace ari {

class LocalScopeStack;
struct LocalInfo;

struct ZonePointerSourceResolver {
    using LocalZonePointerSourceLookup = std::function<bool(const std::string&, std::string&)>;
    using ZoneArgSourceLookup = std::function<bool(const IrExpr&, std::string&)>;
    using CallZoneReturnParamLookup = std::function<std::optional<std::size_t>(const std::string&)>;

    LocalZonePointerSourceLookup local_zone_pointer_source;
    ZoneArgSourceLookup zone_source_from_arg;
    CallZoneReturnParamLookup call_zone_return_param_index;
};

struct ZonePointerLocalAdapter {
    using LocalLookup = std::function<LocalInfo*(const std::string&)>;

    LocalLookup find_local;
};

struct AutoDestroyZoneCleanupContext {
    using HiddenLocalNameFactory = std::function<std::string(const std::string&)>;

    LocalScopeStack& scopes;
    ZonePointerSourceResolver source_resolver;
    ZonePointerLocalAdapter locals;
    HiddenLocalNameFactory make_hidden_local;
};

struct AutoDestroyZoneMaterialization {
    IrExprPtr value;
    std::optional<std::string> error;
};

bool is_auto_destroy_zone(const LocalInfo& local);
const IrExpr& zone_pointer_source_expr(const IrExpr& value);
std::string zone_pointer_escape_name(const IrExpr& value);
bool is_zone_pointer_trackable_type(const IrType& type);
bool zone_source_name_from_arg(const IrExpr& zone_arg,
                               const ZonePointerLocalAdapter& locals,
                               std::string& out);
bool zone_pointer_source_name_from_expr(const IrExpr& value,
                                        const ZonePointerSourceResolver& resolver,
                                        std::string& out);
void clear_zone_pointer_source(LocalInfo& local);
bool set_zone_pointer_source_from_name(LocalInfo& target,
                                       const std::string& source_name,
                                       const ZonePointerLocalAdapter& locals);
bool set_zone_pointer_source_from_expr(LocalInfo& target,
                                       const IrExpr& value,
                                       const ZonePointerSourceResolver& resolver,
                                       const ZonePointerLocalAdapter& locals);
std::optional<std::string> zone_pointer_invalid_error(const std::string& pointer_name,
                                                      const LocalInfo& pointer,
                                                      const ZonePointerLocalAdapter& locals);
bool mark_zone_reset_call(const IrExpr& call, const ZonePointerLocalAdapter& locals);
bool zone_metadata_extern_builtin_allows_zone_pointer_argument(const std::string& function_name,
                                                               std::size_t arg_index);
bool memory_extern_builtin_allows_zone_pointer_argument(const std::string& function_name,
                                                        std::size_t arg_index);
bool temporary_zone_source_from_expr(const IrExpr& value,
                                     const ZonePointerSourceResolver& resolver,
                                     const ZonePointerLocalAdapter& locals,
                                     const LocalScopeStack& scopes,
                                     std::string& source_name,
                                     std::size_t& source_scope_index);
std::optional<std::string> temporary_zone_pointer_escape_error(const IrExpr& value,
                                                               std::size_t first_scope_index,
                                                               const std::string& context,
                                                               const ZonePointerSourceResolver& resolver,
                                                               const ZonePointerLocalAdapter& locals,
                                                               const LocalScopeStack& scopes);
std::optional<std::string> zone_pointer_escape_error(const IrExpr& value,
                                                     const std::string& context,
                                                     const ZonePointerSourceResolver& resolver,
                                                     const ZonePointerLocalAdapter& locals,
                                                     const LocalScopeStack& scopes);
std::optional<std::string> outer_temporary_zone_pointer_escape_error(std::size_t first_scope_index,
                                                                     const LocalScopeStack& scopes);
IrStmtPtr make_zone_destroy_stmt(SourceLocation loc, const std::string& name, const IrType& type);
bool has_auto_destroy_zone_cleanup(const LocalScopeStack& scopes, std::size_t first_scope_index);
std::optional<std::string> append_auto_destroy_zone_cleanup(SourceLocation loc,
                                                            std::vector<IrStmtPtr>& statements,
                                                            LocalScopeStack& scopes,
                                                            std::size_t first_scope_index);
std::optional<std::string> require_no_temporary_zone_pointer_escape(const IrExpr& value,
                                                                    std::size_t first_scope_index,
                                                                    const std::string& context,
                                                                    const AutoDestroyZoneCleanupContext& cleanup);
bool has_auto_destroy_zone_cleanup(const AutoDestroyZoneCleanupContext& cleanup,
                                   std::size_t first_scope_index);
std::optional<std::string> append_auto_destroy_zone_cleanup(SourceLocation loc,
                                                            std::vector<IrStmtPtr>& statements,
                                                            AutoDestroyZoneCleanupContext& cleanup,
                                                            std::size_t first_scope_index);
AutoDestroyZoneMaterialization materialize_value_before_auto_destroy_cleanup(
    SourceLocation loc,
    IrExprPtr value,
    std::vector<IrStmtPtr>& statements,
    AutoDestroyZoneCleanupContext& cleanup,
    std::size_t first_scope_index,
    const std::string& hidden_prefix,
    const std::string& escape_context);
std::optional<std::string> materialize_values_before_auto_destroy_cleanup(
    SourceLocation loc,
    std::vector<IrExprPtr>& values,
    std::vector<IrStmtPtr>& statements,
    AutoDestroyZoneCleanupContext& cleanup,
    std::size_t first_scope_index,
    const std::string& hidden_prefix,
    const std::string& escape_context);

} // namespace ari
