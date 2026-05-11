#pragma once

#include "ir.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>

namespace ari {

struct ZonePointerSourceResolver {
    using LocalZonePointerSourceLookup = std::function<bool(const std::string&, std::string&)>;
    using ZoneArgSourceLookup = std::function<bool(const IrExpr&, std::string&)>;
    using CallZoneReturnParamLookup = std::function<std::optional<std::size_t>(const std::string&)>;

    LocalZonePointerSourceLookup local_zone_pointer_source;
    ZoneArgSourceLookup zone_source_from_arg;
    CallZoneReturnParamLookup call_zone_return_param_index;
};

const IrExpr& zone_pointer_source_expr(const IrExpr& value);
std::string zone_pointer_escape_name(const IrExpr& value);
bool is_zone_pointer_trackable_type(const IrType& type);
bool zone_pointer_source_name_from_expr(const IrExpr& value,
                                        const ZonePointerSourceResolver& resolver,
                                        std::string& out);

} // namespace ari
