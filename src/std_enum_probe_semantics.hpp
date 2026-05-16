#pragma once

#include <optional>
#include <string>

namespace ari {

struct StdEnumProbeHelper {
    std::string enum_name;
    std::string case_name;
};

std::optional<StdEnumProbeHelper> std_enum_probe_helper_for_name(const std::string& resolved_name);

} // namespace ari
