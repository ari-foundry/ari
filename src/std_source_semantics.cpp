#include "std_source_semantics.hpp"

#include "type_semantics.hpp"

#include <string>
#include <vector>

namespace ari {

namespace {

bool is_i64_value_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::I64;
}

bool is_source_file_value_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::source::SourceFile";
}

bool is_source_file_ptr_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Ptr &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::source::SourceFile";
}

bool is_line_map_ptr_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Ptr &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::source::LineMap";
}

std::optional<std::size_t> line_map_starts_index(const IrType& type) {
    if (!is_std_source_line_map_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 1;
    if (type.field_names.size() != 3 || type.field_types.size() != 3) {
        return std::nullopt;
    }

    bool has_file = false;
    bool has_lines = false;
    std::optional<std::size_t> starts_index;
    for (std::size_t i = 0; i < type.field_names.size(); ++i) {
        const std::string& name = type.field_names[i];
        const IrType& field_type = type.field_types[i];
        if (name == "file") {
            if (!is_source_file_value_type(field_type)) return std::nullopt;
            has_file = true;
        } else if (name == "starts") {
            if (field_type.qualifier != TypeQualifier::Ptr ||
                field_type.primitive != IrPrimitiveKind::I64) {
                return std::nullopt;
            }
            starts_index = i;
        } else if (name == "lines") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_lines = true;
        } else {
            return std::nullopt;
        }
    }

    if (!has_file || !starts_index || !has_lines) return std::nullopt;
    return starts_index;
}

std::optional<std::size_t> source_map_files_index(const IrType& type) {
    if (!is_std_source_source_map_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 0;
    if (type.field_names.size() != 4 || type.field_types.size() != 4) {
        return std::nullopt;
    }

    std::optional<std::size_t> files_index;
    bool has_maps = false;
    bool has_len = false;
    bool has_capacity = false;
    for (std::size_t i = 0; i < type.field_names.size(); ++i) {
        const std::string& name = type.field_names[i];
        const IrType& field_type = type.field_types[i];
        if (name == "files") {
            if (!is_source_file_ptr_type(field_type)) return std::nullopt;
            files_index = i;
        } else if (name == "maps") {
            if (!is_line_map_ptr_type(field_type)) return std::nullopt;
            has_maps = true;
        } else if (name == "len") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_len = true;
        } else if (name == "capacity") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_capacity = true;
        } else {
            return std::nullopt;
        }
    }

    if (!files_index || !has_maps || !has_len || !has_capacity) return std::nullopt;
    return files_index;
}

} // namespace

bool is_std_source_line_map_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::source::LineMap";
}

bool is_std_source_source_map_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::source::SourceMap";
}

bool is_std_source_zone_handle_type(const IrType& type) {
    return is_std_source_line_map_handle_type(type) ||
           is_std_source_source_map_handle_type(type);
}

std::optional<std::size_t> std_source_zone_handle_source_field_index(
    const IrType& type) {
    if (is_std_source_line_map_handle_type(type)) return line_map_starts_index(type);
    return source_map_files_index(type);
}

std::vector<std::vector<std::size_t>> std_source_zone_handle_storage_field_path_indices(
    const IrType& type) {
    if (is_std_source_line_map_handle_type(type)) {
        std::optional<std::size_t> starts_index = line_map_starts_index(type);
        if (!starts_index) return {};
        return {{*starts_index}};
    }
    if (is_std_source_source_map_handle_type(type)) {
        if (!source_map_files_index(type)) return {};
        if (type.field_names.empty() && type.field_types.empty()) return {{0}, {1}};

        std::optional<std::size_t> files_index;
        std::optional<std::size_t> maps_index;
        for (std::size_t i = 0; i < type.field_names.size(); ++i) {
            if (type.field_names[i] == "files") files_index = i;
            if (type.field_names[i] == "maps") maps_index = i;
        }
        if (!files_index || !maps_index) return {};
        return {{*files_index}, {*maps_index}};
    }
    return {};
}

} // namespace ari
