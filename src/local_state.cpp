#include "local_state.hpp"

#include "common.hpp"
#include "type_semantics.hpp"

#include <algorithm>
#include <utility>

namespace ari {

bool state_snapshot_key_is_field(const std::string& key) {
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
        case LocalState::MaybeUnavailable: return "maybe-unavailable";
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

bool local_is_alive(const LocalInfo& local) {
    return local.state == LocalState::Alive;
}

std::optional<std::string> local_unavailable_binding_error(const std::string& name, const LocalInfo& local) {
    if (local_is_alive(local)) return std::nullopt;
    return "cannot use " + local_state_name(local.state) + " binding '" + name + "'";
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

bool local_owned_field_maybe_unavailable(const LocalInfo& local, const std::string& path) {
    for (const auto& item : local.owned_field_states) {
        if (local_owned_field_path_matches(item.first, path) && item.second == LocalState::MaybeUnavailable) return true;
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

bool local_has_tracked_owned_fields(const LocalInfo& local) {
    return local.owned_field_states_complete || !local.owned_field_states.empty();
}

bool local_has_live_owned_fields(const LocalInfo& local) {
    if (!local_is_alive(local)) return false;
    for (const auto& item : local.owned_field_states) {
        if (item.second == LocalState::Alive) return true;
    }
    return false;
}

bool local_has_live_owner(const LocalInfo& local) {
    if (local_has_tracked_owned_fields(local)) return local_has_live_owned_fields(local);
    return is_owner_type(local.type) && local_is_alive(local);
}

bool local_has_maybe_unavailable_owner(const LocalInfo& local) {
    if (local_has_tracked_owned_fields(local)) {
        for (const auto& item : local.owned_field_states) {
            if (item.second == LocalState::MaybeUnavailable) return true;
        }
        return false;
    }
    return is_owner_type(local.type) && local.state == LocalState::MaybeUnavailable;
}

std::string local_borrow_path_display(const std::string& name, const std::string& path) {
    return path.empty() ? name : name + "." + path;
}

static bool field_borrow_counts_active(const LocalInfo::FieldBorrowCounts& counts) {
    return counts.immutable > 0 || counts.mutable_ > 0;
}

static bool borrow_paths_overlap(const std::string& left, const std::string& right) {
    if (left.empty() || right.empty()) return true;
    return local_owned_field_path_matches(left, right) || local_owned_field_path_matches(right, left);
}

bool local_has_active_borrows(const LocalInfo& local) {
    return local.immutable_borrows > 0 || local.mutable_borrows > 0;
}

bool local_has_mutable_borrows(const LocalInfo& local) {
    return local.mutable_borrows > 0;
}

bool local_has_active_field_borrows(const LocalInfo& local) {
    for (const auto& item : local.field_borrows) {
        if (field_borrow_counts_active(item.second)) return true;
    }
    return false;
}

bool local_has_mutable_field_borrows(const LocalInfo& local) {
    for (const auto& item : local.field_borrows) {
        if (item.second.mutable_ > 0) return true;
    }
    return false;
}

bool local_has_overlapping_field_borrows(const LocalInfo& local, const std::string& path) {
    for (const auto& item : local.field_borrows) {
        if (field_borrow_counts_active(item.second) && borrow_paths_overlap(item.first, path)) return true;
    }
    return false;
}

bool local_has_overlapping_mutable_field_borrows(const LocalInfo& local, const std::string& path) {
    for (const auto& item : local.field_borrows) {
        if (item.second.mutable_ > 0 && borrow_paths_overlap(item.first, path)) return true;
    }
    return false;
}

void add_local_borrow_source(LocalInfo& source, const std::string& path, bool mutable_borrow) {
    if (path.empty()) {
        int& count = mutable_borrow ? source.mutable_borrows : source.immutable_borrows;
        ++count;
        return;
    }
    LocalInfo::FieldBorrowCounts& counts = source.field_borrows[path];
    int& count = mutable_borrow ? counts.mutable_ : counts.immutable;
    ++count;
}

void release_local_borrow_source(const std::string& name,
                                 LocalInfo& source,
                                 const std::string& path,
                                 bool mutable_borrow) {
    if (path.empty()) {
        int& count = mutable_borrow ? source.mutable_borrows : source.immutable_borrows;
        if (count <= 0) throw CompileError("internal error: named borrow count underflow for '" + name + "'");
        --count;
        return;
    }

    auto found = source.field_borrows.find(path);
    if (found == source.field_borrows.end()) {
        throw CompileError("internal error: field borrow count underflow for '" +
                           local_borrow_path_display(name, path) + "'");
    }
    int& count = mutable_borrow ? found->second.mutable_ : found->second.immutable;
    if (count <= 0) {
        throw CompileError("internal error: field borrow count underflow for '" +
                           local_borrow_path_display(name, path) + "'");
    }
    --count;
    if (!field_borrow_counts_active(found->second)) source.field_borrows.erase(found);
}

void set_local_named_borrow_source(LocalInfo& binding,
                                   const std::string& name,
                                   const std::string& path,
                                   bool mutable_borrow) {
    binding.borrow_source = name;
    binding.borrow_source_path = path;
    binding.borrow_source_mutable = mutable_borrow;
    binding.borrow_sources_released = false;
}

void add_local_aggregate_borrow_source(LocalInfo& binding,
                                       const std::string& aggregate_path,
                                       const std::string& name,
                                       const std::string& path,
                                       bool mutable_borrow,
                                       bool release_source) {
    binding.aggregate_borrow_sources.push_back({aggregate_path, name, path, mutable_borrow, release_source});
    binding.borrow_sources_released = false;
}

std::optional<std::string> local_assignment_target_error(const std::string& name, const LocalInfo& local) {
    if (is_borrow_type(local.type)) {
        return "cannot assign to borrow binding '" + name + "'";
    }
    if (!local.mutable_binding) {
        return "cannot assign to immutable binding '" + name + "'";
    }
    return std::nullopt;
}

std::optional<std::string> local_assignment_storage_error(const std::string& name, const LocalInfo& local) {
    if (is_owner_type(local.type) && local_has_maybe_unavailable_owner(local)) {
        return "cannot overwrite owning binding '" + name + "' while it may be unavailable";
    }
    if (is_owner_type(local.type) && local_has_live_owner(local)) {
        return "cannot overwrite owning binding '" + name + "' before it is moved or dropped";
    }
    return std::nullopt;
}

std::optional<std::string> local_field_assignment_base_error(const std::string& name, const LocalInfo& local) {
    if (!local.mutable_binding) {
        return "cannot assign to field of immutable binding '" + name + "'";
    }
    return std::nullopt;
}

std::optional<std::string> local_aggregate_assignment_base_error(const std::string& name,
                                                                 const LocalInfo& local,
                                                                 const IrType& base_type) {
    if (!local.mutable_binding && base_type.qualifier != TypeQualifier::MutRef) {
        return "cannot assign to field of immutable binding '" + name + "'";
    }
    return std::nullopt;
}

std::optional<std::string> local_mutable_borrow_error(const std::string& name, const LocalInfo& local) {
    if (!local.mutable_binding) {
        return "cannot mutably borrow immutable binding '" + name + "'";
    }
    return std::nullopt;
}

std::optional<std::string> local_method_mutability_error(const std::string& name,
                                                         const LocalInfo& local,
                                                         const std::string& method_display) {
    if (!local.mutable_binding) {
        return "cannot call " + method_display + " on immutable binding '" + name + "'";
    }
    return std::nullopt;
}

static LocalState snapshot_state(const StateSnapshot& snapshot, const std::string& name) {
    auto found = snapshot.find(name);
    if (found == snapshot.end()) return LocalState::Alive;
    return found->second.state;
}

static bool compatible_unavailable_owner_states(LocalState left, LocalState right) {
    return (left == LocalState::Moved || left == LocalState::Dropped) &&
           (right == LocalState::Moved || right == LocalState::Dropped);
}

static bool both_states_unavailable(LocalState left, LocalState right) {
    return left != LocalState::Alive && right != LocalState::Alive;
}

static bool field_state_irrelevant_after_base_unavailable(const StateSnapshot& left,
                                                          const StateSnapshot& right,
                                                          const std::string& field_key) {
    std::string base_name;
    std::string field_path;
    if (!split_field_state_key(field_key, base_name, field_path)) return false;
    return both_states_unavailable(snapshot_state(left, base_name),
                                   snapshot_state(right, base_name));
}

static bool borrow_source_equal(const LocalInfo::BorrowSource& left,
                                const LocalInfo::BorrowSource& right) {
    return left.aggregate_path == right.aggregate_path &&
           left.name == right.name &&
           left.path == right.path &&
           left.mutable_borrow == right.mutable_borrow &&
           left.release_source == right.release_source;
}

static bool borrow_sources_equal(const std::vector<LocalInfo::BorrowSource>& left,
                                 const std::vector<LocalInfo::BorrowSource>& right) {
    if (left.size() != right.size()) return false;
    for (std::size_t i = 0; i < left.size(); ++i) {
        if (!borrow_source_equal(left[i], right[i])) return false;
    }
    return true;
}

static bool field_borrow_counts_equal(
    const std::map<std::string, LocalInfo::FieldBorrowCounts>& left,
    const std::map<std::string, LocalInfo::FieldBorrowCounts>& right
) {
    if (left.size() != right.size()) return false;
    auto left_it = left.begin();
    auto right_it = right.begin();
    for (; left_it != left.end(); ++left_it, ++right_it) {
        if (left_it->first != right_it->first) return false;
        if (left_it->second.immutable != right_it->second.immutable ||
            left_it->second.mutable_ != right_it->second.mutable_) {
            return false;
        }
    }
    return true;
}

static void merge_field_borrow_counts_conservatively(
    std::map<std::string, LocalInfo::FieldBorrowCounts>& target,
    const std::map<std::string, LocalInfo::FieldBorrowCounts>& source
) {
    for (const auto& item : source) {
        LocalInfo::FieldBorrowCounts& counts = target[item.first];
        counts.immutable = std::max(counts.immutable, item.second.immutable);
        counts.mutable_ = std::max(counts.mutable_, item.second.mutable_);
    }
}

bool state_snapshot_entry_borrow_state_equal(const StateSnapshotEntry& left,
                                             const StateSnapshotEntry& right) {
    return left.immutable_borrows == right.immutable_borrows &&
           left.mutable_borrows == right.mutable_borrows &&
           field_borrow_counts_equal(left.field_borrows, right.field_borrows) &&
           left.borrow_source == right.borrow_source &&
           left.borrow_source_path == right.borrow_source_path &&
           left.borrow_source_mutable == right.borrow_source_mutable &&
           left.borrow_sources_released == right.borrow_sources_released &&
           borrow_sources_equal(left.aggregate_borrow_sources, right.aggregate_borrow_sources);
}

bool merge_state_snapshot_entry_borrow_state_conservatively(StateSnapshotEntry& target,
                                                            const StateSnapshotEntry& source) {
    if (state_snapshot_entry_borrow_state_equal(target, source)) return true;
    if (target.borrow_source != source.borrow_source ||
        target.borrow_source_path != source.borrow_source_path ||
        target.borrow_source_mutable != source.borrow_source_mutable ||
        !borrow_sources_equal(target.aggregate_borrow_sources, source.aggregate_borrow_sources)) {
        return false;
    }
    target.immutable_borrows = std::max(target.immutable_borrows, source.immutable_borrows);
    target.mutable_borrows = std::max(target.mutable_borrows, source.mutable_borrows);
    merge_field_borrow_counts_conservatively(target.field_borrows, source.field_borrows);
    target.borrow_sources_released = target.borrow_sources_released && source.borrow_sources_released;
    return true;
}

std::optional<std::string> state_snapshot_mismatch_error(const StateSnapshot& left,
                                                         const StateSnapshot& right,
                                                         const std::string& message) {
    for (const auto& item : left) {
        LocalState right_state = snapshot_state(right, item.first);
        if (item.second.state != right_state &&
            !compatible_unavailable_owner_states(item.second.state, right_state) &&
            !field_state_irrelevant_after_base_unavailable(left, right, item.first)) {
            return "binding '" + item.first + "' " + message;
        }
        if (state_snapshot_key_is_field(item.first)) continue;
        auto found = right.find(item.first);
        StateSnapshotEntry default_entry;
        const StateSnapshotEntry& actual = found == right.end() ? default_entry : found->second;
        if (item.second.owned_field_states_complete != actual.owned_field_states_complete &&
            !both_states_unavailable(item.second.state, actual.state)) {
            return "binding '" + item.first + "' " + message;
        }
        if (!state_snapshot_entry_borrow_state_equal(item.second, actual)) {
            return "binding '" + item.first + "' has incompatible borrow states";
        }
    }
    return std::nullopt;
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
                item.second.vector_known_length,
                item.second.immutable_borrows,
                item.second.mutable_borrows,
                item.second.field_borrows,
                item.second.borrow_source,
                item.second.borrow_source_path,
                item.second.borrow_source_mutable,
                item.second.borrow_sources_released,
                item.second.auto_drop_owner,
                item.second.owned_field_states_complete,
                item.second.aggregate_borrow_sources
            };
            for (const auto& field : item.second.owned_field_states) {
                StateSnapshotEntry field_entry;
                field_entry.state = field.second;
                snapshot[field_state_key(item.first, field.first)] = std::move(field_entry);
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
            local.owned_field_states.clear();
            local.zone_generation = item.second.zone_generation;
            local.vector_length_known = item.second.vector_length_known;
            local.vector_known_length = item.second.vector_known_length;
            local.immutable_borrows = item.second.immutable_borrows;
            local.mutable_borrows = item.second.mutable_borrows;
            local.field_borrows = item.second.field_borrows;
            local.borrow_source = item.second.borrow_source;
            local.borrow_source_path = item.second.borrow_source_path;
            local.borrow_source_mutable = item.second.borrow_source_mutable;
            local.borrow_sources_released = item.second.borrow_sources_released;
            local.auto_drop_owner = item.second.auto_drop_owner;
            local.owned_field_states_complete = item.second.owned_field_states_complete;
            local.aggregate_borrow_sources = item.second.aggregate_borrow_sources;
        }
    }
}

void LocalScopeStack::restore_merged_zone_generations(StateSnapshot target, const StateSnapshot& source) {
    restore_states(merge_zone_generations(std::move(target), source));
}

void LocalScopeStack::release_borrow_source(const std::string& name,
                                            const std::string& path,
                                            bool mutable_borrow) {
    release_local_borrow_source(name, require_for_restore(name), path, mutable_borrow);
}

void LocalScopeStack::release_borrow_sources(const LocalInfo& borrow) {
    if (!borrow.borrow_source.empty()) {
        release_borrow_source(borrow.borrow_source, borrow.borrow_source_path, borrow.borrow_source_mutable);
    }
    for (const auto& item : borrow.aggregate_borrow_sources) {
        if (!item.release_source) continue;
        release_borrow_source(item.name, item.path, item.mutable_borrow);
    }
}

void LocalScopeStack::release_aggregate_borrow_sources_at(LocalInfo& binding,
                                                          const std::string& aggregate_path) {
    auto& sources = binding.aggregate_borrow_sources;
    for (auto it = sources.begin(); it != sources.end();) {
        const bool release =
            aggregate_path.empty() ||
            local_owned_field_path_matches(it->aggregate_path, aggregate_path) ||
            local_owned_field_path_matches(aggregate_path, it->aggregate_path);
        if (!release) {
            ++it;
            continue;
        }
        if (it->release_source) release_borrow_source(it->name, it->path, it->mutable_borrow);
        it = sources.erase(it);
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

LocalScopeStack::NameState LocalScopeStack::snapshot_name_state() const {
    return NameState{used_names_, reusable_pattern_binding_names_};
}

void LocalScopeStack::restore_name_state(NameState state) {
    used_names_ = std::move(state.used_names);
    reusable_pattern_binding_names_ = std::move(state.reusable_pattern_binding_names);
}

void LocalScopeStack::declare_current(std::string name, LocalInfo local) {
    current_scope()[std::move(name)] = std::move(local);
}

} // namespace ari
