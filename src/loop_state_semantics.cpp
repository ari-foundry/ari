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
    return (left == LocalState::Moved || left == LocalState::Dropped) &&
           (right == LocalState::Moved || right == LocalState::Dropped);
}

bool can_widen_owner_state(LocalState left, LocalState right) {
    return compatible_unavailable_owner_states(left, right) ||
           (left == LocalState::Alive && right != LocalState::Alive) ||
           (left != LocalState::Alive && right == LocalState::Alive);
}

LocalState merged_unavailable_owner_state(LocalState left, LocalState right) {
    return left == LocalState::Dropped && right == LocalState::Dropped
        ? LocalState::Dropped
        : LocalState::Moved;
}

LocalState widened_owner_state(LocalState left, LocalState right) {
    if (left == right) return left;
    if (left == LocalState::MaybeUnavailable || right == LocalState::MaybeUnavailable) {
        return LocalState::MaybeUnavailable;
    }
    if (left == LocalState::Alive) return right;
    if (right == LocalState::Alive) return left;
    return merged_unavailable_owner_state(left, right);
}

LocalState merged_loop_exit_owner_state(LocalState left, LocalState right) {
    if (left == right) return left;
    if (compatible_unavailable_owner_states(left, right)) {
        return merged_unavailable_owner_state(left, right);
    }
    return LocalState::MaybeUnavailable;
}

std::optional<std::string> merge_loop_exit_state_snapshot(
    StateSnapshot& merged,
    const StateSnapshot& snapshot,
    const std::string&
) {
    StateSnapshotEntry default_entry;
    for (auto& item : merged) {
        const StateSnapshotEntry& actual = snapshot_entry_or_default(snapshot, item.first, default_entry);
        if (item.second.state != actual.state) {
            item.second.state = merged_loop_exit_owner_state(item.second.state, actual.state);
        }
        if (state_snapshot_key_is_field(item.first)) continue;
        if (!merge_state_snapshot_entry_borrow_state_conservatively(item.second, actual)) {
            return "binding '" + item.first + "' has incompatible borrow states";
        }
    }
    merge_existing_zone_generations_into(merged, snapshot);
    return std::nullopt;
}

std::optional<std::string> merge_loop_state_snapshot_conservatively(
    StateSnapshot& merged,
    const StateSnapshot& snapshot,
    const std::string& message
) {
    StateSnapshotEntry default_entry;
    for (auto& item : merged) {
        const StateSnapshotEntry& actual = snapshot_entry_or_default(snapshot, item.first, default_entry);
        if (item.second.state != actual.state) {
            return "binding '" + item.first + "' " + message;
        }
        if (state_snapshot_key_is_field(item.first)) continue;
        if (!merge_state_snapshot_entry_borrow_state_conservatively(item.second, actual)) {
            return "binding '" + item.first + "' has incompatible borrow states";
        }
    }
    merge_existing_zone_generations_into(merged, snapshot);
    return std::nullopt;
}

std::optional<std::string> merge_loop_state_snapshot_with_owner_widening(
    StateSnapshot& merged,
    const StateSnapshot& snapshot,
    const std::string& message,
    bool& widened_owner_state_seen
) {
    StateSnapshotEntry default_entry;
    for (auto& item : merged) {
        const StateSnapshotEntry& actual = snapshot_entry_or_default(snapshot, item.first, default_entry);
        if (item.second.state != actual.state) {
            if (!can_widen_owner_state(item.second.state, actual.state)) {
                return "binding '" + item.first + "' " + message;
            }
            if (item.second.state == LocalState::Alive || actual.state == LocalState::Alive) {
                widened_owner_state_seen = true;
            }
            item.second.state = widened_owner_state(item.second.state, actual.state);
        }
        if (state_snapshot_key_is_field(item.first)) continue;
        if (!merge_state_snapshot_entry_borrow_state_conservatively(item.second, actual)) {
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

std::optional<std::string> merge_loop_state_snapshots_conservatively(
    StateSnapshot& merged,
    const std::vector<StateSnapshot>& snapshots,
    const std::string& message
) {
    for (const auto& snapshot : snapshots) {
        if (auto error = merge_loop_state_snapshot_conservatively(merged, snapshot, message)) {
            return error;
        }
    }
    return std::nullopt;
}

std::optional<std::string> merge_loop_state_snapshots_with_owner_widening(
    StateSnapshot& merged,
    const std::vector<StateSnapshot>& snapshots,
    const std::string& message,
    bool& widened_owner_state
) {
    for (const auto& snapshot : snapshots) {
        if (auto error = merge_loop_state_snapshot_with_owner_widening(
                merged,
                snapshot,
                message,
                widened_owner_state)) {
            return error;
        }
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
