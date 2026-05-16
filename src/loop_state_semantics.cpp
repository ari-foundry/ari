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

const StateSnapshotEntry& snapshot_entry_or_default(
    const StateSnapshot& snapshot,
    const std::string& key,
    const StateSnapshotEntry& default_entry
) {
    auto found = snapshot.find(key);
    return found == snapshot.end() ? default_entry : found->second;
}

bool compatible_unavailable_owner_states(LocalState left, LocalState right) {
    return left != LocalState::Alive && right != LocalState::Alive;
}

LocalState merged_unavailable_owner_state(LocalState left, LocalState right) {
    return left == LocalState::Dropped && right == LocalState::Dropped
        ? LocalState::Dropped
        : LocalState::Moved;
}

std::optional<std::string> merge_loop_exit_state_snapshot(
    StateSnapshot& merged,
    const StateSnapshot& snapshot,
    const std::string& message
) {
    StateSnapshotEntry default_entry;
    for (auto& item : merged) {
        const StateSnapshotEntry& actual = snapshot_entry_or_default(snapshot, item.first, default_entry);
        if (item.second.state != actual.state) {
            if (!compatible_unavailable_owner_states(item.second.state, actual.state)) {
                return "binding '" + item.first + "' " + message;
            }
            item.second.state = merged_unavailable_owner_state(item.second.state, actual.state);
        }
        if (item.first.find("#field:") != std::string::npos) continue;
        if (!state_snapshot_entry_borrow_state_equal(item.second, actual)) {
            return "binding '" + item.first + "' has incompatible borrow states";
        }
    }
    merge_existing_zone_generations_into(merged, snapshot);
    return std::nullopt;
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
    for (const auto& snapshot : break_states) {
        if (auto error = merge_loop_exit_state_snapshot(merged, snapshot, message)) {
            return error;
        }
    }
    return std::nullopt;
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

StateSnapshot project_loop_state_snapshot(
    const StateSnapshot& base,
    const StateSnapshot& snapshot
) {
    StateSnapshot projected = base;
    for (auto& item : projected) {
        auto found = snapshot.find(item.first);
        if (found != snapshot.end()) item.second = found->second;
    }
    return projected;
}

} // namespace ari
