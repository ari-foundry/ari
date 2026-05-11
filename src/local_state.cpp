#include "local_state.hpp"

#include "common.hpp"

#include <algorithm>
#include <utility>

namespace ari {

static bool state_snapshot_key_is_field(const std::string& key) {
    return key.find("#field:") != std::string::npos;
}

static std::string field_state_key(const std::string& name, const std::string& path) {
    return name + "#field:" + path;
}

static bool split_field_state_key(const std::string& key, std::string& name, std::string& path) {
    std::size_t split = key.find("#field:");
    if (split == std::string::npos) return false;
    name = key.substr(0, split);
    path = key.substr(split + 7);
    return true;
}

std::string local_state_name(LocalState state) {
    switch (state) {
        case LocalState::Alive: return "live";
        case LocalState::Moved: return "moved";
        case LocalState::Dropped: return "dropped";
    }
    return "unavailable";
}

LocalInfo make_local_info(SourceLocation loc, const IrType& type, bool mutable_binding) {
    LocalInfo local;
    local.type = type;
    local.mutable_binding = mutable_binding;
    local.state = LocalState::Alive;
    local.loc = loc;
    return local;
}

VectorKnownLength local_vector_known_length(const LocalInfo& local) {
    if (!is_vector_storage_type(local.type)) return {};
    return VectorKnownLength{local.vector_length_known, local.vector_known_length};
}

void set_local_vector_known_length(LocalInfo& local, VectorKnownLength state) {
    if (!is_vector_storage_type(local.type)) return;
    local.vector_length_known = state.known;
    local.vector_known_length = state.known ? state.length : 0;
}

void clear_local_integer_known_value(LocalInfo& local) {
    local.integer_value_known = false;
    local.integer_known_value = 0;
    local.integer_known_negative = false;
}

void set_local_integer_known_value(LocalInfo& local, std::uint64_t value, bool negative) {
    local.integer_value_known = true;
    local.integer_known_value = value;
    local.integer_known_negative = negative;
}

void mark_local_alive(LocalInfo& local) {
    local.state = LocalState::Alive;
}

void mark_local_moved(LocalInfo& local) {
    local.state = LocalState::Moved;
}

void mark_local_dropped(LocalInfo& local) {
    local.state = LocalState::Dropped;
}

void bump_local_zone_generation(LocalInfo& local) {
    ++local.zone_generation;
}

void mark_local_zone_destroyed(LocalInfo& local) {
    mark_local_moved(local);
    bump_local_zone_generation(local);
}

std::string local_owned_field_path(const std::string& base, std::size_t index) {
    return base.empty() ? std::to_string(index) : base + "." + std::to_string(index);
}

bool local_owned_field_path_matches(const std::string& candidate, const std::string& selected) {
    return candidate == selected ||
           (candidate.size() > selected.size() &&
            candidate.compare(0, selected.size(), selected) == 0 &&
            candidate[selected.size()] == '.');
}

bool local_owned_field_is_live(const LocalInfo& local, const std::string& path) {
    for (const auto& item : local.owned_field_states) {
        if (local_owned_field_path_matches(item.first, path) && item.second == LocalState::Alive) return true;
    }
    return false;
}

bool local_owned_field_has_state(const LocalInfo& local, const std::string& path) {
    for (const auto& item : local.owned_field_states) {
        if (local_owned_field_path_matches(item.first, path)) return true;
    }
    return false;
}

void mark_local_owned_field_state(LocalInfo& local, const std::string& path, LocalState state) {
    for (auto& item : local.owned_field_states) {
        if (local_owned_field_path_matches(item.first, path)) item.second = state;
    }
}

void mark_all_local_owned_fields(LocalInfo& local, LocalState state) {
    for (auto& item : local.owned_field_states) item.second = state;
}

bool local_has_moved_or_dropped_owned_fields(const LocalInfo& local) {
    for (const auto& item : local.owned_field_states) {
        if (item.second != LocalState::Alive) return true;
    }
    return false;
}

LocalState snapshot_state(const StateSnapshot& snapshot, const std::string& name) {
    auto found = snapshot.find(name);
    if (found == snapshot.end()) return LocalState::Alive;
    return found->second.state;
}

void merge_zone_generations_into(StateSnapshot& target, const StateSnapshot& source) {
    for (const auto& item : source) {
        if (state_snapshot_key_is_field(item.first)) continue;
        auto found = target.find(item.first);
        if (found == target.end()) {
            target.emplace(item.first, item.second);
            continue;
        }
        found->second.zone_generation = std::max(found->second.zone_generation, item.second.zone_generation);
        if (found->second.vector_length_known &&
            item.second.vector_length_known &&
            found->second.vector_known_length == item.second.vector_known_length) {
            continue;
        }
        found->second.vector_length_known = false;
        found->second.vector_known_length = 0;
    }
}

void merge_existing_zone_generations_into(StateSnapshot& target, const StateSnapshot& source) {
    for (auto& item : target) {
        if (state_snapshot_key_is_field(item.first)) continue;
        auto found = source.find(item.first);
        if (found == source.end()) continue;
        item.second.zone_generation = std::max(item.second.zone_generation, found->second.zone_generation);
        if (item.second.vector_length_known &&
            found->second.vector_length_known &&
            item.second.vector_known_length == found->second.vector_known_length) {
            continue;
        }
        item.second.vector_length_known = false;
        item.second.vector_known_length = 0;
    }
}

StateSnapshot merge_zone_generations(StateSnapshot target, const StateSnapshot& source) {
    merge_zone_generations_into(target, source);
    return target;
}

void LocalScopeStack::clear() {
    scopes_.clear();
    used_names_.clear();
    reusable_pattern_binding_names_.clear();
}

void LocalScopeStack::push_scope() {
    scopes_.push_back({});
}

void LocalScopeStack::pop_scope() {
    scopes_.pop_back();
}

void LocalScopeStack::end_scope(bool check_owners,
                                const LocalReleaseCallback& release_local,
                                const LocalOwnerCheckCallback& has_live_owner,
                                const LocalOwnerErrorCallback& report_live_owner) {
    for (const auto& item : current_scope()) {
        release_local(item.second);
    }
    if (check_owners) {
        for (const auto& item : current_scope()) {
            const LocalInfo& local = item.second;
            if (has_live_owner(local)) report_live_owner(item.first, local);
        }
    }
    pop_scope();
}

bool LocalScopeStack::empty() const {
    return scopes_.empty();
}

std::size_t LocalScopeStack::size() const {
    return scopes_.size();
}

bool LocalScopeStack::contains_scope(std::size_t index) const {
    return index < scopes_.size();
}

bool LocalScopeStack::any_local_from(std::size_t first_scope_index,
                                     const LocalConstPredicate& predicate) const {
    if (!contains_scope(first_scope_index)) return false;
    for (std::size_t scope_index = first_scope_index; scope_index < scopes_.size(); ++scope_index) {
        for (const auto& item : scope_at(scope_index)) {
            if (predicate(item.first, item.second)) return true;
        }
    }
    return false;
}

void LocalScopeStack::for_each_local_from(std::size_t first_scope_index,
                                          const LocalConstVisitor& visitor) const {
    if (!contains_scope(first_scope_index)) return;
    for (std::size_t scope_index = first_scope_index; scope_index < scopes_.size(); ++scope_index) {
        for (const auto& item : scope_at(scope_index)) {
            visitor(item.first, item.second);
        }
    }
}

void LocalScopeStack::for_each_local_before(std::size_t end_scope_index,
                                            const LocalConstVisitor& visitor) const {
    std::size_t capped_end = std::min(end_scope_index, scopes_.size());
    for (std::size_t scope_index = 0; scope_index < capped_end; ++scope_index) {
        for (const auto& item : scope_at(scope_index)) {
            visitor(item.first, item.second);
        }
    }
}

void LocalScopeStack::for_each_local_from_inner_to_outer(std::size_t first_scope_index,
                                                        const LocalMutableVisitor& visitor) {
    if (!contains_scope(first_scope_index)) return;
    for (std::size_t offset = scopes_.size(); offset > first_scope_index; --offset) {
        for (auto& item : scope_at(offset - 1)) {
            visitor(item.first, item.second);
        }
    }
}

LocalScopeStack::Scope& LocalScopeStack::current_scope() {
    return scopes_.back();
}

const LocalScopeStack::Scope& LocalScopeStack::current_scope() const {
    return scopes_.back();
}

LocalScopeStack::Scope& LocalScopeStack::scope_at(std::size_t index) {
    return scopes_[index];
}

const LocalScopeStack::Scope& LocalScopeStack::scope_at(std::size_t index) const {
    return scopes_[index];
}

LocalInfo* LocalScopeStack::find(const std::string& name) {
    for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope) {
        auto found = scope->find(name);
        if (found != scope->end()) return &found->second;
    }
    return nullptr;
}

const LocalInfo* LocalScopeStack::find(const std::string& name) const {
    for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope) {
        auto found = scope->find(name);
        if (found != scope->end()) return &found->second;
    }
    return nullptr;
}

LocalInfo& LocalScopeStack::require_for_restore(const std::string& name) {
    if (LocalInfo* local = find(name)) return *local;
    throw CompileError("internal error: missing local '" + name + "' while restoring state");
}

bool LocalScopeStack::scope_index(const std::string& name, std::size_t& out) const {
    for (std::size_t index = scopes_.size(); index > 0; --index) {
        if (scopes_[index - 1].find(name) != scopes_[index - 1].end()) {
            out = index - 1;
            return true;
        }
    }
    return false;
}

StateSnapshot LocalScopeStack::snapshot_states() const {
    StateSnapshot snapshot;
    for (const auto& scope : scopes_) {
        for (const auto& item : scope) {
            snapshot[item.first] = StateSnapshotEntry{
                item.second.state,
                item.second.zone_generation,
                item.second.vector_length_known,
                item.second.vector_known_length
            };
            for (const auto& field : item.second.owned_field_states) {
                snapshot[field_state_key(item.first, field.first)] = StateSnapshotEntry{field.second, 0};
            }
        }
    }
    return snapshot;
}

void LocalScopeStack::restore_states(const StateSnapshot& snapshot) {
    for (const auto& item : snapshot) {
        std::string local_name;
        std::string field_path;
        if (split_field_state_key(item.first, local_name, field_path)) {
            require_for_restore(local_name).owned_field_states[field_path] = item.second.state;
        } else {
            LocalInfo& local = require_for_restore(item.first);
            local.state = item.second.state;
            local.zone_generation = item.second.zone_generation;
            local.vector_length_known = item.second.vector_length_known;
            local.vector_known_length = item.second.vector_known_length;
        }
    }
}

bool LocalScopeStack::name_was_used(const std::string& name) const {
    return used_names_.count(name) != 0;
}

bool LocalScopeStack::reusable_pattern_binding(const std::string& name) const {
    return reusable_pattern_binding_names_.count(name) != 0;
}

void LocalScopeStack::mark_name_used(const std::string& name) {
    used_names_.insert(name);
}

void LocalScopeStack::set_reusable_pattern_bindings(std::set<std::string> names) {
    reusable_pattern_binding_names_ = std::move(names);
}

void LocalScopeStack::clear_reusable_pattern_bindings() {
    reusable_pattern_binding_names_.clear();
}

void LocalScopeStack::declare_current(std::string name, LocalInfo local) {
    current_scope()[std::move(name)] = std::move(local);
}

} // namespace ari
