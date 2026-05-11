#include "loop_state_semantics.hpp"

namespace ari {

namespace {

std::string snapshot_key_binding_name(const std::string& key) {
    std::size_t field_split = key.find("#field:");
    if (field_split == std::string::npos) return key;
    return key.substr(0, field_split);
}

LocalState snapshot_state_or_alive(const StateSnapshot& snapshot, const std::string& key) {
    auto found = snapshot.find(key);
    if (found == snapshot.end()) return LocalState::Alive;
    return found->second.state;
}

} // namespace

std::optional<std::string> loop_state_mismatch_error_ignoring_bindings(
    const StateSnapshot& expected,
    const StateSnapshot& actual,
    const std::set<std::string>& ignored_bindings,
    const std::string& message
) {
    for (const auto& item : expected) {
        if (ignored_bindings.count(snapshot_key_binding_name(item.first)) != 0) continue;
        if (item.second.state != snapshot_state_or_alive(actual, item.first)) {
            return "binding '" + item.first + "' " + message;
        }
    }
    return std::nullopt;
}

std::optional<std::string> merge_loop_exit_states(
    StateSnapshot& merged,
    const std::vector<StateSnapshot>& break_states,
    const std::string& message
) {
    return merge_loop_state_snapshots(merged, break_states, message);
}

std::optional<std::string> merge_loop_state_snapshots(
    StateSnapshot& merged,
    const std::vector<StateSnapshot>& snapshots,
    const std::string& message
) {
    for (const auto& snapshot : snapshots) {
        if (auto error = state_snapshot_mismatch_error(merged, snapshot, message)) {
            return error;
        }
        merge_existing_zone_generations_into(merged, snapshot);
    }
    return std::nullopt;
}

} // namespace ari
