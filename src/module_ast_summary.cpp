#include "module_ast_summary.hpp"

#include "common.hpp"
#include "module_metadata.hpp"
#include "module_path.hpp"

#include <cstdint>
#include <limits>
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
        consume_literal("ari-ast-decls-v1;");
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

private:
    const std::string& text_;
    std::string display_;
    std::size_t pos_ = 0;

    [[noreturn]] void fail(const std::string& detail) const {
        throw CompileError("malformed declaration summary for " + display_ + ": " + detail);
    }

    void consume_literal(const std::string& literal) {
        if (text_.compare(pos_, literal.size(), literal) != 0) {
            fail("expected '" + literal + "'");
        }
        pos_ += literal.size();
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

    void skip_type(const std::string& label) {
        read_field(label + " qualifier");
        read_field(label + " name");
        read_bool(label + " dyn flag");
        read_bool(label + " nullable flag");
        read_count(label + " array size");
        std::uint64_t arg_count = read_count(label + " argument count");
        for (std::uint64_t i = 0; i < arg_count; ++i) skip_type(label + " argument");
    }

    void skip_generics() {
        std::uint64_t count = read_count("generic parameter count");
        for (std::uint64_t i = 0; i < count; ++i) {
            read_field("generic parameter name");
            bool has_constraint = read_bool("generic constraint flag");
            if (has_constraint) skip_type("generic constraint type");
        }
    }

    void skip_attributes() {
        std::uint64_t count = read_count("attribute count");
        for (std::uint64_t i = 0; i < count; ++i) {
            read_field("attribute name");
            read_bool("attribute args flag");
            std::uint64_t arg_count = read_count("attribute token count");
            for (std::uint64_t j = 0; j < arg_count; ++j) {
                read_count("attribute token kind");
                read_field("attribute token text");
            }
        }
    }

    void skip_function_signature() {
        read_field("function module name");
        read_field("function name");
        read_bool("function meta flag");
        read_bool("function extern flag");
        read_bool("function visibility");
        read_bool("function variadic flag");
        read_field("function extern ABI");
        read_field("function extern link name");
        bool has_return_type = read_bool("function return type flag");
        read_bool("function body flag");
        skip_generics();
        skip_attributes();
        std::uint64_t param_count = read_count("function parameter count");
        for (std::uint64_t i = 0; i < param_count; ++i) {
            read_field("function parameter name");
            read_bool("function parameter pattern flag");
            skip_type("function parameter type");
        }
        if (has_return_type) skip_type("function return type");
    }
};

std::string declaration_summary_payload(const Program& program) {
    std::ostringstream out;
    out << "ari-ast-decls-v1;";

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
    } catch (const CompileError& error) {
        throw CompileError("invalid module cache '" + display_path + "': " + error.what());
    }
}

} // namespace ari
