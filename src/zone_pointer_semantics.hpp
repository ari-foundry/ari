#pragma once

#include "ir.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>

namespace ari {

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

} // namespace ari
