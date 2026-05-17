#include "module_ir_replay.hpp"

#include "common.hpp"
#include "type_semantics.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ari {
namespace {

SourceLocation replay_loc() {
    return SourceLocation{1, 1};
}

std::uint32_t to_u32(std::uint64_t value, const std::string& label) {
    if (value > std::numeric_limits<std::uint32_t>::max()) {
        throw CompileError("IR summary " + label + " exceeds u32");
    }
    return static_cast<std::uint32_t>(value);
}

bool starts_with(const std::string& text, std::size_t pos, const std::string& prefix) {
    return text.compare(pos, prefix.size(), prefix) == 0;
}

std::string trim(std::string text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.erase(text.begin());
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.pop_back();
    }
    return text;
}

bool is_qualified_name(const std::string& name) {
    return name.find("::") != std::string::npos;
}

std::string qualify_decl_name(const std::string& module_name, const std::string& name) {
    if (module_name.empty() || is_qualified_name(name)) return name;
    return module_name + "::" + name;
}

std::string short_name(const std::string& name) {
    std::size_t pos = name.rfind("::");
    if (pos == std::string::npos) return name;
    return name.substr(pos + 2);
}

struct ReplayTypeContext {
    struct StructShape {
        std::vector<std::string> generic_names;
        std::vector<StructField> fields;
    };

    struct EnumShape {
        std::vector<std::string> generic_names;
        std::vector<EnumCase> cases;
    };

    std::set<std::string> structs;
    std::set<std::string> enums;
    std::set<std::string> traits;
    std::map<std::string, StructShape> struct_shapes;
    std::map<std::string, EnumShape> enum_shapes;
};

template <typename Shape>
const Shape* find_shape_by_name(const std::map<std::string, Shape>& shapes, const std::string& name) {
    auto exact = shapes.find(name);
    if (exact != shapes.end()) return &exact->second;

    std::string short_form = short_name(name);
    auto short_found = shapes.find(short_form);
    if (short_found != shapes.end()) return &short_found->second;

    const Shape* candidate = nullptr;
    std::string suffix = "::" + short_form;
    for (const auto& item : shapes) {
        if (item.first.size() >= suffix.size() &&
            item.first.compare(item.first.size() - suffix.size(), suffix.size(), suffix) == 0) {
            if (candidate) return nullptr;
            candidate = &item.second;
        }
    }
    return candidate;
}

ReplayTypeContext make_type_context(const Program& program) {
    ReplayTypeContext context;
    for (const auto& decl : program.structs) {
        context.structs.insert(decl.name);
        ReplayTypeContext::StructShape shape;
        shape.fields = decl.fields;
        for (const auto& generic : decl.generics) shape.generic_names.push_back(generic.name);
        std::string qualified = qualify_decl_name(decl.module_name, decl.name);
        context.structs.insert(qualified);
        context.structs.insert(short_name(decl.name));
        context.struct_shapes.emplace(decl.name, shape);
        context.struct_shapes.emplace(qualified, shape);
        context.struct_shapes.emplace(short_name(decl.name), std::move(shape));
    }
    for (const auto& decl : program.enums) {
        context.enums.insert(decl.name);
        ReplayTypeContext::EnumShape shape;
        shape.cases = decl.cases;
        for (const auto& generic : decl.generics) shape.generic_names.push_back(generic.name);
        std::string qualified = qualify_decl_name(decl.module_name, decl.name);
        context.enums.insert(qualified);
        context.enums.insert(short_name(decl.name));
        context.enum_shapes.emplace(decl.name, shape);
        context.enum_shapes.emplace(qualified, shape);
        context.enum_shapes.emplace(short_name(decl.name), std::move(shape));
    }
    for (const auto& decl : program.traits) context.traits.insert(decl.name);
    return context;
}

IrType replay_type_ref(const TypeRef& ref,
                       const ReplayTypeContext& context,
                       const std::map<std::string, IrType>& substitutions);

IrType replay_primitive_type(IrPrimitiveKind primitive, std::string name, SourceLocation loc = replay_loc()) {
    IrType type;
    type.qualifier = TypeQualifier::Value;
    type.primitive = primitive;
    type.name = std::move(name);
    type.loc = loc;
    return type;
}

IrType replay_integer_type(IrPrimitiveKind primitive, SourceLocation loc = replay_loc()) {
    return replay_primitive_type(primitive, primitive_name(primitive), loc);
}

IrType enum_tag_storage_type(SourceLocation loc = replay_loc()) {
    return replay_integer_type(IrPrimitiveKind::I32, loc);
}

IrType enum_payload_storage_type(SourceLocation loc = replay_loc()) {
    return replay_integer_type(IrPrimitiveKind::U64, loc);
}

IrType enum_payload_slot_storage_type(SourceLocation loc, const IrType& payload_type) {
    if (has_aggregate_enum_layout(payload_type)) return payload_type;
    return enum_payload_storage_type(loc);
}

void apply_aggregate_shape(IrType& type, const ReplayTypeContext& context) {
    if (type.primitive != IrPrimitiveKind::Struct) return;
    const auto* found = find_shape_by_name(context.struct_shapes, type.name);
    if (!found) return;

    std::map<std::string, IrType> substitutions;
    const auto& shape = *found;
    for (std::size_t i = 0; i < shape.generic_names.size() && i < type.args.size(); ++i) {
        substitutions.emplace(shape.generic_names[i], type.args[i]);
    }

    type.field_names.clear();
    type.field_types.clear();
    type.field_mutable.clear();
    for (const auto& field : shape.fields) {
        type.field_names.push_back(field.name);
        type.field_types.push_back(replay_type_ref(field.type, context, substitutions));
        type.field_mutable.push_back(field.mutable_field);
    }
}

void apply_enum_shape(IrType& type, const ReplayTypeContext& context) {
    if (type.qualifier != TypeQualifier::Value || type.primitive != IrPrimitiveKind::Enum) return;
    const auto* found = find_shape_by_name(context.enum_shapes, type.name);
    if (!found) return;

    const auto& shape = *found;
    std::map<std::string, IrType> substitutions;
    for (std::size_t i = 0; i < shape.generic_names.size() && i < type.args.size(); ++i) {
        substitutions.emplace(shape.generic_names[i], type.args[i]);
    }

    bool aggregate_layout = false;
    std::size_t max_payloads = 0;
    std::vector<IrType> payload_slot_types;
    std::vector<bool> payload_slot_set;

    for (const auto& item : shape.cases) {
        if (item.payloads.size() > 1) aggregate_layout = true;
        max_payloads = std::max(max_payloads, item.payloads.size());
        if (payload_slot_types.size() < item.payloads.size()) {
            payload_slot_types.resize(item.payloads.size());
            payload_slot_set.resize(item.payloads.size(), false);
        }

        for (std::size_t i = 0; i < item.payloads.size(); ++i) {
            IrType payload = replay_type_ref(item.payloads[i], context, substitutions);
            bool unresolved_generic_payload =
                payload.qualifier == TypeQualifier::Value &&
                payload.primitive == IrPrimitiveKind::Unknown &&
                payload.args.empty();
            bool payload_needs_aggregate =
                !is_legacy_enum_payload_type(payload) || item.payloads.size() > 1;
            if (payload_needs_aggregate) aggregate_layout = true;
            if (payload_needs_aggregate &&
                !unresolved_generic_payload &&
                !is_aggregate_enum_payload_type(payload)) {
                throw CompileError("IR summary enum type replay cannot restore aggregate payload layout for " +
                                   type.name + " payload " + type_name(payload));
            }

            IrType slot_type = enum_payload_slot_storage_type(item.payloads[i].loc, payload);
            if (!payload_slot_set[i]) {
                payload_slot_types[i] = std::move(slot_type);
                payload_slot_set[i] = true;
            } else if (!same_type(payload_slot_types[i], slot_type)) {
                throw CompileError("IR summary enum type replay found mixed payload slot storage in " + type.name);
            }
        }
    }

    if (!aggregate_layout) return;

    type.field_names.clear();
    type.field_types.clear();
    type.field_mutable.clear();
    type.field_types.push_back(enum_tag_storage_type(type.loc));
    type.field_names.push_back("$tag");
    type.field_mutable.push_back(false);
    for (std::size_t i = 0; i < max_payloads; ++i) {
        type.field_types.push_back(payload_slot_set[i] ? payload_slot_types[i] : enum_payload_storage_type(type.loc));
        type.field_names.push_back("$payload" + std::to_string(i));
        type.field_mutable.push_back(false);
    }
}

void apply_array_shape(IrType& type) {
    if (type.primitive != IrPrimitiveKind::Array || type.args.size() != 1 || !type.field_types.empty()) return;
    type.field_types.reserve(static_cast<std::size_t>(type.array_size));
    type.field_mutable.reserve(static_cast<std::size_t>(type.array_size));
    for (std::uint64_t i = 0; i < type.array_size; ++i) {
        type.field_types.push_back(type.args[0]);
        type.field_mutable.push_back(false);
    }
}

IrType replay_type_ref(const TypeRef& ref,
                       const ReplayTypeContext& context,
                       const std::map<std::string, IrType>& substitutions) {
    auto substitution = substitutions.find(ref.name);
    if (substitution != substitutions.end() && ref.args.empty() && !ref.is_dyn_object) {
        IrType type = substitution->second;
        type.qualifier = ref.qualifier;
        type.loc = replay_loc();
        return type;
    }

    IrType type;
    type.qualifier = ref.qualifier;
    type.name = ref.name;
    type.loc = replay_loc();

    auto primitive = [&](IrPrimitiveKind kind, const std::string& name) {
        if (ref.name == name) {
            type.primitive = kind;
            type.name = name;
            return true;
        }
        return false;
    };

    if (ref.is_dyn_object) {
        type.primitive = IrPrimitiveKind::TraitObject;
    } else if (primitive(IrPrimitiveKind::Void, "void") ||
               primitive(IrPrimitiveKind::I8, "i8") ||
               primitive(IrPrimitiveKind::I16, "i16") ||
               primitive(IrPrimitiveKind::I32, "i32") ||
               primitive(IrPrimitiveKind::I64, "i64") ||
               primitive(IrPrimitiveKind::U8, "u8") ||
               primitive(IrPrimitiveKind::U16, "u16") ||
               primitive(IrPrimitiveKind::U32, "u32") ||
               primitive(IrPrimitiveKind::U64, "u64") ||
               primitive(IrPrimitiveKind::F32, "f32") ||
               primitive(IrPrimitiveKind::F64, "f64") ||
               primitive(IrPrimitiveKind::F128, "f128") ||
               primitive(IrPrimitiveKind::Bool, "bool") ||
               primitive(IrPrimitiveKind::String, "string") ||
               primitive(IrPrimitiveKind::Zone, "Zone") ||
               primitive(IrPrimitiveKind::MetaType, "type")) {
    } else if (ref.name == "Vec") {
        type.primitive = IrPrimitiveKind::Vector;
    } else if (context.enums.count(ref.name)) {
        type.primitive = IrPrimitiveKind::Enum;
    } else if (context.structs.count(ref.name)) {
        type.primitive = IrPrimitiveKind::Struct;
    } else {
        type.primitive = IrPrimitiveKind::Unknown;
    }

    for (const auto& arg : ref.args) {
        type.args.push_back(replay_type_ref(arg, context, substitutions));
    }
    type.array_size = ref.array_size;
    apply_aggregate_shape(type, context);
    apply_enum_shape(type, context);
    apply_array_shape(type);
    return type;
}

class TypeNameParser {
public:
    TypeNameParser(std::string text, const ReplayTypeContext& context)
        : text_(std::move(text)), context_(context) {}

    IrType parse_full() {
        skip_spaces();
        if (text_.empty()) return unknown_type("");
        IrType type = parse_type();
        skip_spaces();
        if (pos_ != text_.size()) fail("unexpected trailing type text");
        return type;
    }

private:
    std::string text_;
    const ReplayTypeContext& context_;
    std::size_t pos_ = 0;

    [[noreturn]] void fail(const std::string& message) const {
        throw CompileError("IR summary type replay failed for '" + text_ + "': " + message);
    }

    static IrType primitive(IrPrimitiveKind kind, std::string name) {
        IrType type;
        type.primitive = kind;
        type.name = std::move(name);
        type.loc = replay_loc();
        return type;
    }

    static IrType unknown_type(std::string name) {
        IrType type;
        type.primitive = IrPrimitiveKind::Unknown;
        type.name = std::move(name);
        type.loc = replay_loc();
        return type;
    }

    void skip_spaces() {
        while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) ++pos_;
    }

    bool consume(const std::string& token) {
        skip_spaces();
        if (!starts_with(text_, pos_, token)) return false;
        pos_ += token.size();
        return true;
    }

    void expect(char c) {
        skip_spaces();
        if (pos_ >= text_.size() || text_[pos_] != c) {
            fail(std::string("expected '") + c + "'");
        }
        ++pos_;
    }

    std::string read_name() {
        skip_spaces();
        std::size_t start = pos_;
        while (pos_ < text_.size()) {
            char c = text_[pos_];
            if (c == '[' || c == ']' || c == '(' || c == ')' || c == ',' || c == ';') break;
            if (starts_with(text_, pos_, "->")) break;
            ++pos_;
        }
        return trim(text_.substr(start, pos_ - start));
    }

    std::vector<IrType> parse_bracket_args() {
        std::vector<IrType> args;
        expect('[');
        skip_spaces();
        if (pos_ < text_.size() && text_[pos_] == ']') {
            ++pos_;
            return args;
        }
        while (true) {
            args.push_back(parse_type());
            skip_spaces();
            if (pos_ < text_.size() && text_[pos_] == ',') {
                ++pos_;
                continue;
            }
            expect(']');
            return args;
        }
    }

    std::uint64_t parse_array_size() {
        skip_spaces();
        if (pos_ >= text_.size() || !std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
            fail("expected array size");
        }
        std::uint64_t value = 0;
        while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
            std::uint64_t digit = static_cast<std::uint64_t>(text_[pos_] - '0');
            if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10) {
                fail("array size is too large");
            }
            value = value * 10 + digit;
            ++pos_;
        }
        return value;
    }

    IrType parse_type() {
        skip_spaces();
        TypeQualifier qualifier = TypeQualifier::Value;
        if (consume("ref mut ")) {
            qualifier = TypeQualifier::MutRef;
        } else if (consume("own ")) {
            qualifier = TypeQualifier::Own;
        } else if (consume("ref ")) {
            qualifier = TypeQualifier::Ref;
        } else if (consume("ptr ")) {
            qualifier = TypeQualifier::Ptr;
        }

        IrType type = parse_unqualified_type();
        type.qualifier = qualifier;
        type.loc = replay_loc();
        return type;
    }

    IrType parse_unqualified_type() {
        skip_spaces();
        if (pos_ >= text_.size()) return unknown_type("");

        if (consume("fn(")) return parse_function_type();
        if (consume("dyn ")) return parse_trait_object_type();
        if (pos_ < text_.size() && text_[pos_] == '(') return parse_tuple_type();
        if (pos_ < text_.size() && text_[pos_] == '[') return parse_array_type();

        std::string name = read_name();
        if (name.empty()) fail("expected type name");
        if (name == "<unknown>") return unknown_type("");
        if (name == "void") return primitive(IrPrimitiveKind::Void, "void");
        if (name == "i8") return primitive(IrPrimitiveKind::I8, "i8");
        if (name == "i16") return primitive(IrPrimitiveKind::I16, "i16");
        if (name == "i32") return primitive(IrPrimitiveKind::I32, "i32");
        if (name == "i64") return primitive(IrPrimitiveKind::I64, "i64");
        if (name == "u8") return primitive(IrPrimitiveKind::U8, "u8");
        if (name == "u16") return primitive(IrPrimitiveKind::U16, "u16");
        if (name == "u32") return primitive(IrPrimitiveKind::U32, "u32");
        if (name == "u64") return primitive(IrPrimitiveKind::U64, "u64");
        if (name == "f32") return primitive(IrPrimitiveKind::F32, "f32");
        if (name == "f64") return primitive(IrPrimitiveKind::F64, "f64");
        if (name == "f128") return primitive(IrPrimitiveKind::F128, "f128");
        if (name == "bool") return primitive(IrPrimitiveKind::Bool, "bool");
        if (name == "string") return primitive(IrPrimitiveKind::String, "string");
        if (name == "Zone") return primitive(IrPrimitiveKind::Zone, "Zone");
        if (name == "type") return primitive(IrPrimitiveKind::MetaType, "type");

        IrType type;
        type.name = name;
        type.loc = replay_loc();
        if (name == "Vec") {
            type.primitive = IrPrimitiveKind::Vector;
        } else if (context_.enums.count(name)) {
            type.primitive = IrPrimitiveKind::Enum;
        } else if (context_.structs.count(name)) {
            type.primitive = IrPrimitiveKind::Struct;
        } else {
            type.primitive = IrPrimitiveKind::Unknown;
        }
        skip_spaces();
        if (name == "Vec" && pos_ < text_.size() && text_[pos_] == '[') {
            parse_vector_type_args(type);
        } else if (pos_ < text_.size() && text_[pos_] == '[') {
            type.args = parse_bracket_args();
        }
        apply_aggregate_shape(type, context_);
        apply_enum_shape(type, context_);
        return type;
    }

    IrType parse_function_type() {
        IrType type;
        type.primitive = IrPrimitiveKind::Function;
        type.name = "fn";
        type.loc = replay_loc();
        std::vector<IrType> params;
        skip_spaces();
        if (pos_ < text_.size() && text_[pos_] != ')') {
            while (true) {
                params.push_back(parse_type());
                skip_spaces();
                if (pos_ < text_.size() && text_[pos_] == ',') {
                    ++pos_;
                    continue;
                }
                break;
            }
        }
        expect(')');
        if (!consume("->")) fail("expected function result arrow");
        type.array_size = params.size();
        type.args = std::move(params);
        type.args.push_back(parse_type());
        return type;
    }

    IrType parse_trait_object_type() {
        IrType type;
        type.primitive = IrPrimitiveKind::TraitObject;
        type.name = read_name();
        type.loc = replay_loc();
        if (type.name.empty()) fail("expected trait object name");
        skip_spaces();
        if (pos_ < text_.size() && text_[pos_] == '[') {
            type.args = parse_bracket_args();
        }
        return type;
    }

    IrType parse_tuple_type() {
        IrType type;
        type.primitive = IrPrimitiveKind::Tuple;
        type.loc = replay_loc();
        expect('(');
        skip_spaces();
        if (pos_ < text_.size() && text_[pos_] == ')') {
            ++pos_;
            return type;
        }
        while (true) {
            type.args.push_back(parse_type());
            skip_spaces();
            if (pos_ < text_.size() && text_[pos_] == ',') {
                ++pos_;
                continue;
            }
            expect(')');
            return type;
        }
    }

    IrType parse_array_type() {
        IrType type;
        type.primitive = IrPrimitiveKind::Array;
        type.loc = replay_loc();
        expect('[');
        type.args.push_back(parse_type());
        expect(',');
        type.array_size = parse_array_size();
        expect(']');
        apply_array_shape(type);
        return type;
    }

    void parse_vector_type_args(IrType& type) {
        expect('[');
        skip_spaces();
        if (pos_ < text_.size() && text_[pos_] == ']') {
            ++pos_;
            return;
        }
        type.args.push_back(parse_type());
        skip_spaces();
        if (pos_ < text_.size() && text_[pos_] == ';') {
            ++pos_;
            type.array_size = parse_array_size();
            expect(']');
            return;
        }
        expect(']');
    }
};

IrType replay_type(const std::string& text, const ReplayTypeContext& context) {
    return TypeNameParser(text, context).parse_full();
}

struct SignedInteger {
    bool negative = false;
    std::uint64_t value = 0;
};

SignedInteger parse_signed_integer(const std::string& text, const std::string& label) {
    if (text.empty()) return {};
    SignedInteger result;
    std::size_t pos = 0;
    if (text[pos] == '-') {
        result.negative = true;
        ++pos;
    }
    if (pos >= text.size()) throw CompileError("IR summary expected " + label + " integer");
    for (; pos < text.size(); ++pos) {
        char c = text[pos];
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            throw CompileError("IR summary expected " + label + " integer");
        }
        std::uint64_t digit = static_cast<std::uint64_t>(c - '0');
        if (result.value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10) {
            throw CompileError("IR summary " + label + " integer is too large");
        }
        result.value = result.value * 10 + digit;
    }
    return result;
}

bool parse_bool_text(const std::string& text, const std::string& label) {
    if (text == "0") return false;
    if (text == "1") return true;
    throw CompileError("IR summary expected " + label + " boolean 0 or 1");
}

IrUnaryOp replay_unary_op(const std::string& op) {
    if (op == "not") return IrUnaryOp::Not;
    if (op == "bit-not") return IrUnaryOp::BitNot;
    throw CompileError("IR summary unknown unary op '" + op + "'");
}

IrBinaryOp replay_binary_op(const std::string& op) {
    static const std::map<std::string, IrBinaryOp> ops = {
        {"logical-or", IrBinaryOp::LogicalOr},
        {"logical-and", IrBinaryOp::LogicalAnd},
        {"add", IrBinaryOp::Add},
        {"sub", IrBinaryOp::Sub},
        {"mul", IrBinaryOp::Mul},
        {"div", IrBinaryOp::Div},
        {"mod", IrBinaryOp::Mod},
        {"bit-and", IrBinaryOp::BitAnd},
        {"bit-or", IrBinaryOp::BitOr},
        {"bit-xor", IrBinaryOp::BitXor},
        {"shl", IrBinaryOp::Shl},
        {"shr", IrBinaryOp::Shr},
        {"eq", IrBinaryOp::Eq},
        {"ne", IrBinaryOp::Ne},
        {"lt", IrBinaryOp::Lt},
        {"le", IrBinaryOp::Le},
        {"gt", IrBinaryOp::Gt},
        {"ge", IrBinaryOp::Ge},
    };
    auto found = ops.find(op);
    if (found == ops.end()) throw CompileError("IR summary unknown binary op '" + op + "'");
    return found->second;
}

IrExprKind replay_expr_kind(const std::string& kind) {
    static const std::map<std::string, IrExprKind> kinds = {
        {"integer", IrExprKind::Integer},
        {"float", IrExprKind::Float},
        {"string", IrExprKind::String},
        {"bool", IrExprKind::Bool},
        {"null", IrExprKind::Null},
        {"function-ref", IrExprKind::FunctionRef},
        {"local", IrExprKind::Local},
        {"borrow", IrExprKind::Borrow},
        {"unary", IrExprKind::Unary},
        {"cast", IrExprKind::Cast},
        {"pointer-offset", IrExprKind::PointerOffset},
        {"pointer-add", IrExprKind::PointerAdd},
        {"pointer-load", IrExprKind::PointerLoad},
        {"pointer-store", IrExprKind::PointerStore},
        {"try", IrExprKind::Try},
        {"null-coalesce", IrExprKind::NullCoalesce},
        {"enum-tag", IrExprKind::EnumTag},
        {"enum-construct", IrExprKind::EnumConstruct},
        {"tuple", IrExprKind::Tuple},
        {"tuple-index", IrExprKind::TupleIndex},
        {"index", IrExprKind::Index},
        {"slice-range", IrExprKind::SliceRange},
        {"vector", IrExprKind::Vector},
        {"vector-push", IrExprKind::VectorPush},
        {"vector-pop", IrExprKind::VectorPop},
        {"vector-reserve", IrExprKind::VectorReserve},
        {"vector-clear", IrExprKind::VectorClear},
        {"vector-truncate", IrExprKind::VectorTruncate},
        {"vector-set", IrExprKind::VectorSet},
        {"vector-swap", IrExprKind::VectorSwap},
        {"vector-remove", IrExprKind::VectorRemove},
        {"vector-insert", IrExprKind::VectorInsert},
        {"vector-contains", IrExprKind::VectorContains},
        {"vector-index-of", IrExprKind::VectorIndexOf},
        {"vector-count", IrExprKind::VectorCount},
        {"noop", IrExprKind::Noop},
        {"format-print", IrExprKind::FormatPrint},
        {"match", IrExprKind::Match},
        {"if", IrExprKind::If},
        {"block", IrExprKind::Block},
        {"indirect-call", IrExprKind::IndirectCall},
        {"trait-object-call", IrExprKind::TraitObjectCall},
        {"binary", IrExprKind::Binary},
        {"call", IrExprKind::Call},
    };
    auto found = kinds.find(kind);
    if (found == kinds.end()) throw CompileError("IR summary unknown expression kind '" + kind + "'");
    return found->second;
}

IrStmtKind replay_stmt_kind(const std::string& kind) {
    static const std::map<std::string, IrStmtKind> kinds = {
        {"block", IrStmtKind::Block},
        {"var", IrStmtKind::VarDecl},
        {"assign", IrStmtKind::Assign},
        {"expr", IrStmtKind::ExprStmt},
        {"return", IrStmtKind::Return},
        {"if", IrStmtKind::If},
        {"while", IrStmtKind::While},
        {"while-let", IrStmtKind::WhileLet},
        {"for-range", IrStmtKind::ForRange},
        {"for-vector", IrStmtKind::ForVector},
        {"init-while", IrStmtKind::InitWhile},
        {"continue", IrStmtKind::Continue},
        {"break", IrStmtKind::Break},
        {"match", IrStmtKind::Match},
        {"drop", IrStmtKind::Drop},
    };
    auto found = kinds.find(kind);
    if (found == kinds.end()) throw CompileError("IR summary unknown statement kind '" + kind + "'");
    return found->second;
}

IrExprPtr replay_expr(const ModuleCacheIrExprSummaryPtr& summary, const ReplayTypeContext& context);
IrStmtPtr replay_stmt(const ModuleCacheIrStmtSummaryPtr& summary, const ReplayTypeContext& context);

std::vector<IrExprPtr> replay_exprs(const std::vector<ModuleCacheIrExprSummaryPtr>& summaries,
                                    const ReplayTypeContext& context) {
    std::vector<IrExprPtr> result;
    result.reserve(summaries.size());
    for (const auto& summary : summaries) result.push_back(replay_expr(summary, context));
    return result;
}

std::vector<IrStmtPtr> replay_stmts(const std::vector<ModuleCacheIrStmtSummaryPtr>& summaries,
                                    const ReplayTypeContext& context) {
    std::vector<IrStmtPtr> result;
    result.reserve(summaries.size());
    for (const auto& summary : summaries) result.push_back(replay_stmt(summary, context));
    return result;
}

IrBinding replay_binding(const ModuleCacheIrBindingSummary& summary, const ReplayTypeContext& context) {
    IrBinding binding;
    binding.name = summary.name;
    binding.type = replay_type(summary.type, context);
    binding.init = replay_expr(summary.init, context);
    binding.mutable_binding = summary.mutable_binding;
    binding.loc = replay_loc();
    return binding;
}

IrPayloadBinding replay_payload_binding(const ModuleCacheIrPayloadBindingSummary& summary,
                                        const ReplayTypeContext& context) {
    IrPayloadBinding binding;
    binding.index = to_u32(summary.index, "payload binding index");
    binding.name = summary.name;
    binding.type = replay_type(summary.type, context);
    binding.compact_enum_payload = summary.compact_enum_payload;
    binding.compact_enum_type = replay_type(summary.compact_enum_type, context);
    binding.compact_enum_payload_index =
        to_u32(summary.compact_enum_payload_index, "compact enum payload index");
    return binding;
}

std::vector<IrPayloadBinding> replay_payload_bindings(
    const std::vector<ModuleCacheIrPayloadBindingSummary>& summaries,
    const ReplayTypeContext& context) {
    std::vector<IrPayloadBinding> result;
    result.reserve(summaries.size());
    for (const auto& summary : summaries) result.push_back(replay_payload_binding(summary, context));
    return result;
}

std::vector<IrPayloadLiteralCondition> replay_payload_literal_conditions(
    const std::vector<ModuleCacheIrPayloadLiteralConditionSummary>& summaries) {
    std::vector<IrPayloadLiteralCondition> result;
    result.reserve(summaries.size());
    for (const auto& summary : summaries) {
        std::uint32_t index = to_u32(summary.index, "payload literal condition index");
        if (summary.is_bool) {
            result.push_back(IrPayloadLiteralCondition::boolean(
                index,
                parse_bool_text(summary.literal, "payload literal")));
        } else {
            result.push_back(IrPayloadLiteralCondition::integer(
                index,
                parse_signed_integer(summary.literal, "payload literal").value));
        }
    }
    return result;
}

std::vector<IrPayloadRangeCondition> replay_payload_range_conditions(
    const std::vector<ModuleCacheIrPayloadRangeConditionSummary>& summaries,
    const ReplayTypeContext& context) {
    std::vector<IrPayloadRangeCondition> result;
    result.reserve(summaries.size());
    for (const auto& summary : summaries) {
        IrPayloadRangeCondition condition;
        condition.index = to_u32(summary.index, "payload range condition index");
        SignedInteger start = parse_signed_integer(summary.start, "payload range start");
        SignedInteger end = parse_signed_integer(summary.end, "payload range end");
        condition.start_int = start.value;
        condition.start_negative = start.negative;
        condition.end_int = end.value;
        condition.end_negative = end.negative;
        condition.inclusive = summary.inclusive;
        condition.is_unsigned = summary.is_unsigned;
        condition.type = replay_type(summary.type, context);
        condition.compact_enum_payload = summary.compact_enum_payload;
        result.push_back(std::move(condition));
    }
    return result;
}

std::vector<IrPayloadEnumCondition> replay_payload_enum_conditions(
    const std::vector<ModuleCacheIrPayloadEnumConditionSummary>& summaries,
    const ReplayTypeContext& context) {
    std::vector<IrPayloadEnumCondition> result;
    result.reserve(summaries.size());
    for (const auto& summary : summaries) {
        IrPayloadEnumCondition condition;
        condition.index = to_u32(summary.index, "payload enum condition index");
        condition.enum_type = replay_type(summary.enum_type, context);
        condition.tag = to_u32(summary.tag, "payload enum tag");
        condition.nested_payload_index =
            to_u32(summary.nested_payload_index, "payload enum nested payload index");
        condition.has_payload_literal = summary.has_payload_literal;
        condition.payload_literal_is_bool = summary.payload_literal_is_bool;
        if (summary.payload_literal_is_bool) {
            condition.payload_literal.boolean =
                parse_bool_text(summary.payload_literal, "payload enum literal");
        } else {
            condition.payload_literal.integer =
                parse_signed_integer(summary.payload_literal, "payload enum literal").value;
        }
        condition.payload_literal_negative = summary.payload_literal_negative;
        condition.has_payload_range = summary.has_payload_range;
        SignedInteger start = parse_signed_integer(summary.range_start, "payload enum range start");
        SignedInteger end = parse_signed_integer(summary.range_end, "payload enum range end");
        condition.range_start_int = start.value;
        condition.range_start_negative = start.negative;
        condition.range_end_int = end.value;
        condition.range_end_negative = end.negative;
        condition.range_inclusive = summary.range_inclusive;
        condition.range_is_unsigned = summary.range_is_unsigned;
        condition.payload_type = replay_type(summary.payload_type, context);
        result.push_back(std::move(condition));
    }
    return result;
}

template <typename Arm>
void replay_match_pattern(const ModuleCacheIrMatchArmPatternSummary& summary,
                          Arm& arm,
                          const ReplayTypeContext& context) {
    arm.wildcard = summary.wildcard;
    arm.has_literal = summary.has_literal;
    arm.literal_is_bool = summary.literal_is_bool;
    if (summary.has_literal) {
        if (summary.literal_is_bool) {
            arm.literal_bool = parse_bool_text(summary.literal, "match literal");
        } else {
            SignedInteger literal = parse_signed_integer(summary.literal, "match literal");
            arm.literal_int = literal.value;
            arm.literal_negative = literal.negative;
        }
    }
    arm.has_range = summary.has_range;
    SignedInteger range_start = parse_signed_integer(summary.range_start, "match range start");
    SignedInteger range_end = parse_signed_integer(summary.range_end, "match range end");
    arm.range_start_int = range_start.value;
    arm.range_start_negative = range_start.negative;
    arm.range_end_int = range_end.value;
    arm.range_end_negative = range_end.negative;
    arm.range_inclusive = summary.range_inclusive;
    arm.range_is_unsigned = summary.range_is_unsigned;
    arm.case_name = summary.case_name;
    arm.enum_tag = to_u32(summary.enum_tag, "match enum tag");
    arm.has_value_binding = summary.has_value_binding;
    arm.value_name = summary.value_name;
    arm.value_type = replay_type(summary.value_type, context);
    arm.has_payload_binding = summary.has_payload_binding;
    arm.payload_name = summary.payload_name;
    arm.payload_type = replay_type(summary.payload_type, context);
    arm.payload_index = to_u32(summary.payload_index, "match payload index");
    arm.payload_bindings = replay_payload_bindings(summary.payload_bindings, context);
    arm.payload_literal_conditions =
        replay_payload_literal_conditions(summary.payload_literal_conditions);
    arm.payload_range_conditions =
        replay_payload_range_conditions(summary.payload_range_conditions, context);
    arm.payload_enum_conditions =
        replay_payload_enum_conditions(summary.payload_enum_conditions, context);
    arm.loc = replay_loc();
}

IrMatchArm replay_stmt_match_arm(const ModuleCacheIrStmtMatchArmSummary& summary,
                                 const ReplayTypeContext& context) {
    IrMatchArm arm;
    replay_match_pattern(summary.pattern, arm, context);
    arm.body = replay_stmts(summary.body, context);
    return arm;
}

IrMatchExprArm replay_expr_match_arm(const ModuleCacheIrExprMatchArmSummary& summary,
                                     const ReplayTypeContext& context) {
    IrMatchExprArm arm;
    replay_match_pattern(summary.pattern, arm, context);
    arm.body = replay_stmts(summary.body, context);
    arm.value = replay_expr(summary.value, context);
    return arm;
}

IrExprPtr replay_expr(const ModuleCacheIrExprSummaryPtr& summary, const ReplayTypeContext& context) {
    if (!summary) return nullptr;
    auto expr = std::make_unique<IrExpr>();
    expr->kind = replay_expr_kind(summary->kind);
    expr->type = replay_type(summary->type, context);
    expr->loc = replay_loc();
    expr->mutable_borrow = summary->mutable_borrow;
    expr->unary_op = replay_unary_op(summary->unary_op);
    expr->op = replay_binary_op(summary->binary_op);
    set_ir_expr_name(*expr, summary->name);
    set_ir_expr_label(*expr, summary->label);
    set_ir_expr_string_value(*expr, summary->string_value);
    set_ir_expr_borrow_source(*expr, summary->borrow_source_name, summary->borrow_source_path);
    set_ir_expr_enum_case(*expr, summary->enum_name, summary->case_name);

    switch (expr->kind) {
        case IrExprKind::Integer: {
            SignedInteger integer = parse_signed_integer(summary->scalar_payload, "integer payload");
            expr->int_value = integer.value;
            expr->int_negative = integer.negative;
            break;
        }
        case IrExprKind::Float:
            expr->float_value = summary->scalar_payload.empty()
                ? 0.0
                : std::strtod(summary->scalar_payload.c_str(), nullptr);
            break;
        case IrExprKind::Bool:
            expr->bool_value = parse_bool_text(summary->scalar_payload, "bool payload");
            break;
        case IrExprKind::TupleIndex:
            expr->tuple_index = parse_signed_integer(summary->scalar_payload, "tuple index").value;
            break;
        default:
            break;
    }

    set_ir_expr_enum_result_payload(
        *expr,
        to_u32(summary->enum_tag, "expression enum tag"),
        summary->has_enum_payload,
        replay_type(summary->enum_payload_type, context));
    set_ir_expr_operand(*expr, replay_expr(summary->operand, context));
    set_ir_expr_left(*expr, replay_expr(summary->left, context));
    set_ir_expr_right(*expr, replay_expr(summary->right, context));
    set_ir_expr_payload(*expr, replay_expr(summary->payload, context));
    for (auto& arg : replay_exprs(summary->args, context)) expr->args.push_back(std::move(arg));

    std::vector<IrType> call_param_types;
    call_param_types.reserve(summary->call_param_types.size());
    for (const auto& type : summary->call_param_types) {
        call_param_types.push_back(replay_type(type, context));
    }
    if (!call_param_types.empty()) set_ir_expr_call_param_types(*expr, std::move(call_param_types));

    if (summary->if_condition || !summary->if_then_body.empty() || summary->if_then_value ||
        !summary->if_else_body.empty() || summary->if_else_value) {
        set_ir_expr_if_payload(
            *expr,
            replay_expr(summary->if_condition, context),
            replay_stmts(summary->if_then_body, context),
            replay_expr(summary->if_then_value, context),
            replay_stmts(summary->if_else_body, context),
            replay_expr(summary->if_else_value, context));
    }

    if (!summary->block_label.empty() || !summary->block_body.empty() || summary->block_value) {
        set_ir_expr_block_payload(
            *expr,
            summary->block_label,
            replay_stmts(summary->block_body, context),
            replay_expr(summary->block_value, context));
    }

    if (summary->match_value || !summary->match_arms.empty()) {
        set_ir_expr_match_value(*expr, replay_expr(summary->match_value, context));
        auto& arms = ir_expr_match_arms(*expr);
        arms.reserve(summary->match_arms.size());
        for (const auto& arm : summary->match_arms) arms.push_back(replay_expr_match_arm(arm, context));
    }

    if (expr->kind == IrExprKind::FormatPrint || !summary->format_parts.empty() ||
        !summary->format_specs.empty()) {
        std::vector<IrFormatSpec> specs;
        specs.reserve(summary->format_specs.size());
        for (const auto& spec_text : summary->format_specs) {
            IrFormatSpec spec;
            spec.precision = spec_text.empty() ? -1 : std::atoi(spec_text.c_str());
            specs.push_back(spec);
        }
        set_ir_expr_format_print_payload(
            *expr,
            summary->format_parts,
            std::move(specs),
            summary->format_print_newline);
    }

    if (expr->kind == IrExprKind::Try || summary->try_converts_residual ||
        summary->try_residual_has_payload || !summary->try_residual_cleanup.empty()) {
        set_ir_expr_try_payload(
            *expr,
            summary->try_converts_residual,
            to_u32(summary->try_return_residual_tag, "try residual return tag"),
            summary->try_residual_has_payload,
            replay_type(summary->try_return_residual_payload_type, context),
            replay_stmts(summary->try_residual_cleanup, context));
    }

    return expr;
}

IrStmtPtr replay_stmt(const ModuleCacheIrStmtSummaryPtr& summary, const ReplayTypeContext& context) {
    if (!summary) return nullptr;
    auto stmt = std::make_unique<IrStmt>();
    stmt->kind = replay_stmt_kind(summary->kind);
    stmt->loc = replay_loc();
    set_ir_stmt_label(*stmt, summary->label);
    if (!summary->drop_name.empty()) set_ir_stmt_drop_name(*stmt, summary->drop_name);
    stmt->binding = replay_binding(summary->binding, context);

    if (!summary->assign_name.empty() || summary->assign_target || summary->assign_rhs) {
        set_ir_stmt_assign_name(*stmt, summary->assign_name);
        set_ir_stmt_assign_target(*stmt, replay_expr(summary->assign_target, context));
        set_ir_stmt_assign_rhs(*stmt, replay_expr(summary->assign_rhs, context));
    }

    stmt->expr = replay_expr(summary->expr, context);
    stmt->condition = replay_expr(summary->condition, context);
    set_ir_stmt_statements(*stmt, replay_stmts(summary->statements, context));
    set_ir_stmt_then_body(*stmt, replay_stmts(summary->then_body, context));
    set_ir_stmt_else_body(*stmt, replay_stmts(summary->else_body, context));
    set_ir_stmt_loop_body(*stmt, replay_stmts(summary->loop_body, context));

    if (!summary->for_binding_name.empty() || !summary->for_index_name.empty() ||
        !summary->for_end_name.empty() || summary->for_start || summary->for_end ||
        !summary->for_values.empty()) {
        ir_stmt_for_binding_name(*stmt) = summary->for_binding_name;
        ir_stmt_for_index_name(*stmt) = summary->for_index_name;
        ir_stmt_for_end_name(*stmt) = summary->for_end_name;
        ir_stmt_for_binding_type(*stmt) = replay_type(summary->for_binding_type, context);
        set_ir_stmt_for_inclusive(*stmt, summary->for_inclusive);
        ir_stmt_for_start(*stmt) = replay_expr(summary->for_start, context);
        ir_stmt_for_end(*stmt) = replay_expr(summary->for_end, context);
        ir_stmt_for_values(*stmt) = replay_exprs(summary->for_values, context);
    }

    stmt->match_value = replay_expr(summary->match_value, context);
    stmt->init_bindings.reserve(summary->init_bindings.size());
    for (const auto& binding : summary->init_bindings) {
        stmt->init_bindings.push_back(replay_binding(binding, context));
    }
    stmt->updates = replay_exprs(summary->updates, context);

    if (!summary->match_arms.empty()) {
        auto& arms = ensure_ir_stmt_match_arms(*stmt);
        arms.reserve(summary->match_arms.size());
        for (const auto& arm : summary->match_arms) arms.push_back(replay_stmt_match_arm(arm, context));
    }

    if (!summary->break_label.empty() || summary->break_value) {
        set_ir_stmt_break_label(*stmt, summary->break_label);
        set_ir_stmt_break_value(*stmt, replay_expr(summary->break_value, context));
    }
    return stmt;
}

IrFunction replay_function(const ModuleCacheIrFunctionSummary& summary,
                           const ReplayTypeContext& context) {
    IrFunction fn;
    fn.name = summary.name;
    fn.module_name = summary.module_name;
    fn.link_name = summary.link_name;
    fn.return_type = replay_type(summary.return_type, context);
    fn.shared_export = summary.shared_export;
    fn.loc = replay_loc();
    fn.params.reserve(summary.params.size());
    for (const auto& param : summary.params) {
        fn.params.push_back(IrParam{param.name, replay_type(param.type, context)});
    }
    fn.body = replay_stmts(summary.body.statements, context);
    return fn;
}

} // namespace

std::set<std::string> module_cache_ir_function_names(
    const std::vector<ModuleCacheIrFunctionSummary>& functions) {
    std::set<std::string> names;
    for (const auto& fn : functions) names.insert(fn.name);
    return names;
}

std::vector<IrFunction> replay_module_cache_ir_functions(
    const std::vector<ModuleCacheIrFunctionSummary>& functions,
    const Program& declarations) {
    ReplayTypeContext context = make_type_context(declarations);
    std::map<std::string, std::size_t> declaration_order;
    for (const auto& fn : declarations.functions) {
        if (!declaration_order.count(fn.name)) declaration_order.emplace(fn.name, declaration_order.size());
    }

    std::map<std::string, std::pair<std::uint32_t, std::uint32_t>> impl_method_locations;
    for (const auto& impl : declarations.impls) {
        for (const auto& method : impl.methods) {
            std::pair<std::uint32_t, std::uint32_t> loc{method.loc.line, method.loc.column};
            auto found = impl_method_locations.find(method.name);
            if (found == impl_method_locations.end() || loc < found->second) {
                impl_method_locations[method.name] = loc;
            }
        }
    }
    std::vector<std::pair<std::string, std::pair<std::uint32_t, std::uint32_t>>> impl_methods(
        impl_method_locations.begin(), impl_method_locations.end());
    std::sort(
        impl_methods.begin(),
        impl_methods.end(),
        [](const auto& left, const auto& right) {
            if (left.second != right.second) return left.second < right.second;
            return left.first < right.first;
        });
    std::map<std::string, std::size_t> impl_method_order;
    for (const auto& method : impl_methods) {
        impl_method_order.emplace(method.first, impl_method_order.size());
    }

    auto sort_key = [&](const ModuleCacheIrFunctionSummary& fn) {
        auto free_order = declaration_order.find(fn.name);
        if (free_order != declaration_order.end()) {
            return std::pair<int, std::size_t>{0, free_order->second};
        }
        if (starts_with(fn.name, 0, "impl::")) {
            int impl_group = fn.name.find("::inherent::") != std::string::npos ? 1 : 2;
            auto method_order = impl_method_order.find(short_name(fn.name));
            std::size_t order = method_order == impl_method_order.end() ? 0 : method_order->second;
            return std::pair<int, std::size_t>{impl_group, order};
        }
        auto method_order = impl_method_order.find(short_name(fn.name));
        if (method_order != impl_method_order.end()) {
            return std::pair<int, std::size_t>{2, method_order->second};
        }
        return std::pair<int, std::size_t>{3, 0};
    };

    std::vector<const ModuleCacheIrFunctionSummary*> ordered;
    ordered.reserve(functions.size());
    for (const auto& fn : functions) ordered.push_back(&fn);
    std::stable_sort(
        ordered.begin(),
        ordered.end(),
        [&](const ModuleCacheIrFunctionSummary* left, const ModuleCacheIrFunctionSummary* right) {
            return sort_key(*left) < sort_key(*right);
        });

    std::vector<IrFunction> result;
    result.reserve(functions.size());
    for (const auto* fn : ordered) result.push_back(replay_function(*fn, context));
    return result;
}

} // namespace ari
