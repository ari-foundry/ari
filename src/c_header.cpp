#include "c_header.hpp"

#include "aggregate_abi.hpp"
#include "common.hpp"
#include "layout.hpp"
#include "symbol_mangle.hpp"
#include "target.hpp"

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ari {
namespace {

std::string unqualified_name(const std::string& name) {
    std::size_t split = name.rfind("::");
    if (split == std::string::npos) return name;
    return name.substr(split + 2);
}

bool is_std_module_name(const std::string& module_name) {
    return module_name == "std" ||
           (module_name.size() > 5 && module_name.compare(0, 5, "std::") == 0);
}

bool should_emit_function_prototype(const IrFunction& fn) {
    if (!fn.shared_export) return false;
    return !is_std_module_name(fn.module_name) || !fn.link_name.empty();
}

struct CRecordInfo {
    std::string c_name;
    bool opaque = false;
};

struct CConcreteRecord {
    std::string key;
    std::string c_name;
    std::vector<IrCRecordField> fields;
    SourceLocation loc;
};

struct CEnumInfo {
    std::string c_name;
    bool aggregate_layout = false;
    IrType type;
};

enum class CGeneratedAggregateKind {
    Array,
    Tuple,
    Vector,
    AggregateEnum
};

struct CGeneratedAggregate {
    std::string key;
    std::string c_name;
    IrType type;
    CGeneratedAggregateKind kind = CGeneratedAggregateKind::Array;
};

using CRecordNames = std::map<std::string, CRecordInfo>;
using CConcreteRecordNames = std::map<std::string, std::string>;
using CEnumNames = std::map<std::string, CEnumInfo>;
using CGeneratedAggregateNames = std::map<std::string, std::string>;

std::string c_type_name(const IrType& type,
                        const CRecordNames& c_record_names,
                        const CConcreteRecordNames& c_concrete_record_names,
                        const CEnumNames& c_enum_names,
                        const CGeneratedAggregateNames& c_generated_aggregate_names,
                        bool allow_record_values);
std::string c_declaration(const IrType& type,
                          const CRecordNames& c_record_names,
                          const CConcreteRecordNames& c_concrete_record_names,
                          const CEnumNames& c_enum_names,
                          const CGeneratedAggregateNames& c_generated_aggregate_names,
                          const std::string& declarator,
                          bool allow_record_values,
                          bool const_value = false);
std::string c_record_type_name(const IrType& type, const CRecordNames& c_record_names);
std::string c_generated_aggregate_type_name(const IrType& type,
                                            const CGeneratedAggregateNames& c_generated_aggregate_names);

std::string c_identifier_suffix(const std::string& text) {
    std::string out;
    for (unsigned char c : text) {
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9')) {
            out.push_back(static_cast<char>(c));
        } else {
            out.push_back('_');
        }
    }
    while (!out.empty() && out.front() == '_') out.erase(out.begin());
    while (!out.empty() && out.back() == '_') out.pop_back();
    if (out.empty()) return "type";
    return out;
}

std::string c_collapsed_identifier_suffix(const std::string& text) {
    std::string suffix = c_identifier_suffix(text);
    std::string out;
    bool previous_underscore = false;
    for (char c : suffix) {
        if (c == '_') {
            if (previous_underscore) continue;
            previous_underscore = true;
        } else {
            previous_underscore = false;
        }
        out.push_back(c);
    }
    if (out.empty()) return "type";
    return out;
}

std::string concrete_record_key(const IrType& type) {
    return type_name(type);
}

std::string concrete_record_c_name(const IrType& type, const CRecordNames& c_record_names) {
    std::string out = c_record_type_name(type, c_record_names);
    for (const auto& arg : type.args) {
        out += "_";
        out += c_identifier_suffix(type_name(arg));
    }
    return out;
}

bool is_value_array_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Array;
}

bool is_fixed_vector_storage_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Vector &&
           type.args.size() == 1 &&
           type.array_size > 0 &&
           type.field_types.size() == 2;
}

bool is_generated_aggregate_type(const IrType& type) {
    return is_value_array_type(type) ||
           (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Tuple) ||
           is_fixed_vector_storage_type(type) ||
           ari_has_aggregate_enum_layout(type);
}

CGeneratedAggregateKind generated_aggregate_kind(const IrType& type) {
    if (is_value_array_type(type)) return CGeneratedAggregateKind::Array;
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Tuple) {
        return CGeneratedAggregateKind::Tuple;
    }
    if (is_fixed_vector_storage_type(type)) return CGeneratedAggregateKind::Vector;
    if (ari_has_aggregate_enum_layout(type)) return CGeneratedAggregateKind::AggregateEnum;
    throw CompileError("internal error: unsupported generated C aggregate type " + type_name(type));
}

std::string generated_aggregate_key(const IrType& type) {
    if (type.qualifier == TypeQualifier::Value &&
        type.primitive == IrPrimitiveKind::Vector &&
        type.args.size() == 1) {
        return "Vec[" + type_name(type.args[0]) + "; " + std::to_string(type.array_size) + "]";
    }
    return type_name(type);
}

std::string c_generated_aggregate_c_name(const IrType& type) {
    switch (generated_aggregate_kind(type)) {
        case CGeneratedAggregateKind::Array:
            return "AriArray_" + c_collapsed_identifier_suffix(generated_aggregate_key(type));
        case CGeneratedAggregateKind::Tuple:
            return "AriTuple_" + c_collapsed_identifier_suffix(generated_aggregate_key(type));
        case CGeneratedAggregateKind::Vector:
            return "AriVec_" + c_collapsed_identifier_suffix(generated_aggregate_key(type));
        case CGeneratedAggregateKind::AggregateEnum:
            return "AriEnum_" + c_collapsed_identifier_suffix(generated_aggregate_key(type));
    }
    throw CompileError("internal error: unknown generated C aggregate kind");
}

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
    if (found != c_enum_names.end()) return found->second.c_name;
    throw CompileError("C header emission does not support enum type " + type_name(type) +
                       "; expose a public @repr(C) enum or an explicit scalar ABI");
}

const CEnumInfo* c_enum_info_for_type(const IrType& type, const CEnumNames& c_enum_names) {
    if (type.qualifier != TypeQualifier::Value || type.primitive != IrPrimitiveKind::Enum) {
        return nullptr;
    }
    auto found = c_enum_names.find(type.name);
    if (found == c_enum_names.end()) return nullptr;
    return &found->second;
}

bool is_c_aggregate_enum_type(const IrType& type, const CEnumNames& c_enum_names) {
    const CEnumInfo* info = c_enum_info_for_type(type, c_enum_names);
    return info && info->aggregate_layout;
}

std::string c_generated_aggregate_type_name(
    const IrType& type,
    const CGeneratedAggregateNames& c_generated_aggregate_names) {
    auto found = c_generated_aggregate_names.find(generated_aggregate_key(type));
    if (found != c_generated_aggregate_names.end()) return found->second;
    throw CompileError("C header emission has no generated aggregate ABI wrapper for " +
                       generated_aggregate_key(type));
}

std::string c_type_name(const IrType& type,
                        const CRecordNames& c_record_names,
                        const CConcreteRecordNames& c_concrete_record_names,
                        const CEnumNames& c_enum_names,
                        const CGeneratedAggregateNames& c_generated_aggregate_names,
                        bool allow_record_values) {
    if (type.qualifier == TypeQualifier::Own) {
        throw CompileError("C header emission does not support ownership-qualified type " + type_name(type) +
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
            const CEnumInfo* enum_info = c_enum_info_for_type(pointee, c_enum_names);
            if (enum_info) {
                return c_pointer_type_name(enum_info->c_name, const_pointee);
            }
        }
        if (is_generated_aggregate_type(pointee) && !is_value_array_type(pointee)) {
            return c_pointer_type_name(
                c_generated_aggregate_type_name(pointee, c_generated_aggregate_names),
                const_pointee);
        }
        if (pointee.primitive == IrPrimitiveKind::Enum) {
            return c_pointer_type_name(c_enum_type_name(pointee, c_enum_names),
                                       const_pointee);
        }
        if (pointee.primitive == IrPrimitiveKind::Array) {
            throw CompileError("C header emission needs a declarator for pointer-to-array type " +
                               type_name(type));
        }
        return c_pointer_type_name(c_scalar_type_name(pointee), const_pointee);
    }

    if (type.primitive == IrPrimitiveKind::StaticStr) {
        throw CompileError("C header emission does not support static string literal values; use ptr c_char");
    }
    if (type.primitive == IrPrimitiveKind::Enum) {
        const CEnumInfo* enum_info = c_enum_info_for_type(type, c_enum_names);
        if (enum_info) return enum_info->c_name;
    }
    if (allow_record_values && is_generated_aggregate_type(type)) {
        return c_generated_aggregate_type_name(type, c_generated_aggregate_names);
    }
    if (type.primitive == IrPrimitiveKind::Struct) {
        if (allow_record_values) {
            const CRecordInfo& record = c_record_info(type, c_record_names);
            if (record.opaque) {
                auto concrete = c_concrete_record_names.find(concrete_record_key(type));
                if (concrete != c_concrete_record_names.end()) return concrete->second;
                throw CompileError("C header emission does not support by-value generic aggregate type " +
                                   type_name(type) + "; no concrete C layout was collected for this instantiation");
            }
            return record.c_name;
        }
        throw CompileError("C header emission does not support aggregate parameter or return type " +
                           type_name(type) + "; expose an explicit raw pointer ABI instead");
    }
    if (type.primitive == IrPrimitiveKind::Enum) {
        return c_enum_type_name(type, c_enum_names);
    }
    if (type.primitive == IrPrimitiveKind::Array) {
        throw CompileError("C header emission needs a declarator for fixed-array type " +
                           type_name(type));
    }
    return c_scalar_type_name(type);
}

std::string c_declaration(const IrType& type,
                          const CRecordNames& c_record_names,
                          const CConcreteRecordNames& c_concrete_record_names,
                          const CEnumNames& c_enum_names,
                          const CGeneratedAggregateNames& c_generated_aggregate_names,
                          const std::string& declarator,
                          bool allow_record_values,
                          bool const_value) {
    if (type.qualifier == TypeQualifier::Own) {
        throw CompileError("C header emission does not support ownership-qualified type " + type_name(type) +
                           "; expose an explicit raw pointer or scalar ABI instead");
    }

    if (type.qualifier == TypeQualifier::Ptr ||
        type.qualifier == TypeQualifier::Ref ||
        type.qualifier == TypeQualifier::MutRef) {
        IrType pointee = type;
        pointee.qualifier = TypeQualifier::Value;
        if (!is_value_array_type(pointee)) {
            return c_type_name(
                type,
                c_record_names,
                c_concrete_record_names,
                c_enum_names,
                c_generated_aggregate_names,
                allow_record_values) + " " + declarator;
        }
        std::string pointer_declarator = "(*" + declarator + ")";
        return c_declaration(
            pointee,
            c_record_names,
            c_concrete_record_names,
            c_enum_names,
            c_generated_aggregate_names,
            pointer_declarator,
            allow_record_values,
            type.qualifier == TypeQualifier::Ref);
    }

    if (type.primitive == IrPrimitiveKind::Array) {
        if (type.args.size() != 1) {
            throw CompileError("C header emission cannot render malformed fixed-array type " +
                               type_name(type));
        }
        return c_declaration(
            type.args[0],
            c_record_names,
            c_concrete_record_names,
            c_enum_names,
            c_generated_aggregate_names,
            declarator + "[" + std::to_string(type.array_size) + "]",
            allow_record_values,
            const_value);
    }

    std::string base = c_type_name(
        type,
        c_record_names,
        c_concrete_record_names,
        c_enum_names,
        c_generated_aggregate_names,
        allow_record_values);
    if (const_value) base = "const " + base;
    return base + " " + declarator;
}

std::string emitted_function_symbol(const IrFunction& fn) {
    return fn.link_name.empty() ? mangle_function_name(fn.name) : fn.link_name;
}

std::string pointer_abi_hint(const IrType& type) {
    if (type.primitive == IrPrimitiveKind::Struct) return type.name;
    return type_name(type);
}

[[noreturn]] void fail_c_header_abi(SourceLocation loc,
                                    std::string message,
                                    std::string note,
                                    std::string help) {
    CompileError error(std::move(loc), std::move(message));
    error.add_note(DiagnosticNote{std::nullopt, std::move(note), DiagnosticNoteKind::Note});
    error.add_note(DiagnosticNote{std::nullopt, std::move(help), DiagnosticNoteKind::Help});
    throw error;
}

std::string c_header_aggregate_abi_layout_note(const NonLocalAggregateAbi& abi) {
    if (abi.size_bytes == 0 && abi.align_bytes == 0) return "";
    return " Ari computed this aggregate as " + std::to_string(abi.size_bytes) +
           " bytes with " + std::to_string(abi.align_bytes) + "-byte alignment.";
}

std::string c_function_value_type_name(const IrType& type,
                                       const CRecordNames& c_record_names,
                                       const CConcreteRecordNames& c_concrete_record_names,
                                       const CEnumNames& c_enum_names,
                                       const CGeneratedAggregateNames& c_generated_aggregate_names,
                                       bool allow_record_values) {
    if (c_enum_info_for_type(type, c_enum_names)) {
        return c_enum_type_name(type, c_enum_names);
    }
    if (is_generated_aggregate_type(type)) {
        return c_generated_aggregate_type_name(type, c_generated_aggregate_names);
    }
    return c_type_name(
        type,
        c_record_names,
        c_concrete_record_names,
        c_enum_names,
        c_generated_aggregate_names,
        allow_record_values);
}

std::string function_prototype(const IrFunction& fn,
                               const CRecordNames& c_record_names,
                               const CConcreteRecordNames& c_concrete_record_names,
                               const CEnumNames& c_enum_names,
                               const CGeneratedAggregateNames& c_generated_aggregate_names) {
    std::ostringstream out;
    out << c_function_value_type_name(
            fn.return_type,
            c_record_names,
            c_concrete_record_names,
            c_enum_names,
            c_generated_aggregate_names,
            true)
        << " " << emitted_function_symbol(fn) << "(";
    for (std::size_t i = 0; i < fn.params.size(); ++i) {
        if (i > 0) out << ", ";
        std::string arg_name = "arg" + std::to_string(i);
        if (is_generated_aggregate_type(fn.params[i].type) &&
            !c_enum_info_for_type(fn.params[i].type, c_enum_names)) {
            out << c_generated_aggregate_type_name(
                fn.params[i].type,
                c_generated_aggregate_names) << " " << arg_name;
        } else {
            out << c_declaration(
                fn.params[i].type,
                c_record_names,
                c_concrete_record_names,
                c_enum_names,
                c_generated_aggregate_names,
                arg_name,
                true);
        }
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
        names.emplace(item.name, CEnumInfo{c_name, item.aggregate_layout, item.type});
        for (const auto& enum_case : item.cases) {
            if (!used_case_names.insert(enum_case.c_name).second) {
                throw CompileError("C header emission found duplicate C enum constant '" + enum_case.c_name + "'");
            }
        }
    }
    return names;
}

void collect_concrete_record_type(const IrType& type,
                                  const CRecordNames& c_record_names,
                                  std::map<std::string, CConcreteRecord>& records) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Array && type.args.size() == 1) {
        collect_concrete_record_type(type.args[0], c_record_names, records);
        return;
    }
    if (type.qualifier == TypeQualifier::Value &&
        (type.primitive == IrPrimitiveKind::Tuple ||
         type.primitive == IrPrimitiveKind::Vector ||
         ari_has_aggregate_enum_layout(type))) {
        for (const auto& field_type : ari_aggregate_field_types(type)) {
            collect_concrete_record_type(field_type, c_record_names, records);
        }
        return;
    }
    if (type.qualifier != TypeQualifier::Value || type.primitive != IrPrimitiveKind::Struct) return;
    const CRecordInfo& source = c_record_info(type, c_record_names);
    if (!source.opaque || type.args.empty()) return;

    std::string key = concrete_record_key(type);
    if (records.count(key)) return;
    if (type.field_names.size() != type.field_types.size()) {
        throw CompileError("C header emission cannot materialize concrete generic aggregate type " +
                           type_name(type) + "; missing instantiated field layout");
    }

    CConcreteRecord record;
    record.key = key;
    record.c_name = concrete_record_c_name(type, c_record_names);
    record.loc = type.loc;
    for (std::size_t i = 0; i < type.field_names.size(); ++i) {
        record.fields.push_back(IrCRecordField{
            type.field_names[i],
            type.field_types[i],
            type.loc
        });
    }

    for (const auto& field : record.fields) {
        collect_concrete_record_type(field.type, c_record_names, records);
    }
    records.emplace(key, std::move(record));
}

std::vector<CConcreteRecord> collect_concrete_records(const IrProgram& program,
                                                      const CRecordNames& c_record_names) {
    std::map<std::string, CConcreteRecord> by_key;
    for (const auto& fn : program.functions) {
        if (!should_emit_function_prototype(fn)) continue;
        collect_concrete_record_type(fn.return_type, c_record_names, by_key);
        for (const auto& param : fn.params) {
            collect_concrete_record_type(param.type, c_record_names, by_key);
        }
    }

    std::vector<CConcreteRecord> out;
    out.reserve(by_key.size());
    for (auto& item : by_key) out.push_back(std::move(item.second));
    return out;
}

CConcreteRecordNames collect_concrete_record_names(const std::vector<CConcreteRecord>& records) {
    CConcreteRecordNames names;
    for (const auto& record : records) {
        names.emplace(record.key, record.c_name);
    }
    return names;
}

void collect_generated_aggregate_type(const IrType& type,
                                      const CEnumNames& c_enum_names,
                                      std::map<std::string, CGeneratedAggregate>& aggregates,
                                      std::vector<std::string>& order) {
    if (type.qualifier == TypeQualifier::Ptr ||
        type.qualifier == TypeQualifier::Ref ||
        type.qualifier == TypeQualifier::MutRef) {
        IrType pointee = type;
        pointee.qualifier = TypeQualifier::Value;
        if (is_c_aggregate_enum_type(pointee, c_enum_names)) {
            for (const auto& field_type : ari_aggregate_field_types(pointee)) {
                collect_generated_aggregate_type(field_type, c_enum_names, aggregates, order);
            }
            return;
        }
        if (is_generated_aggregate_type(pointee) && !is_value_array_type(pointee)) {
            collect_generated_aggregate_type(pointee, c_enum_names, aggregates, order);
        }
        return;
    }
    if (is_c_aggregate_enum_type(type, c_enum_names)) {
        for (const auto& field_type : ari_aggregate_field_types(type)) {
            collect_generated_aggregate_type(field_type, c_enum_names, aggregates, order);
        }
        return;
    }
    if (!is_generated_aggregate_type(type)) return;
    if (type.primitive == IrPrimitiveKind::Array && type.args.size() == 1) {
        collect_generated_aggregate_type(type.args[0], c_enum_names, aggregates, order);
    } else {
        for (const auto& field_type : ari_aggregate_field_types(type)) {
            collect_generated_aggregate_type(field_type, c_enum_names, aggregates, order);
        }
    }

    std::string key = generated_aggregate_key(type);
    if (aggregates.count(key)) return;
    aggregates.emplace(key, CGeneratedAggregate{
        key,
        c_generated_aggregate_c_name(type),
        type,
        generated_aggregate_kind(type)
    });
    order.push_back(key);
}

std::vector<CGeneratedAggregate> collect_generated_aggregates(const IrProgram& program,
                                                              const CEnumNames& c_enum_names) {
    std::map<std::string, CGeneratedAggregate> by_key;
    std::vector<std::string> order;
    for (const auto& item : program.c_enums) {
        if (!item.aggregate_layout) continue;
        for (const auto& field_type : ari_aggregate_field_types(item.type)) {
            collect_generated_aggregate_type(field_type, c_enum_names, by_key, order);
        }
    }
    for (const auto& fn : program.functions) {
        if (!should_emit_function_prototype(fn)) continue;
        collect_generated_aggregate_type(fn.return_type, c_enum_names, by_key, order);
        for (const auto& param : fn.params) {
            collect_generated_aggregate_type(param.type, c_enum_names, by_key, order);
        }
    }

    std::vector<CGeneratedAggregate> out;
    out.reserve(by_key.size());
    for (const auto& key : order) out.push_back(by_key.at(key));
    return out;
}

CGeneratedAggregateNames collect_generated_aggregate_names(
    const std::vector<CGeneratedAggregate>& aggregates) {
    CGeneratedAggregateNames names;
    for (const auto& aggregate : aggregates) {
        names.emplace(aggregate.key, aggregate.c_name);
    }
    return names;
}

void require_direct_aggregate_abi(SourceLocation loc,
                                  const IrType& type,
                                  const TargetInfo& target,
                                  const std::string& context) {
    NonLocalAggregateAbi abi = classify_nonlocal_aggregate_abi(type, target);
    if (abi.kind == NonLocalAggregateAbiKind::NotAggregate) return;
    if (abi.reason == NonLocalAggregateAbiReason::TargetUnsupported) {
        fail_c_header_abi(
            loc,
            "C header emission for by-value " + context + " " + type_name(type) +
                " is currently supported only on 64-bit Unix targets; use ptr " +
                pointer_abi_hint(type) + " for a stable ABI on target '" + target.triple + "'",
            "C header generation emits Ari's supported C ABI contract and cannot promise target-specific aggregate calling rules for this target",
            "pass ptr " + pointer_abi_hint(type) +
                " or emit the header for a supported 64-bit Unix target");
    }
    if (abi.reason == NonLocalAggregateAbiReason::LayoutUnavailable) {
        fail_c_header_abi(
            loc,
            "C header emission cannot compute layout for by-value " + context + " " +
                type_name(type) + "; use an explicit raw pointer ABI",
            "header emission needs a concrete Ari layout before it can spell a by-value C prototype",
            "expose a fixed-layout C wrapper or pass an explicit raw pointer ABI");
    }
    if (abi.reason == NonLocalAggregateAbiReason::ZeroSized) {
        fail_c_header_abi(
            loc,
            "C header emission does not support zero-sized by-value " + context + " " +
                type_name(type) + "; use ptr " + pointer_abi_hint(type),
            "zero-sized aggregate values do not have a stable C by-value representation",
            "pass ptr " + pointer_abi_hint(type) +
                " or keep the zero-sized value out of exported C prototypes");
    }
    if (abi.kind == NonLocalAggregateAbiKind::Direct) return;
    if (abi.kind == NonLocalAggregateAbiKind::Indirect) {
        fail_c_header_abi(
            loc,
            "C header emission for by-value " + context + " " + type_name(type) +
                " is limited to direct aggregate ABI values up to 16 bytes and 8-byte alignment; use ptr " +
                pointer_abi_hint(type),
            "this aggregate exceeds Ari's direct C header aggregate ABI subset." +
                c_header_aggregate_abi_layout_note(abi),
            "pass ptr " + pointer_abi_hint(type) +
                " or expose a smaller C wrapper type at the exported ABI boundary");
    }
}

void require_direct_function_abi(const IrFunction& fn, const TargetInfo& target) {
    require_direct_aggregate_abi(fn.loc, fn.return_type, target, "return type");
    for (const auto& param : fn.params) {
        require_direct_aggregate_abi(fn.loc, param.type, target, "parameter");
    }
}

void require_unique_generated_c_type_names(const IrProgram& program,
                                           const std::vector<CConcreteRecord>& concrete_records,
                                           const std::vector<CGeneratedAggregate>& generated_aggregates) {
    std::set<std::string> used_c_names;
    for (const auto& item : program.c_records) {
        std::string c_name = item.c_name.empty() ? unqualified_name(item.name) : item.c_name;
        used_c_names.insert(c_name);
    }
    for (const auto& item : program.c_enums) {
        std::string c_name = item.c_name.empty() ? unqualified_name(item.name) : item.c_name;
        used_c_names.insert(c_name);
    }
    for (const auto& item : concrete_records) {
        if (!used_c_names.insert(item.c_name).second) {
            throw CompileError("C header emission found duplicate C type name '" + item.c_name + "'");
        }
    }
    for (const auto& item : generated_aggregates) {
        if (!used_c_names.insert(item.c_name).second) {
            throw CompileError("C header emission found duplicate C type name '" + item.c_name + "'");
        }
    }
}

std::string enum_constants_definition(const IrCEnum& item) {
    std::string c_name = item.c_name.empty() ? unqualified_name(item.name) : item.c_name;
    std::ostringstream out;
    out << "enum {\n";
    for (const auto& enum_case : item.cases) {
        out << "    " << enum_case.c_name << " = " << enum_case.tag << ",\n";
    }
    out << "};";
    return out.str();
}

std::string fieldless_enum_definition(const IrCEnum& item) {
    std::string c_name = item.c_name.empty() ? unqualified_name(item.name) : item.c_name;
    std::ostringstream out;
    out << "typedef int64_t " << c_name << ";\n";
    out << enum_constants_definition(item);
    return out.str();
}

std::string record_forward_declaration(const IrCRecord& record) {
    std::string c_name = record.c_name.empty() ? unqualified_name(record.name) : record.c_name;
    return "typedef struct " + c_name + " " + c_name + ";";
}

std::string aggregate_enum_forward_declaration(const IrCEnum& item) {
    std::string c_name = item.c_name.empty() ? unqualified_name(item.name) : item.c_name;
    return "typedef struct " + c_name + " " + c_name + ";";
}

std::string concrete_record_forward_declaration(const CConcreteRecord& record) {
    return "typedef struct " + record.c_name + " " + record.c_name + ";";
}

std::string generated_aggregate_forward_declaration(const CGeneratedAggregate& aggregate) {
    return "typedef struct " + aggregate.c_name + " " + aggregate.c_name + ";";
}

std::string record_definition(const IrCRecord& record,
                              const CRecordNames& c_record_names,
                              const CConcreteRecordNames& c_concrete_record_names,
                              const CEnumNames& c_enum_names,
                              const CGeneratedAggregateNames& c_generated_aggregate_names) {
    std::string c_name = record.c_name.empty() ? unqualified_name(record.name) : record.c_name;
    std::ostringstream out;
    out << "struct " << c_name << " {\n";
    for (const auto& field : record.fields) {
        out << "    " << c_declaration(
            field.type,
            c_record_names,
            c_concrete_record_names,
            c_enum_names,
            c_generated_aggregate_names,
            field.name,
            false) << ";\n";
    }
    out << "};";
    return out.str();
}

std::string concrete_record_definition(const CConcreteRecord& record,
                                       const CRecordNames& c_record_names,
                                       const CConcreteRecordNames& c_concrete_record_names,
                                       const CEnumNames& c_enum_names,
                                       const CGeneratedAggregateNames& c_generated_aggregate_names) {
    std::ostringstream out;
    out << "struct " << record.c_name << " {\n";
    for (const auto& field : record.fields) {
        out << "    " << c_declaration(
            field.type,
            c_record_names,
            c_concrete_record_names,
            c_enum_names,
            c_generated_aggregate_names,
            field.name,
            false) << ";\n";
    }
    out << "};";
    return out.str();
}

std::string aggregate_enum_field_name(std::size_t index) {
    if (index == 0) return "tag";
    return "payload" + std::to_string(index - 1);
}

std::string aggregate_enum_definition(const IrCEnum& item,
                                      const CRecordNames& c_record_names,
                                      const CConcreteRecordNames& c_concrete_record_names,
                                      const CEnumNames& c_enum_names,
                                      const CGeneratedAggregateNames& c_generated_aggregate_names) {
    std::string c_name = item.c_name.empty() ? unqualified_name(item.name) : item.c_name;
    std::ostringstream out;
    out << "struct " << c_name << " {\n";
    const std::vector<IrType>& fields = ari_aggregate_field_types(item.type);
    for (std::size_t i = 0; i < fields.size(); ++i) {
        out << "    " << c_declaration(
            fields[i],
            c_record_names,
            c_concrete_record_names,
            c_enum_names,
            c_generated_aggregate_names,
            aggregate_enum_field_name(i),
            true) << ";\n";
    }
    out << "};";
    return out.str();
}

struct CDefinitionNode {
    std::string name;
    std::vector<std::string> dependencies;
    std::string definition;
    SourceLocation loc;
};

void add_definition_dependency(std::set<std::string>& seen,
                               std::vector<std::string>& dependencies,
                               const std::string& name) {
    if (name.empty()) return;
    if (!seen.insert(name).second) return;
    dependencies.push_back(name);
}

void collect_definition_dependencies_for_type(
    const IrType& type,
    const CRecordNames& c_record_names,
    const CConcreteRecordNames& c_concrete_record_names,
    const CEnumNames& c_enum_names,
    const CGeneratedAggregateNames& c_generated_aggregate_names,
    std::set<std::string>& seen,
    std::vector<std::string>& dependencies) {
    if (type.qualifier == TypeQualifier::Ptr ||
        type.qualifier == TypeQualifier::Ref ||
        type.qualifier == TypeQualifier::MutRef) {
        return;
    }
    if (type.qualifier == TypeQualifier::Own) {
        throw CompileError("C header emission does not support ownership-qualified type " + type_name(type) +
                           "; expose an explicit raw pointer or scalar ABI instead");
    }
    if (type.qualifier != TypeQualifier::Value) return;

    if (type.primitive == IrPrimitiveKind::Array) {
        if (type.args.size() != 1) {
            throw CompileError("C header emission cannot render malformed fixed-array type " +
                               type_name(type));
        }
        collect_definition_dependencies_for_type(
            type.args[0],
            c_record_names,
            c_concrete_record_names,
            c_enum_names,
            c_generated_aggregate_names,
            seen,
            dependencies);
        return;
    }

    if (type.primitive == IrPrimitiveKind::Struct) {
        const CRecordInfo& record = c_record_info(type, c_record_names);
        if (record.opaque) {
            auto concrete = c_concrete_record_names.find(concrete_record_key(type));
            if (concrete != c_concrete_record_names.end()) {
                add_definition_dependency(seen, dependencies, concrete->second);
                return;
            }
            throw CompileError("C header emission does not support by-value generic aggregate type " +
                               type_name(type) + "; no concrete C layout was collected for this instantiation");
        }
        add_definition_dependency(seen, dependencies, record.c_name);
        return;
    }

    if (type.primitive == IrPrimitiveKind::Enum) {
        const CEnumInfo* enum_info = c_enum_info_for_type(type, c_enum_names);
        if (enum_info) {
            if (enum_info->aggregate_layout) {
                add_definition_dependency(seen, dependencies, enum_info->c_name);
            }
            return;
        }
    }

    if (is_generated_aggregate_type(type) && !is_value_array_type(type)) {
        add_definition_dependency(
            seen,
            dependencies,
            c_generated_aggregate_type_name(type, c_generated_aggregate_names));
    }
}

std::vector<std::string> collect_definition_dependencies(
    const std::vector<IrType>& fields,
    const CRecordNames& c_record_names,
    const CConcreteRecordNames& c_concrete_record_names,
    const CEnumNames& c_enum_names,
    const CGeneratedAggregateNames& c_generated_aggregate_names,
    const std::string& self_name) {
    std::set<std::string> seen;
    std::vector<std::string> dependencies;
    for (const auto& field : fields) {
        collect_definition_dependencies_for_type(
            field,
            c_record_names,
            c_concrete_record_names,
            c_enum_names,
            c_generated_aggregate_names,
            seen,
            dependencies);
    }
    dependencies.erase(
        std::remove(dependencies.begin(), dependencies.end(), self_name),
        dependencies.end());
    return dependencies;
}

std::vector<std::string> record_definition_dependencies(
    const IrCRecord& record,
    const CRecordNames& c_record_names,
    const CConcreteRecordNames& c_concrete_record_names,
    const CEnumNames& c_enum_names,
    const CGeneratedAggregateNames& c_generated_aggregate_names,
    const std::string& self_name) {
    std::vector<IrType> fields;
    fields.reserve(record.fields.size());
    for (const auto& field : record.fields) fields.push_back(field.type);
    return collect_definition_dependencies(
        fields,
        c_record_names,
        c_concrete_record_names,
        c_enum_names,
        c_generated_aggregate_names,
        self_name);
}

std::vector<std::string> concrete_record_definition_dependencies(
    const CConcreteRecord& record,
    const CRecordNames& c_record_names,
    const CConcreteRecordNames& c_concrete_record_names,
    const CEnumNames& c_enum_names,
    const CGeneratedAggregateNames& c_generated_aggregate_names) {
    std::vector<IrType> fields;
    fields.reserve(record.fields.size());
    for (const auto& field : record.fields) fields.push_back(field.type);
    return collect_definition_dependencies(
        fields,
        c_record_names,
        c_concrete_record_names,
        c_enum_names,
        c_generated_aggregate_names,
        record.c_name);
}

std::vector<std::string> generated_aggregate_definition_dependencies(
    const CGeneratedAggregate& aggregate,
    const CRecordNames& c_record_names,
    const CConcreteRecordNames& c_concrete_record_names,
    const CEnumNames& c_enum_names,
    const CGeneratedAggregateNames& c_generated_aggregate_names) {
    std::vector<IrType> fields;
    if (aggregate.kind == CGeneratedAggregateKind::Array) {
        fields.push_back(aggregate.type);
    } else {
        fields = ari_aggregate_field_types(aggregate.type);
    }
    return collect_definition_dependencies(
        fields,
        c_record_names,
        c_concrete_record_names,
        c_enum_names,
        c_generated_aggregate_names,
        aggregate.c_name);
}

std::string generated_aggregate_field_name(const CGeneratedAggregate& aggregate, std::size_t index) {
    if (aggregate.kind == CGeneratedAggregateKind::Tuple) {
        return "field" + std::to_string(index);
    }
    if (aggregate.kind == CGeneratedAggregateKind::Vector) {
        if (index < aggregate.type.field_names.size() && !aggregate.type.field_names[index].empty()) {
            return aggregate.type.field_names[index];
        }
        return index == 0 ? "len" : "data";
    }
    if (aggregate.kind == CGeneratedAggregateKind::AggregateEnum) {
        if (index == 0) return "tag";
        return "payload" + std::to_string(index - 1);
    }
    return "field" + std::to_string(index);
}

std::string generated_aggregate_definition(const CGeneratedAggregate& aggregate,
                                           const CRecordNames& c_record_names,
                                           const CConcreteRecordNames& c_concrete_record_names,
                                           const CEnumNames& c_enum_names,
                                           const CGeneratedAggregateNames& c_generated_aggregate_names) {
    std::ostringstream out;
    out << "struct " << aggregate.c_name << " {\n";
    if (aggregate.kind == CGeneratedAggregateKind::Array) {
        out << "    " << c_declaration(
            aggregate.type,
            c_record_names,
            c_concrete_record_names,
            c_enum_names,
            c_generated_aggregate_names,
            "elements",
            true) << ";\n";
    } else {
        const std::vector<IrType>& fields = ari_aggregate_field_types(aggregate.type);
        for (std::size_t i = 0; i < fields.size(); ++i) {
            out << "    " << c_declaration(
                fields[i],
                c_record_names,
                c_concrete_record_names,
                c_enum_names,
                c_generated_aggregate_names,
                generated_aggregate_field_name(aggregate, i),
                true) << ";\n";
        }
    }
    out << "};";
    return out.str();
}

void add_definition_node(std::map<std::string, CDefinitionNode>& nodes,
                         std::vector<std::string>& order,
                         CDefinitionNode node) {
    std::string name = node.name;
    if (!nodes.emplace(name, std::move(node)).second) {
        throw CompileError("C header emission found duplicate C definition name '" + name + "'");
    }
    order.push_back(std::move(name));
}

void emit_ordered_definition(const std::string& name,
                             const std::map<std::string, CDefinitionNode>& nodes,
                             std::set<std::string>& visiting,
                             std::set<std::string>& emitted,
                             std::ostringstream& out) {
    if (emitted.count(name)) return;
    auto found = nodes.find(name);
    if (found == nodes.end()) return;
    if (!visiting.insert(name).second) {
        throw CompileError(where(found->second.loc) +
                           ": C header emission found recursive by-value C type dependency involving '" +
                           name + "'");
    }

    for (const auto& dependency : found->second.dependencies) {
        emit_ordered_definition(dependency, nodes, visiting, emitted, out);
    }

    visiting.erase(name);
    emitted.insert(name);
    out << found->second.definition << "\n\n";
}

void emit_ordered_definitions(const std::vector<std::string>& order,
                              const std::map<std::string, CDefinitionNode>& nodes,
                              std::ostringstream& out) {
    std::set<std::string> visiting;
    std::set<std::string> emitted;
    for (const auto& name : order) {
        emit_ordered_definition(name, nodes, visiting, emitted, out);
    }
}

} // namespace

std::string emit_c_header(const IrProgram& program) {
    require_unique_c_type_names(program);
    TargetInfo target = resolve_target_info(program.target_triple);
    CRecordNames c_record_names = collect_c_record_names(program);
    CEnumNames c_enum_names = collect_c_enum_names(program);
    std::vector<CConcreteRecord> c_concrete_records = collect_concrete_records(program, c_record_names);
    std::vector<CGeneratedAggregate> c_generated_aggregates =
        collect_generated_aggregates(program, c_enum_names);
    require_unique_generated_c_type_names(program, c_concrete_records, c_generated_aggregates);
    CConcreteRecordNames c_concrete_record_names = collect_concrete_record_names(c_concrete_records);
    CGeneratedAggregateNames c_generated_aggregate_names =
        collect_generated_aggregate_names(c_generated_aggregates);
    for (const auto& fn : program.functions) {
        if (!should_emit_function_prototype(fn)) continue;
        require_direct_function_abi(fn, target);
    }

    std::ostringstream out;
    out << "#pragma once\n\n";
    out << "#include <stdbool.h>\n";
    out << "#include <stddef.h>\n";
    out << "#include <stdint.h>\n\n";

    if (!program.c_enums.empty()) {
        for (const auto& item : program.c_enums) {
            if (!item.aggregate_layout) {
                out << fieldless_enum_definition(item) << "\n\n";
            }
        }
    }

    bool has_aggregate_c_enums = false;
    for (const auto& item : program.c_enums) {
        if (item.aggregate_layout) {
            has_aggregate_c_enums = true;
            break;
        }
    }

    if (has_aggregate_c_enums ||
        !program.c_records.empty() ||
        !c_concrete_records.empty() ||
        !c_generated_aggregates.empty()) {
        for (const auto& item : program.c_enums) {
            if (item.aggregate_layout) {
                out << aggregate_enum_forward_declaration(item) << "\n";
            }
        }
        for (const auto& record : program.c_records) {
            out << record_forward_declaration(record) << "\n";
        }
        for (const auto& record : c_concrete_records) {
            out << concrete_record_forward_declaration(record) << "\n";
        }
        for (const auto& aggregate : c_generated_aggregates) {
            out << generated_aggregate_forward_declaration(aggregate) << "\n";
        }
        out << "\n";
        std::map<std::string, CDefinitionNode> definition_nodes;
        std::vector<std::string> definition_order;
        for (const auto& item : program.c_enums) {
            if (!item.aggregate_layout) continue;
            std::string c_name = item.c_name.empty() ? unqualified_name(item.name) : item.c_name;
            add_definition_node(
                definition_nodes,
                definition_order,
                CDefinitionNode{
                    c_name,
                    collect_definition_dependencies(
                        ari_aggregate_field_types(item.type),
                        c_record_names,
                        c_concrete_record_names,
                        c_enum_names,
                        c_generated_aggregate_names,
                        c_name),
                    aggregate_enum_definition(
                        item,
                        c_record_names,
                        c_concrete_record_names,
                        c_enum_names,
                        c_generated_aggregate_names) + "\n\n" +
                        enum_constants_definition(item),
                    item.loc
                });
        }
        for (const auto& record : program.c_records) {
            if (record.opaque) continue;
            std::string c_name = record.c_name.empty() ? unqualified_name(record.name) : record.c_name;
            add_definition_node(
                definition_nodes,
                definition_order,
                CDefinitionNode{
                    c_name,
                    record_definition_dependencies(
                        record,
                        c_record_names,
                        c_concrete_record_names,
                        c_enum_names,
                        c_generated_aggregate_names,
                        c_name),
                    record_definition(
                        record,
                        c_record_names,
                        c_concrete_record_names,
                        c_enum_names,
                        c_generated_aggregate_names),
                    record.loc
                });
        }
        for (const auto& record : c_concrete_records) {
            add_definition_node(
                definition_nodes,
                definition_order,
                CDefinitionNode{
                    record.c_name,
                    concrete_record_definition_dependencies(
                        record,
                        c_record_names,
                        c_concrete_record_names,
                        c_enum_names,
                        c_generated_aggregate_names),
                    concrete_record_definition(
                        record,
                        c_record_names,
                        c_concrete_record_names,
                        c_enum_names,
                        c_generated_aggregate_names),
                    record.loc
                });
        }
        for (const auto& aggregate : c_generated_aggregates) {
            add_definition_node(
                definition_nodes,
                definition_order,
                CDefinitionNode{
                    aggregate.c_name,
                    generated_aggregate_definition_dependencies(
                        aggregate,
                        c_record_names,
                        c_concrete_record_names,
                        c_enum_names,
                        c_generated_aggregate_names),
                    generated_aggregate_definition(
                        aggregate,
                        c_record_names,
                        c_concrete_record_names,
                        c_enum_names,
                        c_generated_aggregate_names),
                    aggregate.type.loc
                });
        }
        emit_ordered_definitions(definition_order, definition_nodes, out);
    }

    out << "#ifdef __cplusplus\n";
    out << "extern \"C\" {\n";
    out << "#endif\n\n";

    for (const auto& fn : program.functions) {
        if (!should_emit_function_prototype(fn)) continue;
        out << function_prototype(
            fn,
            c_record_names,
            c_concrete_record_names,
            c_enum_names,
            c_generated_aggregate_names) << "\n";
    }

    out << "\n#ifdef __cplusplus\n";
    out << "}\n";
    out << "#endif\n";
    return out.str();
}

} // namespace ari
