#include "std_enum_probe_semantics.hpp"

namespace ari {

std::optional<StdEnumProbeHelper> std_enum_probe_helper_for_name(const std::string& resolved_name) {
    if (resolved_name == "std::option::__is_some") {
        return StdEnumProbeHelper{"std::Option", "std::Some"};
    }
    if (resolved_name == "std::option::__is_none") {
        return StdEnumProbeHelper{"std::Option", "std::None"};
    }
    if (resolved_name == "std::result::__is_ok") {
        return StdEnumProbeHelper{"std::Result", "std::Ok"};
    }
    if (resolved_name == "std::result::__is_err") {
        return StdEnumProbeHelper{"std::Result", "std::Err"};
    }
    return std::nullopt;
}

} // namespace ari
