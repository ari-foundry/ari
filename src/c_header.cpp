#include "c_header.hpp"

#include "common.hpp"
#include "symbol_mangle.hpp"

#include <cstddef>
#include <map>
#include <set>
#include <sstream>
#include <string>

namespace ari {
namespace {

std::string unqualified_name(const std::string& name) {
    std::size_t split = name.rfind("::");
    if (split == std::string::npos) return name;
    return name.substr(split + 2);
}

struct CRecordInfo {
    std::string c_name;
    bool opaque = false;
};

using CRecordNames = std::map<std::string, CRecordInfo>;
using CEnumNames = std::map<std::string, std::string>;

std::string c_type_name(const IrType& type,
                        const CRecordNames& c_record_names,
                        const CEnumNames& c_enum_names,
                        bool allow_record_values);

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

const CRecordInfo& c_record_info(const IrType& type, const CRecordNames& c_record_names) {
    auto found = c_record_names.find(type.name);
    if (found != c_record_names.end()) return found->second;
    throw CompileError("C header emission does not support aggregate type " + type_name(type) +
                       "; expose a public non-generic @repr(C) struct or an explicit raw pointer ABI");
}

std::string c_record_type_name(const IrType& type, const CRecordNames& c_record_names) {
    return c_record_info(type, c_record_names).c_name;
}

std::string c_pointer_type_name(const std::string& pointee, bool const_pointee) {
    return (const_pointee ? "const " : "") + pointee + "*";
}

std::string c_enum_type_name(const IrType& type, const CEnumNames& c_enum_names) {
    auto found = c_enum_names.find(type.name);
    if (found != c_enum_names.end()) return found->second;
    throw CompileError("C header emission does not support enum type " + type_name(type) +
                       "; expose a public non-generic fieldless @repr(C) enum or an explicit scalar ABI");
}

std::string c_type_name(const IrType& type,
                        const CRecordNames& c_record_names,
                        const CEnumNames& c_enum_names,
                        bool allow_record_values) {
    if (type.qualifier == TypeQualifier::Own) {
        throw CompileError("C header emission does not support owning type " + type_name(type) +
                           "; expose an explicit raw pointer or scalar ABI instead");
    }

    if (type.qualifier == TypeQualifier::Ptr ||
        type.qualifier == TypeQualifier::Ref ||
        type.qualifier == TypeQualifier::MutRef) {
        IrType pointee = type;
        pointee.qualifier = TypeQualifier::Value;
        bool const_pointee = type.qualifier == TypeQualifier::Ref;
        if (pointee.primitive == IrPrimitiveKind::Void) {
            return c_pointer_type_name("void", const_pointee);
        }
        if (pointee.primitive == IrPrimitiveKind::Struct) {
            return c_pointer_type_name(c_record_type_name(pointee, c_record_names),
                                       const_pointee);
        }
        if (pointee.primitive == IrPrimitiveKind::Enum) {
            return c_pointer_type_name(c_enum_type_name(pointee, c_enum_names),
                                       const_pointee);
        }
        return c_pointer_type_name(c_scalar_type_name(pointee), const_pointee);
    }

    if (type.primitive == IrPrimitiveKind::String) {
        throw CompileError("C header emission does not support Ari string values; use ptr c_char");
    }
    if (type.primitive == IrPrimitiveKind::Struct) {
        if (allow_record_values) {
            const CRecordInfo& record = c_record_info(type, c_record_names);
            if (record.opaque) {
                throw CompileError("C header emission does not support by-value generic aggregate type " +
                                   type_name(type) + "; use a raw pointer ABI until concrete generic C layouts are emitted");
            }
            return record.c_name;
        }
        throw CompileError("C header emission does not support aggregate parameter or return type " +
                           type_name(type) + "; expose an explicit raw pointer ABI instead");
    }
    if (type.primitive == IrPrimitiveKind::Enum) {
        return c_enum_type_name(type, c_enum_names);
    }
    return c_scalar_type_name(type);
}

std::string emitted_function_symbol(const IrFunction& fn) {
    return fn.link_name.empty() ? mangle_function_name(fn.name) : fn.link_name;
}

std::string function_prototype(const IrFunction& fn,
                               const CRecordNames& c_record_names,
                               const CEnumNames& c_enum_names) {
    std::ostringstream out;
    out << c_type_name(fn.return_type, c_record_names, c_enum_names, true) << " " << emitted_function_symbol(fn) << "(";
    for (std::size_t i = 0; i < fn.params.size(); ++i) {
        if (i > 0) out << ", ";
        out << c_type_name(fn.params[i].type, c_record_names, c_enum_names, true) << " arg" << i;
    }
    if (fn.params.empty()) out << "void";
    out << ");";
    return out.str();
}

CRecordNames collect_c_record_names(const IrProgram& program) {
    CRecordNames names;
    std::set<std::string> used_c_names;
    for (const auto& record : program.c_records) {
        std::string c_name = record.c_name.empty() ? unqualified_name(record.name) : record.c_name;
        if (!used_c_names.insert(c_name).second) {
            throw CompileError("C header emission found duplicate C record name '" + c_name + "'");
        }
        names.emplace(record.name, CRecordInfo{c_name, record.opaque});
    }
    return names;
}

void require_unique_c_type_names(const IrProgram& program) {
    std::set<std::string> used_c_names;
    for (const auto& item : program.c_records) {
        std::string c_name = item.c_name.empty() ? unqualified_name(item.name) : item.c_name;
        if (!used_c_names.insert(c_name).second) {
            throw CompileError("C header emission found duplicate C type name '" + c_name + "'");
        }
    }
    for (const auto& item : program.c_enums) {
        std::string c_name = item.c_name.empty() ? unqualified_name(item.name) : item.c_name;
        if (!used_c_names.insert(c_name).second) {
            throw CompileError("C header emission found duplicate C type name '" + c_name + "'");
        }
    }
}

CEnumNames collect_c_enum_names(const IrProgram& program) {
    CEnumNames names;
    std::set<std::string> used_case_names;
    for (const auto& item : program.c_enums) {
        std::string c_name = item.c_name.empty() ? unqualified_name(item.name) : item.c_name;
        names.emplace(item.name, c_name);
        for (const auto& enum_case : item.cases) {
            if (!used_case_names.insert(enum_case.c_name).second) {
                throw CompileError("C header emission found duplicate C enum constant '" + enum_case.c_name + "'");
            }
        }
    }
    return names;
}

std::string enum_definition(const IrCEnum& item) {
    std::string c_name = item.c_name.empty() ? unqualified_name(item.name) : item.c_name;
    std::ostringstream out;
    out << "typedef int64_t " << c_name << ";\n";
    out << "enum {\n";
    for (const auto& enum_case : item.cases) {
        out << "    " << enum_case.c_name << " = " << enum_case.tag << ",\n";
    }
    out << "};";
    return out.str();
}

std::string record_forward_declaration(const IrCRecord& record) {
    std::string c_name = record.c_name.empty() ? unqualified_name(record.name) : record.c_name;
    return "typedef struct " + c_name + " " + c_name + ";";
}

std::string record_definition(const IrCRecord& record,
                              const CRecordNames& c_record_names,
                              const CEnumNames& c_enum_names) {
    std::string c_name = record.c_name.empty() ? unqualified_name(record.name) : record.c_name;
    std::ostringstream out;
    out << "struct " << c_name << " {\n";
    for (const auto& field : record.fields) {
        out << "    " << c_type_name(field.type, c_record_names, c_enum_names, false) << " " << field.name << ";\n";
    }
    out << "};";
    return out.str();
}

} // namespace

std::string emit_c_header(const IrProgram& program) {
    require_unique_c_type_names(program);
    CRecordNames c_record_names = collect_c_record_names(program);
    CEnumNames c_enum_names = collect_c_enum_names(program);

    std::ostringstream out;
    out << "#pragma once\n\n";
    out << "#include <stdbool.h>\n";
    out << "#include <stddef.h>\n";
    out << "#include <stdint.h>\n\n";

    if (!program.c_enums.empty()) {
        for (const auto& item : program.c_enums) {
            out << enum_definition(item) << "\n\n";
        }
    }

    if (!program.c_records.empty()) {
        for (const auto& record : program.c_records) {
            out << record_forward_declaration(record) << "\n";
        }
        out << "\n";
        for (const auto& record : program.c_records) {
            if (record.opaque) continue;
            out << record_definition(record, c_record_names, c_enum_names) << "\n\n";
        }
    }

    out << "#ifdef __cplusplus\n";
    out << "extern \"C\" {\n";
    out << "#endif\n\n";

    for (const auto& fn : program.functions) {
        if (!fn.shared_export) continue;
        out << function_prototype(fn, c_record_names, c_enum_names) << "\n";
    }

    out << "\n#ifdef __cplusplus\n";
    out << "}\n";
    out << "#endif\n";
    return out.str();
}

} // namespace ari
