#include "std_process_semantics.hpp"

#include "std_vec_semantics.hpp"

namespace ari {

namespace {

bool is_i64_value_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::I64;
}

} // namespace

bool is_std_process_output_zone_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::process::Output";
}

std::optional<std::size_t> std_process_output_zone_handle_source_field_index(const IrType& type) {
    if (!is_std_process_output_zone_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 1;
    if (type.field_names.size() != 3 || type.field_types.size() != 3) return std::nullopt;
    if (type.field_names[0] != "status_value" ||
        type.field_names[1] != "stdout_bytes" ||
        type.field_names[2] != "stderr_bytes") {
        return std::nullopt;
    }
    if (!is_i64_value_type(type.field_types[0]) ||
        !is_std_vec_handle_type(type.field_types[1]) ||
        !is_std_vec_handle_type(type.field_types[2])) {
        return std::nullopt;
    }
    return 1;
}

std::vector<std::vector<std::size_t>> std_process_output_zone_handle_storage_field_path_indices(const IrType& type) {
    if (!std_process_output_zone_handle_source_field_index(type)) return {};
    // `Output` stores stdout and stderr in the same caller-provided zone. The
    // first Vec is the canonical source, and the second must match it.
    return {{1}, {2}};
}

} // namespace ari
