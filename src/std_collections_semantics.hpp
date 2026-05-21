#pragma once

#include "ir.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace ari {

struct StdCollectionsImplicitZoneMethod {
    std::string lowered_name;
    bool allow_untracked_fallback = false;
};

// Compiler-side provenance rules for std::collections source handles.
// Collection behavior stays in Ari source; sema only tracks zone ownership.
bool is_std_collections_set_handle_type(const IrType& type);
bool is_std_collections_iter_handle_type(const IrType& type);
bool is_std_collections_map_update_entry_handle_type(const IrType& type);
bool is_std_collections_mutable_handle_type(const IrType& type);
bool is_std_collections_zone_handle_type(const IrType& type);
std::string std_collections_handle_display_name(const IrType& type);
std::optional<std::size_t> std_collections_set_zone_handle_source_field_index(const IrType& type);
std::optional<std::vector<std::size_t>> std_collections_set_zone_handle_data_field_path_indices(const IrType& type);
std::vector<std::vector<std::size_t>> std_collections_zone_handle_storage_field_path_indices(const IrType& type);
bool std_collections_set_method_requires_same_zone_argument(const std::string& method_name);
std::optional<StdCollectionsImplicitZoneMethod> std_collections_implicit_zone_method_for_call(
    const IrType& receiver_type,
    const std::string& method_name,
    std::size_t user_arg_count);
bool std_collections_result_preserves_receiver_zone(const IrExpr& call);

using StdCollectionsZoneSourceLookup = std::function<bool(const IrExpr&, std::string&)>;
std::optional<std::string> std_collections_set_same_zone_method_violation(
    const std::string& method_name,
    const IrType& receiver_type,
    const std::vector<IrExprPtr>& args,
    const StdCollectionsZoneSourceLookup& receiver_zone_source,
    const StdCollectionsZoneSourceLookup& argument_zone_source);

} // namespace ari
