#pragma once

#include "ir.hpp"
#include "vector_semantics.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace ari {

enum class LocalState {
    Alive,
    Moved,
    Dropped
};

std::string local_state_name(LocalState state);

struct LocalInfo {
    IrType type;
    IrType* ir_storage_type = nullptr;
    IrExpr* ir_init_expr = nullptr;
    bool mutable_binding = false;
    bool function_parameter = false;
    bool auto_destroy_zone = false;
    bool vector_length_known = false;
    std::uint64_t vector_known_length = 0;
    bool integer_value_known = false;
    std::uint64_t integer_known_value = 0;
    bool integer_known_negative = false;
    LocalState state = LocalState::Alive;
    std::map<std::string, LocalState> owned_field_states;
    int immutable_borrows = 0;
    int mutable_borrows = 0;
    struct FieldBorrowCounts {
        int immutable = 0;
        int mutable_ = 0;
    };
    std::map<std::string, FieldBorrowCounts> field_borrows;
    std::string borrow_source;
    std::string borrow_source_path;
    bool borrow_source_mutable = false;
    bool borrow_sources_released = false;
    bool zone_pointer = false;
    std::string zone_pointer_source;
    std::uint64_t zone_pointer_generation = 0;
    std::uint64_t zone_generation = 0;
    std::string generic_origin;
    struct BorrowSource {
        std::string aggregate_path;
        std::string name;
        std::string path;
        bool mutable_borrow = false;
        bool release_source = true;
    };
    std::vector<BorrowSource> aggregate_borrow_sources;
    SourceLocation loc;
};

struct StateSnapshotEntry {
    LocalState state = LocalState::Alive;
    std::uint64_t zone_generation = 0;
    bool vector_length_known = false;
    std::uint64_t vector_known_length = 0;
};

using StateSnapshot = std::map<std::string, StateSnapshotEntry>;

LocalInfo make_local_info(SourceLocation loc, const IrType& type, bool mutable_binding);
bool local_is_alive(const LocalInfo& local);
std::optional<std::string> local_unavailable_binding_error(const std::string& name, const LocalInfo& local);
VectorKnownLength local_vector_known_length(const LocalInfo& local);
void set_local_vector_known_length(LocalInfo& local, VectorKnownLength state);
void clear_local_integer_known_value(LocalInfo& local);
void set_local_integer_known_value(LocalInfo& local, std::uint64_t value, bool negative);
void mark_local_alive(LocalInfo& local);
void mark_local_moved(LocalInfo& local);
void mark_local_dropped(LocalInfo& local);
void bump_local_zone_generation(LocalInfo& local);
void mark_local_zone_destroyed(LocalInfo& local);
std::string local_owned_field_path(const std::string& base, std::size_t index);
bool local_owned_field_path_matches(const std::string& candidate, const std::string& selected);
bool local_owned_field_is_live(const LocalInfo& local, const std::string& path);
bool local_owned_field_has_state(const LocalInfo& local, const std::string& path);
void mark_local_owned_field_state(LocalInfo& local, const std::string& path, LocalState state);
void mark_all_local_owned_fields(LocalInfo& local, LocalState state);
bool local_has_moved_or_dropped_owned_fields(const LocalInfo& local);
bool local_has_tracked_owned_fields(const LocalInfo& local);
bool local_has_live_owned_fields(const LocalInfo& local);
bool local_has_live_owner(const LocalInfo& local);
std::string local_borrow_path_display(const std::string& name, const std::string& path);
bool local_has_active_borrows(const LocalInfo& local);
bool local_has_mutable_borrows(const LocalInfo& local);
bool local_has_active_field_borrows(const LocalInfo& local);
bool local_has_mutable_field_borrows(const LocalInfo& local);
bool local_has_overlapping_field_borrows(const LocalInfo& local, const std::string& path);
bool local_has_overlapping_mutable_field_borrows(const LocalInfo& local, const std::string& path);
void add_local_borrow_source(LocalInfo& source, const std::string& path, bool mutable_borrow);
void release_local_borrow_source(const std::string& name,
                                 LocalInfo& source,
                                 const std::string& path,
                                 bool mutable_borrow);
void set_local_named_borrow_source(LocalInfo& binding,
                                   const std::string& name,
                                   const std::string& path,
                                   bool mutable_borrow);
void add_local_aggregate_borrow_source(LocalInfo& binding,
                                       const std::string& aggregate_path,
                                       const std::string& name,
                                       const std::string& path,
                                       bool mutable_borrow,
                                       bool release_source = true);
std::optional<std::string> local_assignment_target_error(const std::string& name, const LocalInfo& local);
std::optional<std::string> local_assignment_storage_error(const std::string& name, const LocalInfo& local);
std::optional<std::string> local_field_assignment_base_error(const std::string& name, const LocalInfo& local);
std::optional<std::string> local_aggregate_assignment_base_error(const std::string& name,
                                                                 const LocalInfo& local,
                                                                 const IrType& base_type);
std::optional<std::string> local_mutable_borrow_error(const std::string& name, const LocalInfo& local);
std::optional<std::string> local_method_mutability_error(const std::string& name,
                                                         const LocalInfo& local,
                                                         const std::string& method_display);
std::optional<std::string> state_snapshot_mismatch_error(const StateSnapshot& left,
                                                         const StateSnapshot& right,
                                                         const std::string& message);
void merge_zone_generations_into(StateSnapshot& target, const StateSnapshot& source);
void merge_existing_zone_generations_into(StateSnapshot& target, const StateSnapshot& source);
StateSnapshot merge_zone_generations(StateSnapshot target, const StateSnapshot& source);

class LocalScopeStack {
public:
    using Scope = std::map<std::string, LocalInfo>;
    using LocalReleaseCallback = std::function<void(const LocalInfo&)>;
    using LocalOwnerCheckCallback = std::function<bool(const LocalInfo&)>;
    using LocalOwnerErrorCallback = std::function<void(const std::string&, const LocalInfo&)>;
    using LocalConstVisitor = std::function<void(const std::string&, const LocalInfo&)>;
    using LocalMutableVisitor = std::function<void(const std::string&, LocalInfo&)>;
    using LocalConstPredicate = std::function<bool(const std::string&, const LocalInfo&)>;

    void clear();
    void push_scope();
    void pop_scope();
    void end_scope(bool check_owners,
                   const LocalReleaseCallback& release_local,
                   const LocalOwnerCheckCallback& has_live_owner,
                   const LocalOwnerErrorCallback& report_live_owner);
    bool empty() const;
    std::size_t size() const;

    bool contains_scope(std::size_t index) const;
    bool any_local_from(std::size_t first_scope_index, const LocalConstPredicate& predicate) const;
    void for_each_local_from(std::size_t first_scope_index, const LocalConstVisitor& visitor) const;
    void for_each_local_before(std::size_t end_scope_index, const LocalConstVisitor& visitor) const;
    void for_each_local_from_inner_to_outer(std::size_t first_scope_index, const LocalMutableVisitor& visitor);

    LocalInfo* find(const std::string& name);
    const LocalInfo* find(const std::string& name) const;
    LocalInfo& require_for_restore(const std::string& name);
    bool scope_index(const std::string& name, std::size_t& out) const;
    StateSnapshot snapshot_states() const;
    void restore_states(const StateSnapshot& snapshot);
    void restore_merged_zone_generations(StateSnapshot target, const StateSnapshot& source);
    void release_borrow_source(const std::string& name, const std::string& path, bool mutable_borrow);
    void release_borrow_sources(const LocalInfo& borrow);
    void release_aggregate_borrow_sources_at(LocalInfo& binding, const std::string& aggregate_path);

    bool name_was_used(const std::string& name) const;
    bool reusable_pattern_binding(const std::string& name) const;
    void mark_name_used(const std::string& name);
    void set_reusable_pattern_bindings(std::set<std::string> names);
    void clear_reusable_pattern_bindings();

    void declare_current(std::string name, LocalInfo local);

private:
    Scope& current_scope();
    const Scope& current_scope() const;
    Scope& scope_at(std::size_t index);
    const Scope& scope_at(std::size_t index) const;

    std::vector<Scope> scopes_;
    std::set<std::string> used_names_;
    std::set<std::string> reusable_pattern_binding_names_;
};

} // namespace ari
