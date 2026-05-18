#include "c_export_semantics.hpp"

#include "attribute_semantics.hpp"
#include "layout.hpp"
#include "module_path.hpp"

#include <cstdint>
#include <optional>
#include <utility>

namespace ari {

std::optional<IrCRecord> build_c_export_record(const StructDecl& decl,
                                               const CExportTypeResolver& resolve_type) {
    if (!decl.is_public) return std::nullopt;
    if (!find_attribute(decl.attributes, "repr")) return std::nullopt;

    IrCRecord record;
    record.name = decl.name;
    record.c_name = qualified_basename(decl.name);
    record.loc = decl.loc;
    record.opaque = !decl.generics.empty();
    if (record.opaque) return record;

    record.fields.reserve(decl.fields.size());
    for (const auto& field : decl.fields) {
        record.fields.push_back(IrCRecordField{
            field.name,
            resolve_type(field.type),
            field.loc
        });
    }
    return record;
}

std::optional<IrCEnum> build_c_export_enum(const EnumDecl& decl,
                                           const CExportEnumTypeLookup& lookup_enum_type) {
    if (!decl.is_public) return std::nullopt;
    if (!find_attribute(decl.attributes, "repr")) return std::nullopt;

    IrCEnum item;
    item.name = decl.name;
    item.c_name = qualified_basename(decl.name);
    item.loc = decl.loc;

    if (std::optional<IrType> type = lookup_enum_type(decl.name)) {
        item.type = std::move(*type);
        item.aggregate_layout = ari_has_aggregate_enum_layout(item.type);
    }

    item.cases.reserve(decl.cases.size());
    for (std::size_t i = 0; i < decl.cases.size(); ++i) {
        const EnumCase& enum_case = decl.cases[i];
        item.cases.push_back(IrCEnumCase{
            enum_case.name,
            item.c_name + "_" + enum_case.name,
            static_cast<std::uint32_t>(i),
            enum_case.loc
        });
    }
    return item;
}

} // namespace ari
