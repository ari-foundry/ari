#include "type_semantics.hpp"

#include "common.hpp"
#include "layout.hpp"

#include <cstddef>
#include <limits>
#include <utility>

namespace ari {
namespace {

IrType primitive_type(IrPrimitiveKind primitive, std::string name, SourceLocation loc) {
    IrType type;
    type.primitive = primitive;
    type.name = std::move(name);
    type.loc = loc;
    return type;
}

IrType integer_type(IrPrimitiveKind primitive, SourceLocation loc) {
    return primitive_type(primitive, primitive_name(primitive), loc);
}

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

} // namespace

bool same_type(const IrType& left, const IrType& right) {
    if (left.qualifier != right.qualifier) return false;
    if (left.primitive != right.primitive) return false;
    if (left.name != right.name) return false;
    if (left.array_size != right.array_size) return false;
    if (left.args.size() != right.args.size()) return false;
    for (std::size_t i = 0; i < left.args.size(); ++i) {
        if (!same_type(left.args[i], right.args[i])) return false;
    }
    return true;
}

bool is_copy_type(const IrType& type) {
    if (type.qualifier == TypeQualifier::Ref || type.qualifier == TypeQualifier::MutRef) return true;
    if (type.qualifier != TypeQualifier::Value) return false;
    return is_integer_primitive(type.primitive) ||
           is_float_primitive(type.primitive) ||
           type.primitive == IrPrimitiveKind::Bool ||
           type.primitive == IrPrimitiveKind::String ||
           type.primitive == IrPrimitiveKind::Enum ||
           type.primitive == IrPrimitiveKind::Function;
}

bool is_owner_type(const IrType& type) {
    if (type.qualifier == TypeQualifier::Own) return true;
    if (type.qualifier != TypeQualifier::Value) return false;
    for (const auto& arg : type.args) {
        if (is_owner_type(arg)) return true;
    }
    for (const auto& field : type.field_types) {
        if (is_owner_type(field)) return true;
    }
    return false;
}

bool is_borrow_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Ref || type.qualifier == TypeQualifier::MutRef;
}

bool contains_borrow_type(const IrType& type) {
    if (is_borrow_type(type)) return true;
    if (type.qualifier == TypeQualifier::Ptr) return false;
    for (const auto& arg : type.args) {
        if (contains_borrow_type(arg)) return true;
    }
    for (const auto& field : type.field_types) {
        if (contains_borrow_type(field)) return true;
    }
    return false;
}

bool is_aggregate_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           (type.primitive == IrPrimitiveKind::Tuple ||
            type.primitive == IrPrimitiveKind::Array ||
            type.primitive == IrPrimitiveKind::Struct);
}

bool is_value_integer_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value && is_integer_primitive(type.primitive);
}

bool is_value_enum_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Enum;
}

bool is_value_float_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value && is_float_primitive(type.primitive);
}

bool is_value_trait_object_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::TraitObject;
}

bool is_void_value_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Void;
}

bool is_raw_pointer_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Ptr;
}

bool is_raw_pointer_cast(const IrType& from, const IrType& to) {
    return (is_raw_pointer_type(from) && is_raw_pointer_type(to)) ||
           (is_raw_pointer_type(from) && is_value_integer_type(to)) ||
           (is_value_integer_type(from) && is_raw_pointer_type(to)) ||
           (from.qualifier == TypeQualifier::Value &&
            from.primitive == IrPrimitiveKind::String &&
            is_raw_pointer_type(to)) ||
           (is_borrow_type(from) && is_raw_pointer_type(to));
}

IrType raw_pointer_pointee_type(IrType type) {
    type.qualifier = TypeQualifier::Value;
    return type;
}

bool is_raw_memory_value_type(const IrType& type) {
    if (type.qualifier != TypeQualifier::Value) return false;
    if (is_integer_primitive(type.primitive) ||
        is_float_primitive(type.primitive) ||
        type.primitive == IrPrimitiveKind::Bool ||
        type.primitive == IrPrimitiveKind::String ||
        type.primitive == IrPrimitiveKind::Function) {
        return true;
    }
    return type.primitive == IrPrimitiveKind::Enum;
}

bool is_raw_pointer_deref_value_type(const IrType& type) {
    if (type.qualifier != TypeQualifier::Value) return false;
    return is_raw_memory_value_type(type) ||
           is_aggregate_type(type) ||
           has_aggregate_enum_layout(type);
}

bool is_integer_literal(const IrExpr& expr) {
    return expr.kind == IrExprKind::Integer;
}

bool is_float_literal(const IrExpr& expr) {
    return expr.kind == IrExprKind::Float;
}

bool is_null_literal(const IrExpr& expr) {
    return expr.kind == IrExprKind::Null;
}

bool is_integer_primitive(IrPrimitiveKind primitive) {
    switch (primitive) {
        case IrPrimitiveKind::I8:
        case IrPrimitiveKind::I16:
        case IrPrimitiveKind::I32:
        case IrPrimitiveKind::I64:
        case IrPrimitiveKind::U8:
        case IrPrimitiveKind::U16:
        case IrPrimitiveKind::U32:
        case IrPrimitiveKind::U64:
            return true;
        default:
            return false;
    }
}

bool is_owned_executable_primitive(IrPrimitiveKind primitive) {
    return is_integer_primitive(primitive) ||
           is_float_primitive(primitive) ||
           primitive == IrPrimitiveKind::Bool ||
           primitive == IrPrimitiveKind::String ||
           primitive == IrPrimitiveKind::Zone;
}

bool is_borrowable_executable_primitive(IrPrimitiveKind primitive) {
    return is_owned_executable_primitive(primitive) ||
           primitive == IrPrimitiveKind::Tuple ||
           primitive == IrPrimitiveKind::Array ||
           primitive == IrPrimitiveKind::Vector ||
           primitive == IrPrimitiveKind::Struct ||
           primitive == IrPrimitiveKind::Enum;
}

bool is_legacy_enum_payload_type(const IrType& type) {
    if (type.qualifier != TypeQualifier::Value) return false;
    if (type.primitive == IrPrimitiveKind::Bool) return true;
    switch (type.primitive) {
        case IrPrimitiveKind::I8:
        case IrPrimitiveKind::I16:
        case IrPrimitiveKind::I32:
        case IrPrimitiveKind::U8:
        case IrPrimitiveKind::U16:
        case IrPrimitiveKind::U32:
            return true;
        default:
            return false;
    }
}

bool is_aggregate_enum_payload_type(const IrType& type) {
    if (type.qualifier == TypeQualifier::Ptr) return true;
    if (type.qualifier != TypeQualifier::Value) return false;
    if (is_owner_type(type) || contains_borrow_type(type)) return false;
    if (type.primitive == IrPrimitiveKind::Bool) return true;
    if (is_integer_primitive(type.primitive)) return true;
    if (type.primitive == IrPrimitiveKind::String ||
        type.primitive == IrPrimitiveKind::Function) {
        return true;
    }
    return type.primitive == IrPrimitiveKind::Enum ||
           type.primitive == IrPrimitiveKind::Tuple ||
           type.primitive == IrPrimitiveKind::Array ||
           type.primitive == IrPrimitiveKind::Struct;
}

bool has_aggregate_enum_layout(const IrType& type) {
    return ari_has_aggregate_enum_layout(type);
}

bool is_float_primitive(IrPrimitiveKind primitive) {
    return primitive == IrPrimitiveKind::F32 ||
           primitive == IrPrimitiveKind::F64 ||
           primitive == IrPrimitiveKind::F128;
}

const char* primitive_name(IrPrimitiveKind primitive) {
    switch (primitive) {
        case IrPrimitiveKind::I8: return "i8";
        case IrPrimitiveKind::I16: return "i16";
        case IrPrimitiveKind::I32: return "i32";
        case IrPrimitiveKind::I64: return "i64";
        case IrPrimitiveKind::U8: return "u8";
        case IrPrimitiveKind::U16: return "u16";
        case IrPrimitiveKind::U32: return "u32";
        case IrPrimitiveKind::U64: return "u64";
        case IrPrimitiveKind::F32: return "f32";
        case IrPrimitiveKind::F64: return "f64";
        case IrPrimitiveKind::F128: return "f128";
        default: return "";
    }
}

std::string type_ref_key(const TypeRef& type) {
    std::string key;
    switch (type.qualifier) {
        case TypeQualifier::Value:
            break;
        case TypeQualifier::Own:
            key += "own ";
            break;
        case TypeQualifier::Ref:
            key += "ref ";
            break;
        case TypeQualifier::MutRef:
            key += "ref mut ";
            break;
        case TypeQualifier::Ptr:
            key += "ptr ";
            break;
    }

    if (type.name == "Array" && type.args.size() == 1) {
        key += "[" + type_ref_key(type.args[0]) + ", " + std::to_string(type.array_size) + "]";
        if (type.nullable) key += "?";
        return key;
    }

    if (type.is_dyn_object) {
        key += "dyn ";
        key += type.name;
        if (!type.args.empty()) {
            key += "[";
            for (std::size_t i = 0; i < type.args.size(); ++i) {
                if (i > 0) key += ", ";
                key += type_ref_key(type.args[i]);
            }
            key += "]";
        }
        if (type.nullable) key += "?";
        return key;
    }

    if (type.name == "fn" && !type.args.empty()) {
        key += "fn(";
        std::size_t param_count = static_cast<std::size_t>(type.array_size);
        if (param_count + 1 > type.args.size()) param_count = type.args.size() - 1;
        for (std::size_t i = 0; i < param_count; ++i) {
            if (i > 0) key += ", ";
            key += type_ref_key(type.args[i]);
        }
        key += ") -> ";
        key += type_ref_key(type.args[param_count]);
        if (type.nullable) key += "?";
        return key;
    }

    if (type.is_macro_invocation) {
        key += type.name;
        key += "!(...)";
        if (type.nullable) key += "?";
        return key;
    }

    if (type.name == "int") key += "i64";
    else if (type.name == "prelude::Vec") key += "Vec";
    else key += type.name;

    if (!type.args.empty()) {
        key += "[";
        for (std::size_t i = 0; i < type.args.size(); ++i) {
            if (i > 0) key += ", ";
            key += type_ref_key(type.args[i]);
        }
        key += "]";
    }
    if (type.has_associated_projection) {
        key += "::";
        key += type.associated_projection;
    }
    if (type.nullable) key += "?";
    return key;
}

IrType integer_literal_suffix_type(const std::string& suffix, SourceLocation loc) {
    if (suffix == "i8") return integer_type(IrPrimitiveKind::I8, loc);
    if (suffix == "i16") return integer_type(IrPrimitiveKind::I16, loc);
    if (suffix == "i32") return integer_type(IrPrimitiveKind::I32, loc);
    if (suffix == "i64") return integer_type(IrPrimitiveKind::I64, loc);
    if (suffix == "u8") return integer_type(IrPrimitiveKind::U8, loc);
    if (suffix == "u16") return integer_type(IrPrimitiveKind::U16, loc);
    if (suffix == "u32") return integer_type(IrPrimitiveKind::U32, loc);
    if (suffix == "u64") return integer_type(IrPrimitiveKind::U64, loc);
    fail(loc, "unsupported integer literal suffix '" + suffix + "'");
}

IrType float_literal_suffix_type(const std::string& suffix, SourceLocation loc) {
    if (suffix == "f32") return primitive_type(IrPrimitiveKind::F32, "f32", loc);
    if (suffix == "f64") return primitive_type(IrPrimitiveKind::F64, "f64", loc);
    if (suffix == "f128") return primitive_type(IrPrimitiveKind::F128, "f128", loc);
    fail(loc, "unsupported float literal suffix '" + suffix + "'");
}

bool is_signed_integer_primitive(IrPrimitiveKind primitive) {
    switch (primitive) {
        case IrPrimitiveKind::I8:
        case IrPrimitiveKind::I16:
        case IrPrimitiveKind::I32:
        case IrPrimitiveKind::I64:
            return true;
        default:
            return false;
    }
}

bool is_unsigned_integer_primitive(IrPrimitiveKind primitive) {
    switch (primitive) {
        case IrPrimitiveKind::U8:
        case IrPrimitiveKind::U16:
        case IrPrimitiveKind::U32:
        case IrPrimitiveKind::U64:
            return true;
        default:
            return false;
    }
}

unsigned integer_primitive_bit_width(IrPrimitiveKind primitive) {
    switch (primitive) {
        case IrPrimitiveKind::I8:
        case IrPrimitiveKind::U8:
            return 8;
        case IrPrimitiveKind::I16:
        case IrPrimitiveKind::U16:
            return 16;
        case IrPrimitiveKind::I32:
        case IrPrimitiveKind::U32:
            return 32;
        case IrPrimitiveKind::I64:
        case IrPrimitiveKind::U64:
            return 64;
        default:
            return 0;
    }
}

std::uint64_t signed_positive_max(IrPrimitiveKind primitive) {
    switch (primitive) {
        case IrPrimitiveKind::I8: return 127;
        case IrPrimitiveKind::I16: return 32767;
        case IrPrimitiveKind::I32: return 2147483647;
        case IrPrimitiveKind::I64: return std::numeric_limits<std::int64_t>::max();
        default: return 0;
    }
}

std::uint64_t signed_negative_limit(IrPrimitiveKind primitive) {
    switch (primitive) {
        case IrPrimitiveKind::I8: return 128;
        case IrPrimitiveKind::I16: return 32768;
        case IrPrimitiveKind::I32: return 2147483648ULL;
        case IrPrimitiveKind::I64: return 9223372036854775808ULL;
        default: return 0;
    }
}

std::uint64_t unsigned_max(IrPrimitiveKind primitive) {
    switch (primitive) {
        case IrPrimitiveKind::U8: return 255;
        case IrPrimitiveKind::U16: return 65535;
        case IrPrimitiveKind::U32: return 4294967295ULL;
        case IrPrimitiveKind::U64: return std::numeric_limits<std::uint64_t>::max();
        default: return 0;
    }
}

std::string integer_literal_name(const IrExpr& expr) {
    return (expr.int_negative ? "-" : "") + std::to_string(expr.int_value);
}

IrType literal_value_type_for_expected(const IrType& expected) {
    IrType type = expected;
    if (type.qualifier == TypeQualifier::Own) type.qualifier = TypeQualifier::Value;
    return type;
}

bool integer_literal_fits(const IrExpr& expr, const IrType& expected) {
    if (!is_integer_literal(expr)) return false;
    IrType target = literal_value_type_for_expected(expected);
    if (!is_value_integer_type(target)) return false;
    if (is_signed_integer_primitive(target.primitive)) {
        if (expr.int_negative) return expr.int_value <= signed_negative_limit(target.primitive);
        return expr.int_value <= signed_positive_max(target.primitive);
    }
    return !expr.int_negative && expr.int_value <= unsigned_max(target.primitive);
}

bool range_start_le_end(const Pattern& pattern, const IrType& match_type) {
    if (is_unsigned_integer_primitive(match_type.primitive)) {
        return pattern.int_value <= pattern.range_end_value;
    }
    if (pattern.int_negative != pattern.range_end_negative) {
        return pattern.int_negative;
    }
    if (pattern.int_negative) return pattern.int_value >= pattern.range_end_value;
    return pattern.int_value <= pattern.range_end_value;
}

IrType require_raw_pointer_memory_type(SourceLocation loc, const IrType& pointer_type, const std::string& operation) {
    if (!is_raw_pointer_type(pointer_type)) {
        fail(loc, operation + " expects a raw pointer, got " + type_name(pointer_type));
    }
    IrType element_type = raw_pointer_pointee_type(pointer_type);
    if (is_void_value_type(element_type)) {
        fail(loc, operation + " cannot access ptr void; cast to ptr T first");
    }
    if (!is_raw_memory_value_type(element_type)) {
        fail(loc, operation + " currently supports scalar pointer element types, got " + type_name(pointer_type));
    }
    return element_type;
}

IrType require_raw_pointer_deref_type(SourceLocation loc, const IrType& pointer_type, const std::string& operation) {
    if (!is_raw_pointer_type(pointer_type)) {
        fail(loc, operation + " expects a raw pointer, got " + type_name(pointer_type));
    }
    IrType element_type = raw_pointer_pointee_type(pointer_type);
    if (is_void_value_type(element_type)) {
        fail(loc, operation + " cannot access ptr void; cast to ptr T first");
    }
    if (!is_raw_pointer_deref_value_type(element_type)) {
        fail(loc, operation + " currently supports scalar or aggregate pointer element types, got " + type_name(pointer_type));
    }
    return element_type;
}

IrType require_raw_pointer_add_type(SourceLocation loc, const IrType& pointer_type, const std::string& operation) {
    if (!is_raw_pointer_type(pointer_type)) {
        fail(loc, operation + " expects a raw pointer, got " + type_name(pointer_type));
    }
    IrType element_type = raw_pointer_pointee_type(pointer_type);
    if (is_void_value_type(element_type)) {
        fail(loc, operation + " cannot scale ptr void; cast to ptr T first");
    }
    if (!is_raw_pointer_deref_value_type(element_type)) {
        fail(loc, operation + " currently supports scalar or aggregate pointer element types, got " + type_name(pointer_type));
    }
    return element_type;
}

IrType require_raw_pointer_materializable_type(SourceLocation loc,
                                               const IrType& pointer_type,
                                               const std::string& operation) {
    IrType element_type = require_raw_pointer_deref_type(loc, pointer_type, operation);
    if (is_owner_type(element_type) || contains_borrow_type(element_type)) {
        fail(loc, operation + " cannot copy ownership- or borrow-valued values through raw pointers yet");
    }
    return element_type;
}

void require_numeric_operands(SourceLocation loc, const IrType& left, const IrType& right) {
    if ((is_value_integer_type(left) || is_value_float_type(left)) && same_type(left, right)) return;
    fail(loc, "numeric operands must have the same numeric type, got " + type_name(left) + " and " + type_name(right));
}

void require_integer_operands(SourceLocation loc, const IrType& left, const IrType& right) {
    if (is_value_integer_type(left) && is_value_integer_type(right)) return;
    fail(loc, "bitwise and modulo operands must be integers, got " + type_name(left) + " and " + type_name(right));
}

void require_integer_shift_operands(SourceLocation loc, const IrType& left, const IrType& right) {
    if (is_value_integer_type(left) && is_value_integer_type(right)) return;
    fail(loc, "shift operands must be integers, got " + type_name(left) + " and " + type_name(right));
}

void require_comparable_operands(SourceLocation loc, const IrType& left, const IrType& right) {
    if ((has_aggregate_enum_layout(left) || has_aggregate_enum_layout(right)) && same_type(left, right)) {
        fail(loc, "comparison for aggregate enum layouts is planned but is not supported yet");
    }
    if (same_type(left, right) &&
        (is_value_integer_type(left) ||
         is_value_float_type(left) ||
         is_value_enum_type(left) ||
         (left.qualifier == TypeQualifier::Value && left.primitive == IrPrimitiveKind::Bool))) {
        return;
    }
    fail(loc, "comparison operands must have the same comparable type, got " + type_name(left) + " and " + type_name(right));
}

void require_logical_operand(SourceLocation loc, const IrType& type) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) return;
    fail(loc, "logical operand must be bool, got " + type_name(type));
}

void require_bitwise_not_operand(SourceLocation loc, const IrType& type) {
    if (is_value_integer_type(type)) return;
    fail(loc, "bitwise-not operand must be integer, got " + type_name(type));
}

void require_boolish(SourceLocation loc, const IrType& type) {
    if (type.qualifier != TypeQualifier::Value || type.primitive != IrPrimitiveKind::Bool) {
        fail(loc, "condition must be bool or integer-convertible, got " + type_name(type));
    }
}

void require_assignable(SourceLocation loc, const IrType& expected, const IrType& actual) {
    if (same_type(expected, actual)) return;
    if (expected.qualifier == TypeQualifier::Ptr &&
        actual.qualifier == TypeQualifier::Value &&
        actual.primitive == IrPrimitiveKind::String &&
        (expected.primitive == IrPrimitiveKind::I8 ||
         expected.primitive == IrPrimitiveKind::U8 ||
         expected.primitive == IrPrimitiveKind::Void)) {
        return;
    }
    if (expected.qualifier == TypeQualifier::Own &&
        actual.qualifier == TypeQualifier::Value &&
        expected.primitive == actual.primitive &&
        expected.name == actual.name &&
        expected.args.empty() &&
        actual.args.empty()) {
        return;
    }
    fail(loc, "type mismatch: expected " + type_name(expected) + ", got " + type_name(actual));
}

} // namespace ari
