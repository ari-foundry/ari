#pragma once

#include "local_state.hpp"

#include <optional>
#include <set>
#include <string>

namespace ari {

std::optional<std::string> loop_state_mismatch_error_ignoring_bindings(
    const StateSnapshot& expected,
    const StateSnapshot& actual,
    const std::set<std::string>& ignored_bindings,
    const std::string& message
);

} // namespace ari
