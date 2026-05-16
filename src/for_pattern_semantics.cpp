#include "for_pattern_semantics.hpp"

#include "common.hpp"
#include "pattern_semantics.hpp"

#include <set>
#include <string>
#include <vector>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

[[noreturn]] void fail_refutable_for_pattern(SourceLocation loc) {
    fail(loc,
         "refutable for-loop patterns require for-let filters on Iterator[T]; non-iterator loop heads currently support irrefutable binding and alias patterns, with aggregate destructuring for list and vector items");
}

const std::vector<IrType>& aggregate_field_types(const IrType& type) {
    if (type.primitive == IrPrimitiveKind::Struct ||
        type.primitive == IrPrimitiveKind::Array) return type.field_types;
    return type.args;
}

void require_for_pattern_hooks(SourceLocation loc, const ForPatternValidationHooks& hooks) {
    if (!hooks.require_struct_pattern_type || !hooks.struct_field_index) {
        fail(loc, "internal error: incomplete for-loop pattern validation hooks");
    }
}

void require_tuple_pattern_arity(const Pattern& pattern,
                                 const IrType& source_type,
                                 const std::vector<IrType>& fields) {
    const char* pattern_name = pattern.kind == PatternKind::Array ? "array" : "tuple";
    if (!pattern.rest_alias_name.empty()) {
        fail(pattern.rest_alias_loc,
             std::string(pattern_name) +
                 " rest bindings currently require a Vec[T] or Slice[T] value");
    }
    if (pattern.has_rest) {
        if (pattern.elements.size() > fields.size()) {
            fail(pattern.loc,
                 std::string(pattern_name) + " match pattern has " +
                     std::to_string(pattern.elements.size()) +
                     " non-rest elements but value has " + std::to_string(fields.size()));
        }
        return;
    }
    if (pattern.elements.size() != fields.size()) {
        fail(pattern.loc,
             std::string(pattern_name) + " match pattern has " +
                 std::to_string(pattern.elements.size()) +
                 " elements but value has " + std::to_string(fields.size()));
    }
    (void)source_type;
}

void require_irrefutable_tuple_struct_for_pattern(const Pattern& pattern,
                                                  const IrType& value_type,
                                                  const ForPatternValidationHooks& hooks) {
    ForPatternStructInfo info = hooks.require_struct_pattern_type(pattern.loc, pattern.case_name, value_type);
    if (!info.tuple_struct) {
        fail(pattern.loc, "tuple-struct for-loop pattern requires a tuple struct");
    }
    if (!pattern.has_payload_pattern || !pattern.payload_pattern) {
        fail(pattern.loc, "tuple-struct for-loop pattern '" + pattern.case_name + "' requires positional fields");
    }

    const std::vector<IrType>& fields = aggregate_field_types(value_type);
    const Pattern& payload = *pattern.payload_pattern;
    if (payload.kind == PatternKind::Tuple) {
        require_tuple_pattern_arity(payload, value_type, fields);
        for (std::size_t i = 0; i < payload.elements.size(); ++i) {
            require_irrefutable_non_iterator_for_pattern(
                payload.elements[i],
                fields[tuple_pattern_field_index(payload, fields.size(), i)],
                hooks
            );
        }
        return;
    }
    if (fields.size() != 1) {
        fail(payload.loc,
             "tuple-struct for-loop pattern for '" + info.name + "' has 1 field but value has " +
                 std::to_string(fields.size()));
    }
    require_irrefutable_non_iterator_for_pattern(payload, fields[0], hooks);
}

} // namespace

void require_irrefutable_non_iterator_for_pattern(const Pattern& pattern,
                                                  const IrType& value_type,
                                                  const ForPatternValidationHooks& hooks) {
    require_for_pattern_hooks(pattern.loc, hooks);

    switch (pattern.kind) {
        case PatternKind::Wildcard:
        case PatternKind::Binding:
            return;
        case PatternKind::Tuple: {
            if (value_type.primitive != IrPrimitiveKind::Tuple) {
                fail(pattern.loc, "tuple for-loop pattern requires a tuple element, got " + type_name(value_type));
            }
            const std::vector<IrType>& fields = aggregate_field_types(value_type);
            require_tuple_pattern_arity(pattern, value_type, fields);
            for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
                require_irrefutable_non_iterator_for_pattern(
                    pattern.elements[i],
                    fields[tuple_pattern_field_index(pattern, fields.size(), i)],
                    hooks
                );
            }
            return;
        }
        case PatternKind::Array: {
            if (value_type.primitive != IrPrimitiveKind::Array) {
                fail(pattern.loc, "array for-loop pattern requires an array element, got " + type_name(value_type));
            }
            const std::vector<IrType>& fields = aggregate_field_types(value_type);
            require_tuple_pattern_arity(pattern, value_type, fields);
            for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
                require_irrefutable_non_iterator_for_pattern(
                    pattern.elements[i],
                    fields[tuple_pattern_field_index(pattern, fields.size(), i)],
                    hooks
                );
            }
            return;
        }
        case PatternKind::Struct: {
            hooks.require_struct_pattern_type(pattern.loc, pattern.case_name, value_type);
            if (pattern.field_names.size() != pattern.elements.size()) {
                throw CompileError("internal error: struct for-loop pattern field/value arity mismatch");
            }
            if (!pattern.has_rest && pattern.field_names.size() != value_type.field_names.size()) {
                fail(pattern.loc, "struct for-loop pattern must mention all fields or use '..'");
            }
            std::set<std::string> seen_fields;
            for (std::size_t i = 0; i < pattern.field_names.size(); ++i) {
                const std::string& field_name = pattern.field_names[i];
                if (!seen_fields.insert(field_name).second) {
                    fail(pattern.elements[i].loc, "duplicate field '" + field_name + "' in struct for-loop pattern");
                }
                std::size_t field_index = hooks.struct_field_index(pattern.elements[i].loc, value_type, field_name);
                require_irrefutable_non_iterator_for_pattern(pattern.elements[i], value_type.field_types[field_index], hooks);
            }
            return;
        }
        case PatternKind::EnumCase:
            if (value_type.primitive == IrPrimitiveKind::Struct) {
                require_irrefutable_tuple_struct_for_pattern(pattern, value_type, hooks);
                return;
            }
            fail(pattern.loc, "for enum-case patterns are planned for Iterator[T] values but are not supported yet");
            return;
        case PatternKind::Alias:
            if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased for-loop pattern");
            require_irrefutable_non_iterator_for_pattern(*pattern.alias_pattern, value_type, hooks);
            return;
        case PatternKind::IntegerLiteral:
        case PatternKind::BoolLiteral:
        case PatternKind::Range:
        case PatternKind::Or:
            fail_refutable_for_pattern(pattern.loc);
            return;
    }
}

} // namespace ari
