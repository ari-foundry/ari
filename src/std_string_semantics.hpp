#pragma once

#include "ir.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace ari {

struct StdStringImplicitZoneMethod {
    std::string lowered_name;
    bool allow_untracked_fallback = false;
};

bool is_std_string_raw_handle_type(const IrType& type);
bool is_std_string_handle_type(const IrType& type);
bool is_std_string_zone_handle_type(const IrType& type);
std::optional<std::size_t> std_string_raw_handle_data_field_index(const IrType& type);
std::optional<std::size_t> std_string_zone_handle_source_field_index(const IrType& type);
std::optional<std::vector<std::size_t>> std_string_zone_handle_data_field_path_indices(const IrType& type);
bool std_string_pointer_result_preserves_receiver_zone(const IrExpr& call);
bool std_string_extern_builtin_allows_zone_pointer_argument(const std::string& function_name,
                                                           std::size_t arg_index);
bool std_string_method_requires_same_zone_argument(const std::string& method_name);
std::optional<StdStringImplicitZoneMethod> std_string_implicit_zone_method_for_call(
    const std::string& method_name,
    std::size_t user_arg_count);
using StdStringZoneSourceLookup = std::function<bool(const IrExpr&, std::string&)>;
std::optional<std::string> std_string_same_zone_method_violation(
    const std::string& method_name,
    const IrType& receiver_type,
    const std::vector<IrExprPtr>& args,
    const StdStringZoneSourceLookup& receiver_zone_source,
    const StdStringZoneSourceLookup& argument_zone_source);

} // namespace ari
