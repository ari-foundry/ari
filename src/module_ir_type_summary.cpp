#include "module_ir_type_summary.hpp"

#include "ir.hpp"

#include <cstddef>
#include <string>

namespace ari {

std::string module_cache_ir_type_name(const IrType& type) {
    std::string base;
    switch (type.primitive) {
        case IrPrimitiveKind::Unknown:
            base = type.name.empty() ? "<unknown>" : type.name;
            break;
        case IrPrimitiveKind::Void:
            base = "void";
            break;
        case IrPrimitiveKind::I8:
        case IrPrimitiveKind::I16:
        case IrPrimitiveKind::I32:
        case IrPrimitiveKind::I64:
        case IrPrimitiveKind::U8:
        case IrPrimitiveKind::U16:
        case IrPrimitiveKind::U32:
        case IrPrimitiveKind::U64:
        case IrPrimitiveKind::F32:
        case IrPrimitiveKind::F64:
        case IrPrimitiveKind::F128:
        case IrPrimitiveKind::Bool:
            base = type.name;
            break;
        case IrPrimitiveKind::StaticStr:
            base = "static_str";
            break;
        case IrPrimitiveKind::Struct:
        case IrPrimitiveKind::Enum:
            base = type.name.empty()
                ? (type.primitive == IrPrimitiveKind::Struct ? "<struct>" : "<enum>")
                : type.name;
            if (!type.args.empty()) {
                base += "[";
                for (std::size_t i = 0; i < type.args.size(); ++i) {
                    if (i > 0) base += ", ";
                    base += module_cache_ir_type_name(type.args[i]);
                }
                base += "]";
            }
            break;
        case IrPrimitiveKind::Tuple:
            base = "(";
            for (std::size_t i = 0; i < type.args.size(); ++i) {
                if (i > 0) base += ", ";
                base += module_cache_ir_type_name(type.args[i]);
            }
            base += ")";
            break;
        case IrPrimitiveKind::Array:
            base = "[";
            base += type.args.empty() ? "<unknown>" : module_cache_ir_type_name(type.args[0]);
            base += ", " + std::to_string(type.array_size) + "]";
            break;
        case IrPrimitiveKind::Vector:
            base = "Vec";
            if (!type.args.empty()) {
                base += "[";
                for (std::size_t i = 0; i < type.args.size(); ++i) {
                    if (i > 0) base += ", ";
                    base += module_cache_ir_type_name(type.args[i]);
                }
                base += "; " + std::to_string(type.array_size) + "]";
            }
            break;
        case IrPrimitiveKind::Zone:
            base = "Zone";
            break;
        case IrPrimitiveKind::Function:
            base = "fn(";
            if (!type.args.empty()) {
                std::size_t param_count = type.args.size() - 1;
                for (std::size_t i = 0; i < param_count; ++i) {
                    if (i > 0) base += ", ";
                    base += module_cache_ir_type_name(type.args[i]);
                }
                base += ") -> ";
                base += module_cache_ir_type_name(type.args.back());
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
                    base += module_cache_ir_type_name(type.args[i]);
                }
                base += "]";
            }
            break;
        case IrPrimitiveKind::MetaType:
            base = "type";
            break;
    }

    switch (type.qualifier) {
        case TypeQualifier::Value:
            return base;
        case TypeQualifier::Own:
            return "own " + base;
        case TypeQualifier::Ref:
            return "ref " + base;
        case TypeQualifier::MutRef:
            return "ref mut " + base;
        case TypeQualifier::Ptr:
            return "ptr " + base;
    }
    return base;
}

} // namespace ari
