#include "ir.hpp"

#include <cstddef>

namespace ari {

static std::string qualifier_name(TypeQualifier qualifier) {
    switch (qualifier) {
        case TypeQualifier::Own: return "own ";
        case TypeQualifier::Ref: return "ref ";
        case TypeQualifier::MutRef: return "ref mut ";
        case TypeQualifier::Ptr: return "ptr ";
        case TypeQualifier::Value: return "";
    }
    return "";
}

std::string type_name(const IrType& type) {
    std::string base;
    switch (type.primitive) {
        case IrPrimitiveKind::Void:
            base = "void";
            break;
        case IrPrimitiveKind::I8:
            base = "i8";
            break;
        case IrPrimitiveKind::I16:
            base = "i16";
            break;
        case IrPrimitiveKind::I32:
            base = "i32";
            break;
        case IrPrimitiveKind::I64:
            base = "i64";
            break;
        case IrPrimitiveKind::U8:
            base = "u8";
            break;
        case IrPrimitiveKind::U16:
            base = "u16";
            break;
        case IrPrimitiveKind::U32:
            base = "u32";
            break;
        case IrPrimitiveKind::U64:
            base = "u64";
            break;
        case IrPrimitiveKind::F32:
            base = "f32";
            break;
        case IrPrimitiveKind::F64:
            base = "f64";
            break;
        case IrPrimitiveKind::F128:
            base = "f128";
            break;
        case IrPrimitiveKind::Bool:
            base = "bool";
            break;
        case IrPrimitiveKind::StaticStr:
            base = "static string literal";
            break;
        case IrPrimitiveKind::Struct:
            if (!type.field_names.empty() &&
                type.field_names[0] == "$call" &&
                !type.args.empty() &&
                type.array_size + 1 == type.args.size()) {
                base = "closure fn(";
                std::size_t param_count = static_cast<std::size_t>(type.array_size);
                for (std::size_t i = 0; i < param_count; ++i) {
                    if (i > 0) base += ", ";
                    base += type_name(type.args[i]);
                }
                base += ") -> ";
                base += type_name(type.args[param_count]);
                break;
            }
            base = type.name.empty() ? "<struct>" : type.name;
            if (!type.args.empty()) {
                base += "[";
                for (std::size_t i = 0; i < type.args.size(); ++i) {
                    if (i > 0) base += ", ";
                    base += type_name(type.args[i]);
                }
                base += "]";
            }
            break;
        case IrPrimitiveKind::Enum:
            base = type.name.empty() ? "<enum>" : type.name;
            if (!type.args.empty()) {
                base += "[";
                for (std::size_t i = 0; i < type.args.size(); ++i) {
                    if (i > 0) base += ", ";
                    base += type_name(type.args[i]);
                }
                base += "]";
            }
            break;
        case IrPrimitiveKind::Tuple:
            base = "(";
            for (std::size_t i = 0; i < type.args.size(); ++i) {
                if (i > 0) base += ", ";
                base += type_name(type.args[i]);
            }
            base += ")";
            break;
        case IrPrimitiveKind::Array:
            base = "[";
            if (!type.args.empty()) base += type_name(type.args[0]);
            else base += "<unknown>";
            base += ", " + std::to_string(type.array_size) + "]";
            break;
        case IrPrimitiveKind::Vector:
            base = "Vec";
            if (!type.args.empty()) {
                base += "[";
                for (std::size_t i = 0; i < type.args.size(); ++i) {
                    if (i > 0) base += ", ";
                    base += type_name(type.args[i]);
                }
                base += "]";
            }
            break;
        case IrPrimitiveKind::Zone:
            base = "Zone";
            break;
        case IrPrimitiveKind::Function:
            base = "fn(";
            if (!type.args.empty()) {
                std::size_t param_count = static_cast<std::size_t>(type.array_size);
                if (param_count + 1 > type.args.size()) param_count = type.args.size() - 1;
                for (std::size_t i = 0; i < param_count; ++i) {
                    if (i > 0) base += ", ";
                    base += type_name(type.args[i]);
                }
                base += ") -> ";
                base += type_name(type.args[param_count]);
            } else {
                base += ") -> <unknown>";
            }
            break;
        case IrPrimitiveKind::TraitObject:
            base = "dyn ";
            base += type.name.empty() ? "<trait>" : type.name;
            if (!type.args.empty()) {
                base += "[";
                for (std::size_t i = 0; i < type.args.size(); ++i) {
                    if (i > 0) base += ", ";
                    base += type_name(type.args[i]);
                }
                base += "]";
            }
            break;
        case IrPrimitiveKind::MetaType:
            base = "type";
            break;
        case IrPrimitiveKind::Unknown:
            base = type.name.empty() ? "<unknown>" : type.name;
            break;
    }
    return qualifier_name(type.qualifier) + base;
}

} // namespace ari
