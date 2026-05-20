#include "std_fs_semantics.hpp"

#include "std_string_semantics.hpp"

namespace ari {

bool is_std_fs_dir_entry_zone_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::fs::DirEntry";
}

std::optional<std::size_t> std_fs_dir_entry_zone_handle_source_field_index(const IrType& type) {
    if (!is_std_fs_dir_entry_zone_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 0;
    if (type.field_names.size() != 2 || type.field_types.size() != 2) return std::nullopt;
    if (type.field_names[0] != "name_value" || type.field_names[1] != "path_value") {
        return std::nullopt;
    }
    if (!is_std_string_handle_type(type.field_types[0]) ||
        !is_std_string_handle_type(type.field_types[1])) {
        return std::nullopt;
    }
    return 0;
}

std::vector<std::vector<std::size_t>> std_fs_dir_entry_zone_handle_storage_field_path_indices(const IrType& type) {
    if (!std_fs_dir_entry_zone_handle_source_field_index(type)) return {};
    return {{0}, {1}};
}

} // namespace ari
