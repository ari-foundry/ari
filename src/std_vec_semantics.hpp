#pragma once

#include "ir.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace ari {

struct StdVecImplicitZoneMethod {
    std::string lowered_name;
    bool allow_untracked_fallback = false;
};

bool is_std_vec_raw_handle_type(const IrType& type);
bool is_std_vec_handle_type(const IrType& type);
bool is_std_vec_iter_handle_type(const IrType& type);
bool is_std_vec_zone_handle_type(const IrType& type);
std::optional<std::size_t> std_vec_raw_handle_data_field_index(const IrType& type);
std::optional<std::size_t> std_vec_zone_handle_source_field_index(const IrType& type);
std::optional<std::vector<std::size_t>> std_vec_zone_handle_data_field_path_indices(const IrType& type);
std::vector<std::vector<std::size_t>> std_vec_zone_handle_storage_field_path_indices(const IrType& type);
bool std_vec_method_requires_same_zone_argument(const std::string& method_name);
std::optional<StdVecImplicitZoneMethod> std_vec_implicit_zone_method_for_call(
    const std::string& method_name,
    std::size_t user_arg_count);
bool std_vec_pointer_result_preserves_receiver_zone(const IrExpr& call);
bool std_vec_result_preserves_receiver_zone(const IrExpr& call);
using StdVecZoneSourceLookup = std::function<bool(const IrExpr&, std::string&)>;
std::optional<std::string> std_vec_same_zone_method_violation(
    const std::string& method_name,
    const IrType& receiver_type,
    const std::vector<IrExprPtr>& args,
    const StdVecZoneSourceLookup& receiver_zone_source,
    const StdVecZoneSourceLookup& argument_zone_source);

} // namespace ari
