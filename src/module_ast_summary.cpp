#include "module_ast_summary.hpp"

#include "common.hpp"
#include "module_metadata.hpp"
#include "module_path.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace ari {
namespace {

void append_field(std::ostringstream& out, const std::string& value) {
    out << value.size() << ':' << value << ';';
}

void append_bool(std::ostringstream& out, bool value) {
    out << (value ? "1;" : "0;");
}

void append_count(std::ostringstream& out, std::uint64_t value) {
    out << value << ';';
}

void append_qualifier(std::ostringstream& out, TypeQualifier qualifier) {
    switch (qualifier) {
        case TypeQualifier::Value: append_field(out, "value"); return;
        case TypeQualifier::Own: append_field(out, "own"); return;
        case TypeQualifier::Ref: append_field(out, "ref"); return;
        case TypeQualifier::MutRef: append_field(out, "ref mut"); return;
        case TypeQualifier::Ptr: append_field(out, "ptr"); return;
    }
}

void append_type(std::ostringstream& out, const TypeRef& type) {
    append_qualifier(out, type.qualifier);
    append_field(out, type.name);
    append_bool(out, type.is_dyn_object);
    append_bool(out, type.nullable);
    append_count(out, type.array_size);
    append_count(out, type.args.size());
    for (const auto& arg : type.args) append_type(out, arg);
}

void append_generics(std::ostringstream& out, const std::vector<GenericParam>& generics) {
    append_count(out, generics.size());
    for (const auto& generic : generics) {
        append_field(out, generic.name);
        append_bool(out, generic.has_constraint);
        if (generic.has_constraint) append_type(out, generic.constraint);
    }
}

void append_attributes(std::ostringstream& out, const std::vector<Attribute>& attributes) {
    append_count(out, attributes.size());
    for (const auto& attr : attributes) {
        append_field(out, attr.name);
        append_bool(out, attr.has_args);
        append_count(out, attr.args.size());
        for (const auto& token : attr.args) {
            append_count(out, static_cast<std::uint64_t>(token.kind));
            append_field(out, token.text);
        }
    }
}

void append_function_signature(std::ostringstream& out, const FunctionDecl& fn) {
    append_field(out, fn.module_name);
    append_field(out, fn.name);
    append_bool(out, fn.meta);
    append_bool(out, fn.is_extern);
    append_bool(out, fn.is_public);
    append_bool(out, fn.is_variadic);
    append_field(out, fn.extern_abi);
    append_field(out, fn.extern_link_name);
    append_bool(out, fn.has_return_type);
    append_bool(out, fn.has_body);
    append_generics(out, fn.generics);
    append_attributes(out, fn.attributes);
    append_count(out, fn.params.size());
    for (const auto& param : fn.params) {
        append_field(out, param.name);
        append_bool(out, param.has_pattern);
        append_type(out, param.type);
    }
    if (fn.has_return_type) append_type(out, fn.return_type);
}

bool is_summary_const_unary_op(TokenKind op) {
    return op == TokenKind::Bang || op == TokenKind::Minus;
}

bool is_summary_const_binary_op(TokenKind op) {
    switch (op) {
        case TokenKind::AmpAmp:
        case TokenKind::PipePipe:
        case TokenKind::EqEq:
        case TokenKind::BangEq:
        case TokenKind::Less:
        case TokenKind::LessEq:
        case TokenKind::Greater:
        case TokenKind::GreaterEq:
        case TokenKind::Plus:
        case TokenKind::Minus:
        case TokenKind::Star:
        case TokenKind::Slash:
        case TokenKind::Percent:
            return true;
        default:
            return false;
    }
}

bool append_const_expr_list(std::ostringstream& out, const std::vector<ExprPtr>& args);

void append_type_arguments(std::ostringstream& out, const std::vector<TypeRef>& type_args) {
    append_count(out, type_args.size());
    for (const auto& type_arg : type_args) append_type(out, type_arg);
}

bool append_const_expr_payload(std::ostringstream& out, const Expr& expr) {
    switch (expr.kind) {
        case ExprKind::Integer:
            append_field(out, "integer");
            append_bool(out, expr.int_negative);
            append_count(out, expr.int_value);
            append_field(out, expr.literal_suffix);
            return true;
        case ExprKind::Bool:
            append_field(out, "bool");
            append_bool(out, expr.bool_value);
            return true;
        case ExprKind::Name:
            append_field(out, "name");
            append_field(out, expr.name);
            return true;
        case ExprKind::Unary: {
            if (!expr.operand || !is_summary_const_unary_op(expr.op)) return false;
            std::ostringstream operand;
            if (!append_const_expr_payload(operand, *expr.operand)) return false;
            append_field(out, "unary");
            append_count(out, static_cast<std::uint64_t>(expr.op));
            out << operand.str();
            return true;
        }
        case ExprKind::Binary: {
            if (!expr.left || !expr.right || !is_summary_const_binary_op(expr.op)) return false;
            std::ostringstream left;
            std::ostringstream right;
            if (!append_const_expr_payload(left, *expr.left)) return false;
            if (!append_const_expr_payload(right, *expr.right)) return false;
            append_field(out, "binary");
            append_count(out, static_cast<std::uint64_t>(expr.op));
            out << left.str();
            out << right.str();
            return true;
        }
        case ExprKind::Tuple: {
            std::ostringstream args;
            if (!append_const_expr_list(args, expr.args)) return false;
            append_field(out, "tuple");
            out << args.str();
            return true;
        }
        case ExprKind::Vector: {
            std::ostringstream args;
            if (!append_const_expr_list(args, expr.args)) return false;
            append_field(out, "vector");
            out << args.str();
            return true;
        }
        case ExprKind::StructLiteral: {
            if (expr.field_names.size() != expr.args.size()) return false;
            std::vector<std::ostringstream> values(expr.args.size());
            for (std::size_t i = 0; i < expr.args.size(); ++i) {
                if (!append_const_expr_payload(values[i], *expr.args[i])) return false;
            }
            append_field(out, "struct");
            append_field(out, expr.name);
            append_type_arguments(out, expr.type_args);
            append_count(out, expr.args.size());
            for (std::size_t i = 0; i < expr.args.size(); ++i) {
                append_field(out, expr.field_names[i]);
                out << values[i].str();
            }
            return true;
        }
        case ExprKind::Call: {
            if (expr.operand) return false;
            std::ostringstream args;
            if (!append_const_expr_list(args, expr.args)) return false;
            append_field(out, "call");
            append_field(out, expr.name);
            append_type_arguments(out, expr.type_args);
            out << args.str();
            return true;
        }
        default:
            return false;
    }
}

bool append_const_expr_list(std::ostringstream& out, const std::vector<ExprPtr>& args) {
    std::vector<std::ostringstream> payloads(args.size());
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (!args[i] || !append_const_expr_payload(payloads[i], *args[i])) return false;
    }
    append_count(out, args.size());
    for (const auto& payload : payloads) out << payload.str();
    return true;
}

void append_const_initializer(std::ostringstream& out, const ExprPtr& init) {
    if (!init) {
        append_bool(out, false);
        return;
    }
    std::ostringstream payload;
    if (!append_const_expr_payload(payload, *init)) {
        append_bool(out, false);
        return;
    }
    append_bool(out, true);
    out << payload.str();
}

struct DeclarationSummaryCounts {
    std::uint64_t use_count = 0;
    std::uint64_t module_import_count = 0;
    std::uint64_t module_decl_count = 0;
    std::uint64_t constant_count = 0;
    std::uint64_t function_count = 0;
    std::uint64_t struct_count = 0;
    std::uint64_t enum_count = 0;
    std::uint64_t trait_count = 0;
    std::uint64_t impl_count = 0;
};

std::string summary_display(const ModuleCacheAstSummary& summary) {
    return "'" + (summary.module_name.empty() ? "<root>" : summary.module_name) +
           "' at '" + summary.path + "'";
}

class DeclarationSummaryReader {
public:
    DeclarationSummaryReader(const std::string& text, std::string display)
        : text_(text), display_(std::move(display)) {}

    DeclarationSummaryCounts parse() {
        consume_header();
        DeclarationSummaryCounts counts;

        counts.use_count = read_count("use count");
        for (std::uint64_t i = 0; i < counts.use_count; ++i) {
            read_field("use module name");
            read_field("use path");
            read_field("use alias");
            read_bool("use visibility");
            read_bool("use glob flag");
        }

        counts.module_import_count = read_count("module import count");
        for (std::uint64_t i = 0; i < counts.module_import_count; ++i) {
            read_field("module import owner");
            read_field("module import name");
            read_field("module import local name");
            read_bool("module import visibility");
        }

        counts.module_decl_count = read_count("module declaration count");
        for (std::uint64_t i = 0; i < counts.module_decl_count; ++i) {
            read_field("module declaration owner");
            read_field("module declaration name");
            read_bool("module declaration visibility");
        }

        counts.constant_count = read_count("constant count");
        for (std::uint64_t i = 0; i < counts.constant_count; ++i) {
            read_field("constant module name");
            read_field("constant name");
            read_bool("constant visibility");
            skip_type("constant type");
            if (version_ >= 2) skip_const_initializer("constant initializer");
        }

        counts.function_count = read_count("function count");
        for (std::uint64_t i = 0; i < counts.function_count; ++i) skip_function_signature();

        counts.struct_count = read_count("struct count");
        for (std::uint64_t i = 0; i < counts.struct_count; ++i) {
            read_field("struct module name");
            read_field("struct name");
            read_bool("struct visibility");
            read_bool("tuple struct flag");
            skip_generics();
            skip_attributes();
            std::uint64_t field_count = read_count("struct field count");
            for (std::uint64_t j = 0; j < field_count; ++j) {
                read_field("struct field name");
                read_bool("struct field mutability");
                skip_type("struct field type");
            }
        }

        counts.enum_count = read_count("enum count");
        for (std::uint64_t i = 0; i < counts.enum_count; ++i) {
            read_field("enum module name");
            read_field("enum name");
            read_bool("enum visibility");
            skip_generics();
            skip_attributes();
            std::uint64_t case_count = read_count("enum case count");
            for (std::uint64_t j = 0; j < case_count; ++j) {
                read_field("enum case name");
                std::uint64_t payload_count = read_count("enum payload count");
                for (std::uint64_t k = 0; k < payload_count; ++k) skip_type("enum payload type");
            }
        }

        counts.trait_count = read_count("trait count");
        for (std::uint64_t i = 0; i < counts.trait_count; ++i) {
            read_field("trait module name");
            read_field("trait name");
            read_bool("trait visibility");
            skip_generics();
            skip_attributes();
            std::uint64_t method_count = read_count("trait method count");
            for (std::uint64_t j = 0; j < method_count; ++j) skip_function_signature();
        }

        counts.impl_count = read_count("impl count");
        for (std::uint64_t i = 0; i < counts.impl_count; ++i) {
            read_field("impl module name");
            read_bool("impl visibility");
            bool has_trait = read_bool("impl trait flag");
            skip_generics();
            skip_attributes();
            if (has_trait) skip_type("impl trait type");
            skip_type("impl target type");
            std::uint64_t method_count = read_count("impl method count");
            for (std::uint64_t j = 0; j < method_count; ++j) skip_function_signature();
        }

        if (pos_ != text_.size()) fail("trailing bytes after declaration summary");
        return counts;
    }

    Program parse_program() {
        consume_header();
        Program program;

        std::uint64_t use_count = read_count("use count");
        program.uses.reserve(static_cast<std::size_t>(use_count));
        for (std::uint64_t i = 0; i < use_count; ++i) {
            UseDecl decl;
            decl.module_name = read_field("use module name");
            decl.path = read_field("use path");
            decl.alias = read_field("use alias");
            decl.is_public = read_bool("use visibility");
            decl.is_glob = read_bool("use glob flag");
            decl.loc = default_loc();
            program.uses.push_back(std::move(decl));
        }

        std::uint64_t import_count = read_count("module import count");
        program.module_imports.reserve(static_cast<std::size_t>(import_count));
        for (std::uint64_t i = 0; i < import_count; ++i) {
            ModuleImport decl;
            decl.module_name = read_field("module import owner");
            decl.name = read_field("module import name");
            decl.local_name = read_field("module import local name");
            decl.is_public = read_bool("module import visibility");
            decl.loc = default_loc();
            program.module_imports.push_back(std::move(decl));
        }

        std::uint64_t module_count = read_count("module declaration count");
        program.modules.reserve(static_cast<std::size_t>(module_count));
        for (std::uint64_t i = 0; i < module_count; ++i) {
            ModuleDecl decl;
            decl.module_name = read_field("module declaration owner");
            decl.name = read_field("module declaration name");
            decl.is_public = read_bool("module declaration visibility");
            decl.loc = default_loc();
            program.modules.push_back(std::move(decl));
        }

        std::uint64_t constant_count = read_count("constant count");
        program.constants.reserve(static_cast<std::size_t>(constant_count));
        for (std::uint64_t i = 0; i < constant_count; ++i) {
            ConstDecl decl;
            decl.module_name = read_field("constant module name");
            decl.name = read_field("constant name");
            decl.is_public = read_bool("constant visibility");
            decl.type = read_type("constant type");
            if (version_ >= 2) {
                decl.init = read_const_initializer("constant initializer");
            }
            decl.loc = default_loc();
            program.constants.push_back(std::move(decl));
        }

        std::uint64_t function_count = read_count("function count");
        program.functions.reserve(static_cast<std::size_t>(function_count));
        for (std::uint64_t i = 0; i < function_count; ++i) {
            program.functions.push_back(read_function_signature());
        }

        std::uint64_t struct_count = read_count("struct count");
        program.structs.reserve(static_cast<std::size_t>(struct_count));
        for (std::uint64_t i = 0; i < struct_count; ++i) {
            StructDecl decl;
            decl.module_name = read_field("struct module name");
            decl.name = read_field("struct name");
            decl.is_public = read_bool("struct visibility");
            decl.tuple_struct = read_bool("tuple struct flag");
            decl.generics = read_generics();
            decl.attributes = read_attributes();
            std::uint64_t field_count = read_count("struct field count");
            decl.fields.reserve(static_cast<std::size_t>(field_count));
            for (std::uint64_t j = 0; j < field_count; ++j) {
                StructField field;
                field.name = read_field("struct field name");
                field.mutable_field = read_bool("struct field mutability");
                field.type = read_type("struct field type");
                field.loc = default_loc();
                decl.fields.push_back(std::move(field));
            }
            decl.loc = default_loc();
            program.structs.push_back(std::move(decl));
        }

        std::uint64_t enum_count = read_count("enum count");
        program.enums.reserve(static_cast<std::size_t>(enum_count));
        for (std::uint64_t i = 0; i < enum_count; ++i) {
            EnumDecl decl;
            decl.module_name = read_field("enum module name");
            decl.name = read_field("enum name");
            decl.is_public = read_bool("enum visibility");
            decl.generics = read_generics();
            decl.attributes = read_attributes();
            std::uint64_t case_count = read_count("enum case count");
            decl.cases.reserve(static_cast<std::size_t>(case_count));
            for (std::uint64_t j = 0; j < case_count; ++j) {
                EnumCase item;
                item.name = read_field("enum case name");
                std::uint64_t payload_count = read_count("enum payload count");
                item.payloads.reserve(static_cast<std::size_t>(payload_count));
                for (std::uint64_t k = 0; k < payload_count; ++k) {
                    item.payloads.push_back(read_type("enum payload type"));
                }
                item.loc = default_loc();
                decl.cases.push_back(std::move(item));
            }
            decl.loc = default_loc();
            program.enums.push_back(std::move(decl));
        }

        std::uint64_t trait_count = read_count("trait count");
        program.traits.reserve(static_cast<std::size_t>(trait_count));
        for (std::uint64_t i = 0; i < trait_count; ++i) {
            TraitDecl decl;
            decl.module_name = read_field("trait module name");
            decl.name = read_field("trait name");
            decl.is_public = read_bool("trait visibility");
            decl.generics = read_generics();
            decl.attributes = read_attributes();
            std::uint64_t method_count = read_count("trait method count");
            decl.methods.reserve(static_cast<std::size_t>(method_count));
            for (std::uint64_t j = 0; j < method_count; ++j) {
                decl.methods.push_back(read_function_signature());
            }
            decl.loc = default_loc();
            program.traits.push_back(std::move(decl));
        }

        std::uint64_t impl_count = read_count("impl count");
        program.impls.reserve(static_cast<std::size_t>(impl_count));
        for (std::uint64_t i = 0; i < impl_count; ++i) {
            ImplDecl decl;
            decl.module_name = read_field("impl module name");
            decl.is_public = read_bool("impl visibility");
            decl.has_trait = read_bool("impl trait flag");
            decl.generics = read_generics();
            decl.attributes = read_attributes();
            if (decl.has_trait) decl.trait_type = read_type("impl trait type");
            decl.for_type = read_type("impl target type");
            std::uint64_t method_count = read_count("impl method count");
            decl.methods.reserve(static_cast<std::size_t>(method_count));
            for (std::uint64_t j = 0; j < method_count; ++j) {
                decl.methods.push_back(read_function_signature());
            }
            program.impls.push_back(std::move(decl));
        }

        if (pos_ != text_.size()) fail("trailing bytes after declaration summary");
        return program;
    }

private:
    const std::string& text_;
    std::string display_;
    std::size_t pos_ = 0;
    int version_ = 0;

    [[noreturn]] void fail(const std::string& detail) const {
        throw CompileError("malformed declaration summary for " + display_ + ": " + detail);
    }

    SourceLocation default_loc() const {
        return SourceLocation{1, 1};
    }

    void consume_header() {
        const std::string v2 = "ari-ast-decls-v2;";
        const std::string v1 = "ari-ast-decls-v1;";
        if (text_.compare(pos_, v2.size(), v2) == 0) {
            version_ = 2;
            pos_ += v2.size();
            return;
        }
        if (text_.compare(pos_, v1.size(), v1) == 0) {
            version_ = 1;
            pos_ += v1.size();
            return;
        }
        fail("expected 'ari-ast-decls-v2;' or 'ari-ast-decls-v1;'");
    }

    void consume_char(char expected, const std::string& label) {
        if (pos_ >= text_.size() || text_[pos_] != expected) {
            fail("expected '" + std::string(1, expected) + "' after " + label);
        }
        ++pos_;
    }

    std::uint64_t read_decimal_until(char terminator, const std::string& label) {
        if (pos_ >= text_.size()) fail("expected " + label);
        std::uint64_t result = 0;
        bool saw_digit = false;
        while (pos_ < text_.size() && text_[pos_] != terminator) {
            char c = text_[pos_++];
            if (c < '0' || c > '9') fail("expected decimal " + label);
            saw_digit = true;
            std::uint64_t digit = static_cast<std::uint64_t>(c - '0');
            if (result > (std::numeric_limits<std::uint64_t>::max() - digit) / 10) {
                fail(label + " is too large");
            }
            result = result * 10 + digit;
        }
        if (!saw_digit) fail("expected decimal " + label);
        consume_char(terminator, label);
        return result;
    }

    std::uint64_t read_count(const std::string& label) {
        return read_decimal_until(';', label);
    }

    std::string read_field(const std::string& label) {
        std::uint64_t size = read_decimal_until(':', label + " length");
        if (size > text_.size() - pos_) fail(label + " length exceeds remaining payload");
        std::string value = text_.substr(pos_, static_cast<std::size_t>(size));
        pos_ += static_cast<std::size_t>(size);
        consume_char(';', label);
        return value;
    }

    bool read_bool(const std::string& label) {
        if (pos_ + 1 >= text_.size() || text_[pos_ + 1] != ';') {
            fail("expected boolean " + label);
        }
        char value = text_[pos_];
        pos_ += 2;
        if (value == '0') return false;
        if (value == '1') return true;
        fail("expected boolean " + label);
    }

    TypeQualifier read_qualifier(const std::string& label) {
        std::string value = read_field(label);
        if (value == "value") return TypeQualifier::Value;
        if (value == "own") return TypeQualifier::Own;
        if (value == "ref") return TypeQualifier::Ref;
        if (value == "ref mut") return TypeQualifier::MutRef;
        if (value == "ptr") return TypeQualifier::Ptr;
        fail("unknown type qualifier '" + value + "'");
    }

    TypeRef read_type(const std::string& label) {
        TypeRef type;
        type.qualifier = read_qualifier(label + " qualifier");
        type.name = read_field(label + " name");
        type.is_dyn_object = read_bool(label + " dyn flag");
        type.nullable = read_bool(label + " nullable flag");
        type.array_size = read_count(label + " array size");
        std::uint64_t arg_count = read_count(label + " argument count");
        type.args.reserve(static_cast<std::size_t>(arg_count));
        for (std::uint64_t i = 0; i < arg_count; ++i) {
            type.args.push_back(read_type(label + " argument"));
        }
        type.loc = default_loc();
        return type;
    }

    void skip_type(const std::string& label) {
        (void)read_type(label);
    }

    std::vector<GenericParam> read_generics() {
        std::uint64_t count = read_count("generic parameter count");
        std::vector<GenericParam> generics;
        generics.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t i = 0; i < count; ++i) {
            GenericParam generic;
            generic.name = read_field("generic parameter name");
            generic.has_constraint = read_bool("generic constraint flag");
            if (generic.has_constraint) generic.constraint = read_type("generic constraint type");
            generic.loc = default_loc();
            generics.push_back(std::move(generic));
        }
        return generics;
    }

    void skip_generics() {
        (void)read_generics();
    }

    std::vector<Attribute> read_attributes() {
        std::uint64_t count = read_count("attribute count");
        std::vector<Attribute> attributes;
        attributes.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t i = 0; i < count; ++i) {
            Attribute attr;
            attr.name = read_field("attribute name");
            attr.has_args = read_bool("attribute args flag");
            std::uint64_t arg_count = read_count("attribute token count");
            attr.args.reserve(static_cast<std::size_t>(arg_count));
            for (std::uint64_t j = 0; j < arg_count; ++j) {
                Token token;
                token.kind = static_cast<TokenKind>(read_count("attribute token kind"));
                token.text = read_field("attribute token text");
                token.loc = default_loc();
                attr.args.push_back(std::move(token));
            }
            attr.loc = default_loc();
            attributes.push_back(std::move(attr));
        }
        return attributes;
    }

    void skip_attributes() {
        (void)read_attributes();
    }

    FunctionDecl read_function_signature() {
        FunctionDecl fn;
        fn.module_name = read_field("function module name");
        fn.name = read_field("function name");
        fn.meta = read_bool("function meta flag");
        fn.is_extern = read_bool("function extern flag");
        fn.is_public = read_bool("function visibility");
        fn.is_variadic = read_bool("function variadic flag");
        fn.extern_abi = read_field("function extern ABI");
        fn.extern_link_name = read_field("function extern link name");
        fn.has_return_type = read_bool("function return type flag");
        fn.has_body = read_bool("function body flag");
        fn.generics = read_generics();
        fn.attributes = read_attributes();
        std::uint64_t param_count = read_count("function parameter count");
        fn.params.reserve(static_cast<std::size_t>(param_count));
        for (std::uint64_t i = 0; i < param_count; ++i) {
            Param param;
            param.name = read_field("function parameter name");
            param.has_pattern = read_bool("function parameter pattern flag");
            param.type = read_type("function parameter type");
            fn.params.push_back(std::move(param));
        }
        if (fn.has_return_type) fn.return_type = read_type("function return type");
        fn.loc = default_loc();
        fn.variadic_loc = default_loc();
        return fn;
    }

    void skip_function_signature() {
        (void)read_function_signature();
    }

    TokenKind read_token_kind(const std::string& label) {
        return static_cast<TokenKind>(read_count(label));
    }

    std::vector<TypeRef> read_type_arguments(const std::string& label) {
        std::uint64_t count = read_count(label + " count");
        std::vector<TypeRef> type_args;
        type_args.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t i = 0; i < count; ++i) {
            type_args.push_back(read_type(label + " type argument"));
        }
        return type_args;
    }

    std::vector<ExprPtr> read_const_expr_list(const std::string& label) {
        std::uint64_t count = read_count(label + " count");
        std::vector<ExprPtr> args;
        args.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t i = 0; i < count; ++i) {
            args.push_back(read_const_expr(label + " item"));
        }
        return args;
    }

    ExprPtr read_const_expr(const std::string& label) {
        std::string kind = read_field(label + " kind");
        auto expr = std::make_unique<Expr>();
        expr->loc = default_loc();
        if (kind == "integer") {
            expr->kind = ExprKind::Integer;
            expr->int_negative = read_bool(label + " integer sign");
            expr->int_value = read_count(label + " integer value");
            expr->literal_suffix = read_field(label + " integer suffix");
            return expr;
        }
        if (kind == "bool") {
            expr->kind = ExprKind::Bool;
            expr->bool_value = read_bool(label + " bool value");
            return expr;
        }
        if (kind == "name") {
            expr->kind = ExprKind::Name;
            expr->name = read_field(label + " name");
            return expr;
        }
        if (kind == "unary") {
            expr->kind = ExprKind::Unary;
            expr->op = read_token_kind(label + " unary operator");
            if (!is_summary_const_unary_op(expr->op)) fail("unsupported constant summary unary operator");
            expr->operand = read_const_expr(label + " unary operand");
            return expr;
        }
        if (kind == "binary") {
            expr->kind = ExprKind::Binary;
            expr->op = read_token_kind(label + " binary operator");
            if (!is_summary_const_binary_op(expr->op)) fail("unsupported constant summary binary operator");
            expr->left = read_const_expr(label + " binary left operand");
            expr->right = read_const_expr(label + " binary right operand");
            return expr;
        }
        if (kind == "tuple") {
            expr->kind = ExprKind::Tuple;
            expr->args = read_const_expr_list(label + " tuple values");
            return expr;
        }
        if (kind == "vector") {
            expr->kind = ExprKind::Vector;
            expr->args = read_const_expr_list(label + " vector values");
            return expr;
        }
        if (kind == "struct") {
            expr->kind = ExprKind::StructLiteral;
            expr->name = read_field(label + " struct name");
            expr->type_args = read_type_arguments(label + " struct type arguments");
            std::uint64_t field_count = read_count(label + " struct field count");
            expr->field_names.reserve(static_cast<std::size_t>(field_count));
            expr->args.reserve(static_cast<std::size_t>(field_count));
            for (std::uint64_t i = 0; i < field_count; ++i) {
                expr->field_names.push_back(read_field(label + " struct field name"));
                expr->args.push_back(read_const_expr(label + " struct field value"));
            }
            return expr;
        }
        if (kind == "call") {
            expr->kind = ExprKind::Call;
            expr->name = read_field(label + " call name");
            expr->type_args = read_type_arguments(label + " call type arguments");
            expr->args = read_const_expr_list(label + " call arguments");
            return expr;
        }
        fail("unknown constant expression summary kind '" + kind + "'");
    }

    ExprPtr read_const_initializer(const std::string& label) {
        if (!read_bool(label + " flag")) return nullptr;
        return read_const_expr(label);
    }

    void skip_const_initializer(const std::string& label) {
        (void)read_const_initializer(label);
    }
};

std::string declaration_summary_payload(const Program& program) {
    std::ostringstream out;
    out << "ari-ast-decls-v2;";

    append_count(out, program.uses.size());
    for (const auto& decl : program.uses) {
        append_field(out, decl.module_name);
        append_field(out, decl.path);
        append_field(out, decl.alias);
        append_bool(out, decl.is_public);
        append_bool(out, decl.is_glob);
    }

    append_count(out, program.module_imports.size());
    for (const auto& decl : program.module_imports) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_field(out, decl.local_name);
        append_bool(out, decl.is_public);
    }

    append_count(out, program.modules.size());
    for (const auto& decl : program.modules) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
    }

    append_count(out, program.constants.size());
    for (const auto& decl : program.constants) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_type(out, decl.type);
        append_const_initializer(out, decl.init);
    }

    append_count(out, program.functions.size());
    for (const auto& fn : program.functions) append_function_signature(out, fn);

    append_count(out, program.structs.size());
    for (const auto& decl : program.structs) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_bool(out, decl.tuple_struct);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        append_count(out, decl.fields.size());
        for (const auto& field : decl.fields) {
            append_field(out, field.name);
            append_bool(out, field.mutable_field);
            append_type(out, field.type);
        }
    }

    append_count(out, program.enums.size());
    for (const auto& decl : program.enums) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        append_count(out, decl.cases.size());
        for (const auto& item : decl.cases) {
            append_field(out, item.name);
            append_count(out, item.payloads.size());
            for (const auto& payload : item.payloads) append_type(out, payload);
        }
    }

    append_count(out, program.traits.size());
    for (const auto& decl : program.traits) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        append_count(out, decl.methods.size());
        for (const auto& method : decl.methods) append_function_signature(out, method);
    }

    append_count(out, program.impls.size());
    for (const auto& decl : program.impls) {
        append_field(out, decl.module_name);
        append_bool(out, decl.is_public);
        append_bool(out, decl.has_trait);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        if (decl.has_trait) append_type(out, decl.trait_type);
        append_type(out, decl.for_type);
        append_count(out, decl.methods.size());
        for (const auto& method : decl.methods) append_function_signature(out, method);
    }

    return out.str();
}

DeclarationSummaryCounts parse_declaration_summary_payload(const ModuleCacheAstSummary& summary) {
    DeclarationSummaryReader reader(summary.declaration_summary, summary_display(summary));
    return reader.parse();
}

Program materialize_declaration_summary_payload(const ModuleCacheAstSummary& summary) {
    DeclarationSummaryReader reader(summary.declaration_summary, summary_display(summary));
    return reader.parse_program();
}

void require_count_match(std::uint64_t recorded,
                         std::uint64_t parsed,
                         const std::string& label,
                         const ModuleCacheAstSummary& summary) {
    if (recorded == parsed) return;
    throw CompileError("module cache AST summary for " + summary_display(summary) +
                       " has " + label + " count " + std::to_string(recorded) +
                       " but declaration summary contains " + std::to_string(parsed));
}

} // namespace

ModuleCacheAstSummary make_module_cache_ast_summary(const std::string& path,
                                                    const std::string& content_hash,
                                                    const std::vector<std::string>& module_path,
                                                    const Program& program,
                                                    bool is_root) {
    ModuleCacheAstSummary summary;
    summary.module_name = join_qualified_path(module_path);
    summary.path = path;
    summary.content_hash = content_hash;
    summary.declaration_summary = declaration_summary_payload(program);
    summary.declaration_hash = module_metadata_source_hash(summary.declaration_summary);
    summary.is_root = is_root;
    summary.use_count = program.uses.size();
    summary.module_import_count = program.module_imports.size();
    summary.module_decl_count = program.modules.size();
    summary.constant_count = program.constants.size();
    summary.function_count = program.functions.size();
    summary.struct_count = program.structs.size();
    summary.enum_count = program.enums.size();
    summary.trait_count = program.traits.size();
    summary.impl_count = program.impls.size();
    return summary;
}

void require_valid_module_cache_ast_summary_payload(const ModuleCacheAstSummary& summary,
                                                    const std::string& display_path) {
    if (summary.declaration_summary.empty()) {
        throw CompileError("invalid module cache '" + display_path +
                           "': AST summary for " + summary_display(summary) +
                           " is missing a declaration summary");
    }
    std::string hash = module_metadata_source_hash(summary.declaration_summary);
    if (hash != summary.declaration_hash) {
        throw CompileError("invalid module cache '" + display_path +
                           "': AST summary for " + summary_display(summary) +
                           " declaration summary hashes to '" + hash +
                           "' instead of recorded '" + summary.declaration_hash + "'");
    }
    try {
        DeclarationSummaryCounts counts = parse_declaration_summary_payload(summary);
        require_count_match(summary.use_count, counts.use_count, "use", summary);
        require_count_match(summary.module_import_count, counts.module_import_count, "module import", summary);
        require_count_match(summary.module_decl_count, counts.module_decl_count, "module declaration", summary);
        require_count_match(summary.constant_count, counts.constant_count, "constant", summary);
        require_count_match(summary.function_count, counts.function_count, "function", summary);
        require_count_match(summary.struct_count, counts.struct_count, "struct", summary);
        require_count_match(summary.enum_count, counts.enum_count, "enum", summary);
        require_count_match(summary.trait_count, counts.trait_count, "trait", summary);
        require_count_match(summary.impl_count, counts.impl_count, "impl", summary);
        Program declarations = materialize_declaration_summary_payload(summary);
        ModuleCacheAstSummary round_trip = make_module_cache_ast_summary(
            summary.path,
            summary.content_hash,
            split_qualified_path(summary.module_name),
            declarations,
            summary.is_root
        );
        if (round_trip.declaration_summary != summary.declaration_summary ||
            round_trip.declaration_hash != summary.declaration_hash) {
            throw CompileError("declaration summary does not round-trip through materialized declarations");
        }
    } catch (const CompileError& error) {
        throw CompileError("invalid module cache '" + display_path + "': " + error.what());
    }
}

Program materialize_module_cache_ast_summary_declarations(const ModuleCacheAstSummary& summary,
                                                          const std::string& display_path) {
    try {
        return materialize_declaration_summary_payload(summary);
    } catch (const CompileError& error) {
        throw CompileError("invalid module cache '" + display_path + "': " + error.what());
    }
}

bool can_load_module_cache_ast_summary_declarations(const Program& program) {
    for (const auto& decl : program.constants) {
        if (!decl.init) return false;
    }
    for (const auto& fn : program.functions) {
        if (fn.has_body && !fn.is_extern) return false;
    }
    for (const auto& impl : program.impls) {
        for (const auto& method : impl.methods) {
            if (method.has_body) return false;
        }
    }
    return true;
}

} // namespace ari
