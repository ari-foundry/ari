#include "c_header.hpp"

#include "common.hpp"
#include "symbol_mangle.hpp"

#include <cstddef>
#include <sstream>
#include <string>

namespace ari {
namespace {

std::string c_type_name(const IrType& type);

std::string c_scalar_type_name(const IrType& type) {
    switch (type.primitive) {
        case IrPrimitiveKind::Void: return "void";
        case IrPrimitiveKind::I8: return "int8_t";
        case IrPrimitiveKind::I16: return "int16_t";
        case IrPrimitiveKind::I32: return "int32_t";
        case IrPrimitiveKind::I64: return "int64_t";
        case IrPrimitiveKind::U8: return "uint8_t";
        case IrPrimitiveKind::U16: return "uint16_t";
        case IrPrimitiveKind::U32: return "uint32_t";
        case IrPrimitiveKind::U64: return "uint64_t";
        case IrPrimitiveKind::F32: return "float";
        case IrPrimitiveKind::F64: return "double";
        case IrPrimitiveKind::Bool: return "bool";
        default:
            break;
    }
    throw CompileError("C header emission does not support type " + type_name(type) +
                       "; use scalar C ABI aliases or raw pointers");
}

std::string c_type_name(const IrType& type) {
    if (type.qualifier == TypeQualifier::Own) {
        throw CompileError("C header emission does not support owning type " + type_name(type) +
                           "; expose an explicit raw pointer or scalar ABI instead");
    }

    if (type.qualifier == TypeQualifier::Ptr ||
        type.qualifier == TypeQualifier::Ref ||
        type.qualifier == TypeQualifier::MutRef) {
        IrType pointee = type;
        pointee.qualifier = TypeQualifier::Value;
        if (pointee.primitive == IrPrimitiveKind::Void) return "void*";
        return c_scalar_type_name(pointee) + "*";
    }

    if (type.primitive == IrPrimitiveKind::String) {
        throw CompileError("C header emission does not support Ari string values; use ptr c_char");
    }
    return c_scalar_type_name(type);
}

std::string emitted_function_symbol(const IrFunction& fn) {
    return fn.link_name.empty() ? mangle_function_name(fn.name) : fn.link_name;
}

std::string function_prototype(const IrFunction& fn) {
    std::ostringstream out;
    out << c_type_name(fn.return_type) << " " << emitted_function_symbol(fn) << "(";
    for (std::size_t i = 0; i < fn.params.size(); ++i) {
        if (i > 0) out << ", ";
        out << c_type_name(fn.params[i].type) << " arg" << i;
    }
    if (fn.params.empty()) out << "void";
    out << ");";
    return out.str();
}

} // namespace

std::string emit_c_header(const IrProgram& program) {
    std::ostringstream out;
    out << "#pragma once\n\n";
    out << "#include <stdbool.h>\n";
    out << "#include <stddef.h>\n";
    out << "#include <stdint.h>\n\n";
    out << "#ifdef __cplusplus\n";
    out << "extern \"C\" {\n";
    out << "#endif\n\n";

    for (const auto& fn : program.functions) {
        if (!fn.shared_export) continue;
        out << function_prototype(fn) << "\n";
    }

    out << "\n#ifdef __cplusplus\n";
    out << "}\n";
    out << "#endif\n";
    return out.str();
}

} // namespace ari
