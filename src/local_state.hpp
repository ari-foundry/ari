#pragma once

#include "ir.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
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
    bool zone_pointer = false;
    std::string zone_pointer_source;
    std::uint64_t zone_pointer_generation = 0;
    std::uint64_t zone_generation = 0;
    std::string generic_origin;
    struct BorrowSource {
        std::string name;
        std::string path;
        bool mutable_borrow = false;
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

LocalState snapshot_state(const StateSnapshot& snapshot, const std::string& name);
void merge_zone_generations_into(StateSnapshot& target, const StateSnapshot& source);
void merge_existing_zone_generations_into(StateSnapshot& target, const StateSnapshot& source);
StateSnapshot merge_zone_generations(StateSnapshot target, const StateSnapshot& source);

class LocalScopeStack {
public:
    using Scope = std::map<std::string, LocalInfo>;

    void clear();
    void push_scope();
    void pop_scope();
    bool empty() const;
    std::size_t size() const;

    Scope& current_scope();
    const Scope& current_scope() const;
    Scope& scope_at(std::size_t index);
    const Scope& scope_at(std::size_t index) const;

    LocalInfo* find(const std::string& name);
    const LocalInfo* find(const std::string& name) const;
    LocalInfo& require_for_restore(const std::string& name);
    bool scope_index(const std::string& name, std::size_t& out) const;
    StateSnapshot snapshot_states() const;
    void restore_states(const StateSnapshot& snapshot);

    bool name_was_used(const std::string& name) const;
    bool reusable_pattern_binding(const std::string& name) const;
    void mark_name_used(const std::string& name);
    void set_reusable_pattern_bindings(std::set<std::string> names);
    void clear_reusable_pattern_bindings();

    void declare_current(std::string name, LocalInfo local);

private:
    std::vector<Scope> scopes_;
    std::set<std::string> used_names_;
    std::set<std::string> reusable_pattern_binding_names_;
};

} // namespace ari
