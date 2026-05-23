#include "type_inference.hpp"

#include "common.hpp"
#include "type_semantics.hpp"

#include <algorithm>
#include <cstddef>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(std::move(loc), message);
}

bool valid_label_span(SourceLocation loc) {
    Span span = span_from_location(loc);
    return span_has_source(span) && span_has_valid_order(span);
}

[[noreturn]] void fail_generic_inference_conflict(SourceLocation loc,
                                                  const std::string& name,
                                                  const IrType& first,
                                                  const IrType& second) {
    std::string first_type = type_name(first);
    std::string second_type = type_name(second);
    CompileError error(
        std::move(loc),
        "generic type '" + name + "' inferred as both " + first_type + " and " + second_type);
    if (valid_label_span(first.loc)) {
        error.add_label(DiagnosticLabel{
            span_from_location(first.loc),
            "first inferred '" + name + "' as " + first_type,
            false});
    }
    error.add_note(DiagnosticNote{
        std::nullopt,
        "generic type inference must choose one concrete type for '" + name + "'",
        DiagnosticNoteKind::Note});
    error.add_note(DiagnosticNote{
        std::nullopt,
        "make every use of '" + name + "' agree, or pass explicit type arguments",
        DiagnosticNoteKind::Help});
    throw error;
}

bool is_generic_type_name(const std::vector<GenericParam>& generics, const std::string& name) {
    for (const auto& generic : generics) {
        if (generic.name == name) return true;
    }
    return false;
}

bool is_named_generic_type(const std::vector<std::string>& generic_names, const std::string& name) {
    return std::find(generic_names.begin(), generic_names.end(), name) != generic_names.end();
}

} // namespace

void bind_generic_type(SourceLocation loc,
                       const std::string& name,
                       const IrType& binding,
                       std::map<std::string, IrType>& substitutions) {
    auto found = substitutions.find(name);
    if (found == substitutions.end()) {
        substitutions.emplace(name, binding);
        return;
    }
    if (!same_type_or_char_u8_boundary(found->second, binding)) {
        fail_generic_inference_conflict(loc, name, found->second, binding);
    }
}

namespace {

bool try_bind_inferred_type(const std::string& name,
                            const IrType& binding,
                            std::map<std::string, IrType>& substitutions) {
    auto found = substitutions.find(name);
    if (found == substitutions.end()) {
        substitutions.emplace(name, binding);
        return true;
    }
    return same_type_or_char_u8_boundary(found->second, binding);
}

} // namespace

bool infer_generic_pattern_type(const IrType& pattern,
                                const IrType& actual,
                                const std::vector<std::string>& generic_names,
                                std::map<std::string, IrType>& substitutions) {
    if (pattern.primitive == IrPrimitiveKind::Unknown &&
        pattern.args.empty() &&
        is_named_generic_type(generic_names, pattern.name)) {
        IrType binding = actual;
        binding.qualifier = TypeQualifier::Value;
        return try_bind_inferred_type(pattern.name, binding, substitutions);
    }

    if (pattern.qualifier != actual.qualifier ||
        pattern.primitive != actual.primitive ||
        pattern.name != actual.name ||
        pattern.array_size != actual.array_size ||
        pattern.args.size() != actual.args.size()) {
        return false;
    }

    for (std::size_t i = 0; i < pattern.args.size(); ++i) {
        if (!infer_generic_pattern_type(pattern.args[i], actual.args[i], generic_names, substitutions)) {
            return false;
        }
    }
    return true;
}

IrType substitute_inferred_type(const IrType& type,
                                const std::map<std::string, IrType>& substitutions) {
    if (type.primitive == IrPrimitiveKind::Unknown && type.args.empty()) {
        auto found = substitutions.find(type.name);
        if (found != substitutions.end()) {
            IrType resolved = found->second;
            resolved.qualifier = type.qualifier;
            resolved.loc = type.loc;
            return resolved;
        }
    }

    IrType resolved = type;
    for (auto& arg : resolved.args) arg = substitute_inferred_type(arg, substitutions);
    for (auto& field_type : resolved.field_types) {
        field_type = substitute_inferred_type(field_type, substitutions);
    }
    return resolved;
}

void infer_named_generic_type(SourceLocation loc,
                              const TypeRef& expected,
                              const IrType& actual,
                              const std::vector<std::string>& generic_names,
                              std::map<std::string, IrType>& substitutions) {
    if (expected.args.empty() && is_named_generic_type(generic_names, expected.name)) {
        if (expected.qualifier != TypeQualifier::Value && expected.qualifier != actual.qualifier) {
            fail(loc, "type mismatch: expected " + type_ref_key(expected) + ", got " + type_name(actual));
        }
        IrType binding = actual;
        binding.qualifier = TypeQualifier::Value;
        bind_generic_type(loc, expected.name, binding, substitutions);
        return;
    }
    if (expected.args.size() != actual.args.size()) return;
    for (std::size_t i = 0; i < expected.args.size(); ++i) {
        infer_named_generic_type(loc, expected.args[i], actual.args[i], generic_names, substitutions);
    }
}

void infer_generic_type(SourceLocation loc,
                        const TypeRef& expected,
                        const IrType& actual,
                        const std::vector<GenericParam>& generics,
                        std::map<std::string, IrType>& substitutions) {
    if (expected.args.empty() && is_generic_type_name(generics, expected.name)) {
        if (expected.qualifier != TypeQualifier::Value && expected.qualifier != actual.qualifier) {
            fail(loc, "type mismatch: expected " + type_ref_key(expected) + ", got " + type_name(actual));
        }
        IrType binding = actual;
        binding.qualifier = TypeQualifier::Value;
        bind_generic_type(loc, expected.name, binding, substitutions);
        return;
    }
    if (expected.args.size() != actual.args.size()) return;
    for (std::size_t i = 0; i < expected.args.size(); ++i) {
        infer_generic_type(loc, expected.args[i], actual.args[i], generics, substitutions);
    }
}

} // namespace ari
