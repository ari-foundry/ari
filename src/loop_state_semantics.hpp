#pragma once

#include "local_state.hpp"

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace ari {

std::optional<std::string> loop_state_mismatch_error_ignoring_bindings(
    const StateSnapshot& expected,
    const StateSnapshot& actual,
    const std::set<std::string>& ignored_bindings,
    const std::string& message
);

std::optional<std::string> merge_loop_state_snapshots(
    StateSnapshot& merged,
    const std::vector<StateSnapshot>& snapshots,
    const std::string& message
);

std::optional<std::string> merge_loop_state_snapshots_conservatively(
    StateSnapshot& merged,
    const std::vector<StateSnapshot>& snapshots,
    const std::string& message
);

StateSnapshot project_loop_state_snapshot(
    const StateSnapshot& base,
    const StateSnapshot& snapshot
);

std::optional<std::string> merge_loop_exit_states(
    StateSnapshot& merged,
    const std::vector<StateSnapshot>& break_states,
    const std::string& message
);

} // namespace ari
