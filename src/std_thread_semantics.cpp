#include "std_thread_semantics.hpp"

namespace ari {

bool is_std_thread_zone_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           (type.name == "std::thread::ThreadLocal" ||
            type.name == "std::thread::ThreadScope" ||
            type.name == "std::thread::ThreadLocalState");
}

std::optional<std::size_t> std_thread_zone_handle_source_field_index(const IrType& type) {
    if (!is_std_thread_zone_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 0;
    if (type.name == "std::thread::ThreadLocal") {
        if (type.field_names.size() != 1 || type.field_types.size() != 1) return std::nullopt;
        if (type.field_names[0] != "state") return std::nullopt;
        if (type.field_types[0].qualifier != TypeQualifier::Ptr) return std::nullopt;
        return 0;
    }
    if (type.name == "std::thread::ThreadLocalState") {
        if (type.field_names.size() != 3 || type.field_types.size() != 3) return std::nullopt;
        if (type.field_names[1] != "slots") return std::nullopt;
        if (type.field_types[1].qualifier != TypeQualifier::Ptr) return std::nullopt;
        return 1;
    }
    if (type.name == "std::thread::ThreadScope") {
        if (type.field_names.size() != 4 || type.field_types.size() != 4) return std::nullopt;
        if (type.field_names[0] != "handles") return std::nullopt;
        if (type.field_types[0].qualifier != TypeQualifier::Ptr) return std::nullopt;
        return 0;
    }
    return std::nullopt;
}

} // namespace ari
