#include "parser.hpp"

#include "ast_builders.hpp"
#include "ast_clone.hpp"
#include "cfg_eval.hpp"
#include "module_path.hpp"

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ari {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}
    Parser(std::vector<Token> tokens, std::vector<std::string> module_path)
        : tokens_(std::move(tokens)), current_module_(std::move(module_path)) {}
    Parser(std::vector<Token> tokens,
           std::vector<std::string> module_path,
           std::set<std::string> cfg_features,
           std::string target_triple = {})
        : tokens_(std::move(tokens)),
          current_module_(std::move(module_path)),
          cfg_features_(std::move(cfg_features)),
          target_triple_(std::move(target_triple)) {}

    Program parse_program() {
        Program program;
        while (!check(TokenKind::End)) {
            parse_top_level_decl(program);
        }
        return program;
    }

    std::vector<ExprPtr> parse_expression_arguments_until_end(SourceLocation loc) {
        std::vector<ExprPtr> args;
        if (check(TokenKind::End)) return args;
        while (true) {
            args.push_back(parse_expression());
            if (check(TokenKind::End)) return args;
            expect(TokenKind::Comma, "expected , or end of macro invocation arguments");
            if (check(TokenKind::End)) return args;
        }
        fail(loc, "unreachable macro argument parser state");
    }

private:
    std::vector<Token> tokens_;
    std::size_t pos_ = 0;
    std::vector<std::string> current_module_;
    std::set<std::string> cfg_features_;
    std::string target_triple_;
    bool allow_struct_literals_ = true;

    const Token& peek(int offset = 0) const {
        long long at = static_cast<long long>(pos_) + static_cast<long long>(offset);
        if (at < 0) return tokens_.front();
        if (static_cast<std::size_t>(at) >= tokens_.size()) return tokens_.back();
        return tokens_[static_cast<std::size_t>(at)];
    }

    bool check(TokenKind kind) const {
        return peek().kind == kind;
    }

    bool match(TokenKind kind) {
        if (!check(kind)) return false;
        ++pos_;
        return true;
    }

    Token expect(TokenKind kind, const std::string& message) {
        if (!check(kind)) fail(peek().loc, message);
        return tokens_[pos_++];
    }

    Token expect_identifier_or_contextual_name_keyword(const std::string& message) {
        if (!check(TokenKind::Identifier) && !check(TokenKind::KwDrop) && !check(TokenKind::KwNext)) {
            fail(peek().loc, message);
        }
        return tokens_[pos_++];
    }

    [[noreturn]] static void fail(SourceLocation loc, const std::string& message) {
        throw CompileError(where(loc) + ": " + message);
    }

    void optional_separator() {
        match(TokenKind::Comma);
        match(TokenKind::Semicolon);
    }

    void aggregate_member_separator(const std::string& message, const std::string& semicolon_message) {
        if (match(TokenKind::Comma)) return;
        if (check(TokenKind::RBrace)) return;
        if (check(TokenKind::Semicolon)) fail(peek().loc, semicolon_message);
        fail(peek().loc, message);
    }

    void require_semicolon(const std::string& message) {
        expect(TokenKind::Semicolon, message);
    }

    std::string current_module_name() const {
        return join_qualified_path(current_module_);
    }

    std::string qualify_name(const std::string& name) const {
        if (current_module_.empty()) return name;
        return current_module_name() + "::" + name;
    }

    std::string parse_path_after_first(const Token& first) {
        std::string path = first.text;
        while (match(TokenKind::ColonColon)) {
            Token part = expect_identifier_or_contextual_name_keyword("expected name after ::");
            path += "::" + part.text;
        }
        return path;
    }

    std::string parse_path(const std::string& message) {
        Token first = expect(TokenKind::Identifier, message);
        return parse_path_after_first(first);
    }

    static bool is_cfg_attribute(const Attribute& attr) {
        return attr.name == "cfg";
    }

    static void reject_attributes_except_cfg(const std::vector<Attribute>& attributes, const std::string& target) {
        for (const auto& attr : attributes) {
            if (is_cfg_attribute(attr)) continue;
            fail(attr.loc, "attributes are not supported on " + target);
        }
    }

    bool cfg_attributes_enabled(const std::vector<Attribute>& attributes) const {
        for (const auto& attr : attributes) {
            if (!is_cfg_attribute(attr)) continue;
            if (!cfg_attribute_enabled(attr, cfg_features_, target_triple_)) return false;
        }
        return true;
    }

    void parse_top_level_decl(Program& program) {
        std::vector<Attribute> attributes = parse_attributes();
        bool cfg_enabled = cfg_attributes_enabled(attributes);
        Program discarded;
        Program& target = cfg_enabled ? program : discarded;
        bool public_decl = match(TokenKind::KwPub);
        bool meta = match(TokenKind::KwMeta);
        if (meta) {
            expect(TokenKind::KwFn, "expected fn after meta");
            target.functions.push_back(parse_function(true, true, public_decl, std::move(attributes)));
            return;
        }
        if (match(TokenKind::KwExtern)) {
            target.functions.push_back(parse_extern_function(public_decl, std::move(attributes)));
        } else if (match(TokenKind::KwUse)) {
            if (cfg_enabled) reject_attributes_except_cfg(attributes, "use declarations");
            parse_use(target, public_decl);
        } else if (match(TokenKind::KwMod)) {
            if (cfg_enabled) reject_attributes_except_cfg(attributes, "module declarations");
            parse_module(target, public_decl);
        } else if (match(TokenKind::KwConst)) {
            if (cfg_enabled) reject_attributes_except_cfg(attributes, "constant declarations");
            target.constants.push_back(parse_const(public_decl));
        } else if (match(TokenKind::KwFn)) {
            target.functions.push_back(parse_function(false, true, public_decl, std::move(attributes)));
        } else if (match(TokenKind::KwStruct)) {
            target.structs.push_back(parse_struct(public_decl, std::move(attributes)));
        } else if (match(TokenKind::KwEnum)) {
            target.enums.push_back(parse_enum(public_decl, std::move(attributes)));
        } else if (match(TokenKind::KwTrait)) {
            target.traits.push_back(parse_trait(public_decl, std::move(attributes)));
        } else if (match(TokenKind::KwImpl)) {
            target.impls.push_back(parse_impl(public_decl, std::move(attributes)));
        } else if (check(TokenKind::Identifier) && peek(1).kind == TokenKind::Bang) {
            if (cfg_enabled) reject_attributes_except_cfg(attributes, "item macro invocations");
            Token name = expect(TokenKind::Identifier, "expected item macro invocation name");
            Token bang = expect(TokenKind::Bang, "expected ! after item macro invocation name");
            std::vector<Token> tokens = parse_macro_token_tree(bang.loc);
            match(TokenKind::Semicolon);
            if (cfg_enabled) {
                ItemMacroInvocation invocation;
                invocation.name = name.text;
                invocation.module_name = current_module_name();
                invocation.is_public = public_decl;
                invocation.attributes = std::move(attributes);
                invocation.tokens = std::move(tokens);
                invocation.loc = name.loc;
                target.item_macros.push_back(std::move(invocation));
            }
        } else {
            fail(peek().loc, "expected top-level declaration");
        }
    }

    ConstDecl parse_const(bool public_decl) {
        Token name = expect(TokenKind::Identifier, "expected constant name");
        ConstDecl decl;
        decl.name = qualify_name(name.text);
        decl.module_name = current_module_name();
        decl.is_public = public_decl;
        decl.loc = name.loc;
        expect(TokenKind::Colon, "expected : after constant name");
        decl.type = parse_type();
        expect(TokenKind::Equal, "expected = in constant declaration");
        decl.init = parse_expression();
        require_semicolon("expected ; after constant declaration");
        return decl;
    }

    FunctionDecl parse_extern_function(bool public_decl, std::vector<Attribute> attributes) {
        std::string abi_name = "C";
        if (check(TokenKind::String)) {
            Token abi = tokens_[pos_++];
            if (abi.text != "C" && abi.text != "ari") {
                fail(abi.loc, "extern ABI must be \"C\" or \"ari\"");
            }
            abi_name = abi.text;
        }
        expect(TokenKind::KwFn, "expected fn after extern ABI");
        FunctionDecl fn = parse_function(false, false, public_decl, std::move(attributes));
        if (check(TokenKind::LBrace)) {
            fail(peek().loc, "extern functions cannot have a body");
        }
        fn.is_extern = true;
        fn.extern_abi = std::move(abi_name);
        if (match(TokenKind::Equal)) {
            Token link = expect(TokenKind::String, "expected external link name string after =");
            fn.extern_link_name = link.text;
            expect(TokenKind::Semicolon, "expected ; after external link name");
        }
        return fn;
    }

    void parse_use(Program& program, bool public_decl) {
        SourceLocation loc = peek().loc;
        parse_use_tree(program, public_decl, "", loc);
        match(TokenKind::Semicolon);
    }

    void add_use_decl(
        Program& program,
        bool public_decl,
        const std::string& path,
        const std::string& alias,
        SourceLocation loc,
        bool is_glob
    ) {
        UseDecl decl;
        decl.path = path;
        decl.alias = alias;
        decl.module_name = current_module_name();
        decl.is_public = public_decl;
        decl.is_glob = is_glob;
        decl.loc = loc;
        program.uses.push_back(std::move(decl));
    }

    void parse_use_tree(Program& program, bool public_decl, const std::string& prefix, SourceLocation loc) {
        Token first = expect(TokenKind::Identifier, "expected path after use");
        std::string path = prefix.empty() ? first.text : prefix + "::" + first.text;

        while (match(TokenKind::ColonColon)) {
            if (match(TokenKind::Star)) {
                add_use_decl(program, public_decl, path, "", loc, true);
                return;
            }
            if (match(TokenKind::LBrace)) {
                parse_use_group(program, public_decl, path, loc);
                return;
            }
            Token next = expect(TokenKind::Identifier, "expected path segment after ::");
            path += "::" + next.text;
        }

        std::string alias = qualified_basename(path);
        if (match(TokenKind::KwAs)) {
            Token alias = expect(TokenKind::Identifier, "expected alias after as");
            add_use_decl(program, public_decl, path, alias.text, loc, false);
            return;
        }
        add_use_decl(program, public_decl, path, alias, loc, false);
    }

    void parse_use_group(Program& program, bool public_decl, const std::string& prefix, SourceLocation loc) {
        if (check(TokenKind::RBrace)) fail(peek().loc, "use groups must contain at least one item");
        do {
            if (match(TokenKind::Star)) {
                add_use_decl(program, public_decl, prefix, "", loc, true);
            } else {
                parse_use_tree(program, public_decl, prefix, loc);
            }
        } while (match(TokenKind::Comma));
        expect(TokenKind::RBrace, "expected } after use group");
    }

    void parse_module(Program& program, bool public_decl) {
        Token name = expect(TokenKind::Identifier, "expected module name");
        std::string local_name = name.text;
        std::string module_name = name.text;
        if (match(TokenKind::KwAs)) {
            Token alias = expect(TokenKind::Identifier, "expected module alias after as");
            module_name = alias.text;
        }
        ModuleDecl decl;
        decl.name = qualify_name(module_name);
        decl.module_name = current_module_name();
        decl.is_public = public_decl;
        decl.loc = name.loc;
        program.modules.push_back(std::move(decl));

        if (match(TokenKind::Semicolon)) {
            ModuleImport import;
            import.name = qualify_name(module_name);
            import.local_name = local_name;
            import.module_name = current_module_name();
            import.is_public = public_decl;
            import.loc = name.loc;
            program.module_imports.push_back(std::move(import));
            return;
        }
        if (module_name != local_name) fail(name.loc, "inline modules cannot be aliased; use file-backed `mod name as alias;`");
        expect(TokenKind::LBrace, "expected { after module name");
        current_module_.push_back(name.text);
        while (!match(TokenKind::RBrace)) {
            if (check(TokenKind::End)) fail(name.loc, "unterminated module");
            parse_top_level_decl(program);
        }
        current_module_.pop_back();
    }

    std::vector<Attribute> parse_attributes() {
        std::vector<Attribute> attributes;
        while (match(TokenKind::At)) {
            Token name = expect(TokenKind::Identifier, "expected attribute name after @");
            Attribute attr;
            attr.name = parse_path_after_first(name);
            attr.loc = name.loc;
            if (check(TokenKind::LParen)) {
                attr.has_args = true;
                attr.args = parse_attribute_token_tree(attr.loc);
            }
            attributes.push_back(std::move(attr));
        }
        return attributes;
    }

    std::vector<GenericParam> parse_generics() {
        std::vector<GenericParam> generics;
        if (!match(TokenKind::LBracket)) return generics;
        if (!check(TokenKind::RBracket)) {
            do {
                Token name = expect(TokenKind::Identifier, "expected generic parameter name");
                GenericParam param;
                param.name = name.text;
                param.loc = name.loc;
                if (match(TokenKind::Colon)) {
                    param.constraint = parse_type();
                    param.has_constraint = true;
                }
                generics.push_back(std::move(param));
            } while (match(TokenKind::Comma));
        }
        expect(TokenKind::RBracket, "expected ] after generic parameter list");
        return generics;
    }

    TypeRef parse_type() {
        TypeQualifier qualifier = TypeQualifier::Value;
        SourceLocation loc = peek().loc;
        if (match(TokenKind::KwOwn)) {
            qualifier = TypeQualifier::Own;
        } else if (match(TokenKind::KwRef)) {
            qualifier = match(TokenKind::KwMut) ? TypeQualifier::MutRef : TypeQualifier::Ref;
        } else if (match(TokenKind::KwMut)) {
            expect(TokenKind::KwRef, "expected ref after mut in mutable reference type");
            qualifier = TypeQualifier::MutRef;
        } else if (match(TokenKind::KwPtr)) {
            qualifier = TypeQualifier::Ptr;
        }

        if (match(TokenKind::KwFn)) {
            Token fn = tokens_[pos_ - 1];
            TypeRef type;
            type.qualifier = qualifier;
            type.name = "fn";
            type.loc = loc;
            expect(TokenKind::LParen, "expected ( after fn in function pointer type");
            if (!check(TokenKind::RParen)) {
                do {
                    type.args.push_back(parse_type());
                } while (match(TokenKind::Comma));
            }
            expect(TokenKind::RParen, "expected ) after function pointer parameter types");
            type.array_size = type.args.size();
            if (match(TokenKind::Arrow)) {
                type.args.push_back(parse_type());
            } else {
                TypeRef result;
                result.name = "void";
                result.loc = fn.loc;
                type.args.push_back(std::move(result));
            }
            return finish_type(std::move(type));
        }

        if (match(TokenKind::KwDyn)) {
            Token dyn = tokens_[pos_ - 1];
            Token name = expect(TokenKind::Identifier, "expected trait name after dyn");
            TypeRef type;
            type.qualifier = qualifier;
            type.name = parse_path_after_first(name);
            type.loc = qualifier == TypeQualifier::Value ? dyn.loc : loc;
            type.is_dyn_object = true;
            if (match(TokenKind::LBracket)) {
                if (!check(TokenKind::RBracket)) {
                    do {
                        type.args.push_back(parse_type());
                    } while (match(TokenKind::Comma));
                }
                expect(TokenKind::RBracket, "expected ] after dyn trait type arguments");
            }
            return finish_type(std::move(type));
        }

        if (match(TokenKind::LBracket)) {
            TypeRef type;
            type.qualifier = qualifier;
            type.name = "Array";
            type.loc = loc;
            type.args.push_back(parse_type());
            expect(TokenKind::Comma, "expected , between array element type and size");
            Token size = expect(TokenKind::Integer, "expected integer array size");
            if (size.int_value == 0) fail(size.loc, "array types require a size greater than zero");
            type.array_size = size.int_value;
            expect(TokenKind::RBracket, "expected ] after array type");
            return finish_type(std::move(type));
        }

        if (match(TokenKind::LParen)) {
            TypeRef type;
            type.qualifier = qualifier;
            type.name = "Tuple";
            type.loc = loc;
            if (match(TokenKind::RParen)) return finish_type(std::move(type));
            do {
                type.args.push_back(parse_type());
            } while (match(TokenKind::Comma));
            expect(TokenKind::RParen, "expected ) after tuple type");
            if (type.args.size() == 1) fail(loc, "single-element tuple types are not supported");
            return finish_type(std::move(type));
        }

        Token name = expect(TokenKind::Identifier, "expected type name");
        TypeRef type;
        type.qualifier = qualifier;
        type.name = parse_path_after_first(name);
        type.loc = qualifier == TypeQualifier::Value ? name.loc : loc;
        if (match(TokenKind::Bang)) {
            Token bang = tokens_[pos_ - 1];
            type.is_macro_invocation = true;
            type.macro_tokens = parse_macro_token_tree(bang.loc);
        } else if (match(TokenKind::LBracket)) {
            if (!check(TokenKind::RBracket)) {
                do {
                    type.args.push_back(parse_type());
                } while (match(TokenKind::Comma));
            }
            expect(TokenKind::RBracket, "expected ] after type arguments");
        }
        return finish_type(std::move(type));
    }

    TypeRef finish_type(TypeRef type) {
        if (match(TokenKind::Question)) {
            type.nullable = true;
            if (check(TokenKind::Question)) {
                fail(peek().loc, "nullable type suffix ? can only appear once");
            }
        }
        return type;
    }

    FunctionDecl parse_function(
        bool meta,
        bool body_allowed,
        bool public_decl,
        std::vector<Attribute> attributes = {}
    ) {
        Token name = expect_identifier_or_contextual_name_keyword("expected function name");
        FunctionDecl fn;
        fn.name = qualify_name(name.text);
        fn.module_name = current_module_name();
        fn.loc = name.loc;
        fn.meta = meta;
        fn.is_public = public_decl;
        fn.attributes = std::move(attributes);
        fn.generics = parse_generics();
        expect(TokenKind::LParen, "expected ( after function name");
        if (!check(TokenKind::RParen)) {
            do {
                if (match(TokenKind::Ellipsis)) {
                    fn.is_variadic = true;
                    fn.variadic_loc = tokens_[pos_ - 1].loc;
                    if (!check(TokenKind::RParen)) {
                        fail(peek().loc, "variadic ... marker must be the final function parameter");
                    }
                    break;
                }
                fn.params.push_back(parse_function_param());
            } while (match(TokenKind::Comma));
        }
        expect(TokenKind::RParen, "expected ) after parameter list");
        if (match(TokenKind::Arrow)) {
            fn.return_type = parse_type();
            fn.has_return_type = true;
        }
        if (body_allowed && check(TokenKind::LBrace)) {
            fn.body = parse_function_block();
            fn.has_body = true;
        } else {
            match(TokenKind::Semicolon);
            fn.has_body = false;
        }
        return fn;
    }

    Param parse_function_param() {
        if (check(TokenKind::Identifier)) {
            Token param_name = peek();
            if (param_name.text == "self" && peek(1).kind != TokenKind::Colon) {
                ++pos_;
                TypeRef self_type;
                self_type.name = "Self";
                self_type.loc = param_name.loc;
                Param param;
                param.name = param_name.text;
                param.type = std::move(self_type);
                return param;
            }
            if (param_name.text != "_" && peek(1).kind == TokenKind::Colon) {
                ++pos_;
                expect(TokenKind::Colon, "expected : after parameter name");
                Param param;
                param.name = param_name.text;
                param.type = parse_type();
                return param;
            }
        }

        Pattern pattern = parse_pattern(true);
        SourceLocation loc = pattern.loc;
        expect(TokenKind::Colon, "expected : after parameter pattern");
        Param param;
        param.name = "$pattern";
        param.type = parse_type();
        param.has_pattern = true;
        param.pattern = std::move(pattern);
        if (param.pattern.kind == PatternKind::Binding) {
            param.name = param.pattern.payload_name;
            param.has_pattern = false;
        }
        if (!param.has_pattern) param.type.loc = loc;
        return param;
    }

    StructDecl parse_struct(bool public_decl, std::vector<Attribute> attributes) {
        Token name = expect(TokenKind::Identifier, "expected struct name");
        StructDecl decl;
        decl.name = qualify_name(name.text);
        decl.module_name = current_module_name();
        decl.is_public = public_decl;
        decl.loc = name.loc;
        decl.attributes = std::move(attributes);
        decl.generics = parse_generics();

        if (match(TokenKind::LParen)) {
            decl.tuple_struct = true;
            std::size_t index = 0;
            if (check(TokenKind::RParen)) fail(name.loc, "tuple structs require at least one field");
            while (true) {
                bool mutable_field = match(TokenKind::KwMut);
                TypeRef field_type = parse_type();
                SourceLocation field_loc = field_type.loc;
                decl.fields.push_back(StructField{
                    std::to_string(index),
                    std::move(field_type),
                    mutable_field,
                    field_loc
                });
                ++index;
                if (!match(TokenKind::Comma)) break;
                if (check(TokenKind::RParen)) break;
            }
            expect(TokenKind::RParen, "expected ) after tuple struct fields");
            if (check(TokenKind::Semicolon)) fail(peek().loc, "tuple struct declarations do not end with ;");
            return decl;
        }

        expect(TokenKind::LBrace, "expected { after struct name");
        while (!match(TokenKind::RBrace)) {
            if (check(TokenKind::End)) fail(peek().loc, "unterminated struct");
            bool mutable_field = match(TokenKind::KwMut);
            Token field = expect(TokenKind::Identifier, "expected struct field name");
            expect(TokenKind::Colon, "expected : after field name");
            decl.fields.push_back(StructField{field.text, parse_type(), mutable_field, field.loc});
            aggregate_member_separator(
                "expected , or } after struct field",
                "expected , after struct field; struct fields are comma-separated");
        }
        return decl;
    }

    EnumDecl parse_enum(bool public_decl, std::vector<Attribute> attributes) {
        Token name = expect(TokenKind::Identifier, "expected enum name");
        EnumDecl decl;
        decl.name = qualify_name(name.text);
        decl.module_name = current_module_name();
        decl.is_public = public_decl;
        decl.loc = name.loc;
        decl.attributes = std::move(attributes);
        decl.generics = parse_generics();
        expect(TokenKind::LBrace, "expected { after enum name");
        while (!match(TokenKind::RBrace)) {
            if (check(TokenKind::End)) fail(peek().loc, "unterminated enum");
            Token case_name = expect(TokenKind::Identifier, "expected enum case name");
            EnumCase item;
            item.name = case_name.text;
            item.loc = case_name.loc;
            if (match(TokenKind::LParen)) {
                while (!check(TokenKind::RParen)) {
                    item.payloads.push_back(parse_type());
                    if (!match(TokenKind::Comma)) break;
                }
                expect(TokenKind::RParen, "expected ) after enum payload list");
            }
            decl.cases.push_back(std::move(item));
            aggregate_member_separator(
                "expected , or } after enum case",
                "expected , after enum case; enum cases are comma-separated");
        }
        return decl;
    }

    TraitDecl parse_trait(bool public_decl, std::vector<Attribute> attributes) {
        Token name = expect(TokenKind::Identifier, "expected trait name");
        TraitDecl decl;
        decl.name = qualify_name(name.text);
        decl.module_name = current_module_name();
        decl.is_public = public_decl;
        decl.loc = name.loc;
        decl.attributes = std::move(attributes);
        decl.generics = parse_generics();
        if (match(TokenKind::Colon)) {
            do {
                decl.supertraits.push_back(parse_type());
            } while (match(TokenKind::Plus));
        }
        expect(TokenKind::LBrace, "expected { after trait name");
        while (!match(TokenKind::RBrace)) {
            expect(TokenKind::KwFn, "expected trait method");
            decl.methods.push_back(parse_function(false, false, false));
            optional_separator();
        }
        return decl;
    }

    ImplDecl parse_impl(bool public_decl, std::vector<Attribute> attributes) {
        ImplDecl decl;
        decl.module_name = current_module_name();
        decl.is_public = public_decl;
        decl.attributes = std::move(attributes);
        decl.generics = parse_generics();
        TypeRef first = parse_type();
        if (match(TokenKind::KwFor)) {
            decl.has_trait = true;
            decl.trait_type = std::move(first);
            decl.for_type = parse_type();
        } else {
            decl.for_type = std::move(first);
        }
        expect(TokenKind::LBrace, "expected { after impl header");
        while (!match(TokenKind::RBrace)) {
            bool method_public = match(TokenKind::KwPub);
            expect(TokenKind::KwFn, "expected function in impl block");
            decl.methods.push_back(parse_function(false, true, method_public));
            optional_separator();
        }
        return decl;
    }

    std::vector<StmtPtr> parse_block() {
        expect(TokenKind::LBrace, "expected {");
        std::vector<StmtPtr> body;
        while (!match(TokenKind::RBrace)) {
            if (check(TokenKind::End)) fail(peek().loc, "unterminated block");
            body.push_back(parse_statement());
        }
        return body;
    }

    std::vector<StmtPtr> parse_function_block() {
        expect(TokenKind::LBrace, "expected {");
        std::vector<StmtPtr> body;
        while (!match(TokenKind::RBrace)) {
            if (check(TokenKind::End)) fail(peek().loc, "unterminated function body");
            body.push_back(parse_function_body_item());
        }
        return body;
    }

    bool skip_balanced_assignment_scan(int& offset, TokenKind open, TokenKind close) const {
        int depth = 1;
        ++offset;
        while (depth > 0) {
            TokenKind kind = peek(offset).kind;
            if (kind == TokenKind::End || kind == TokenKind::Semicolon) return false;
            if (kind == open) ++depth;
            else if (kind == close) --depth;
            ++offset;
        }
        return true;
    }

    int skip_assignment_postfixes(int offset) const {
        for (;;) {
            if (peek(offset).kind == TokenKind::Dot) {
                TokenKind field = peek(offset + 1).kind;
                if (field != TokenKind::Identifier && field != TokenKind::Integer) return -1;
                offset += 2;
                continue;
            }
            if (peek(offset).kind == TokenKind::LBracket) {
                if (!skip_balanced_assignment_scan(offset, TokenKind::LBracket, TokenKind::RBracket)) return -1;
                continue;
            }
            break;
        }
        return offset;
    }

    bool is_assignment_statement_start() const {
        int offset = 0;
        if (check(TokenKind::Star)) {
            if (peek(1).kind == TokenKind::Identifier && is_assignment_operator(peek(2).kind)) return true;
            offset = 1;
            if (peek(offset).kind == TokenKind::LParen) {
                if (!skip_balanced_assignment_scan(offset, TokenKind::LParen, TokenKind::RParen)) return false;
                offset = skip_assignment_postfixes(offset);
                return offset >= 0 && is_assignment_operator(peek(offset).kind);
            }
            return false;
        }

        if (check(TokenKind::LParen)) {
            if (!skip_balanced_assignment_scan(offset, TokenKind::LParen, TokenKind::RParen)) return false;
            offset = skip_assignment_postfixes(offset);
            return offset >= 0 && is_assignment_operator(peek(offset).kind);
        }

        if (!check(TokenKind::Identifier)) return false;
        if (is_assignment_operator(peek(1).kind)) return true;

        offset = skip_assignment_postfixes(1);
        return offset > 1 && is_assignment_operator(peek(offset).kind);
    }

    StmtPtr parse_function_body_item() {
        if (!starts_expression(peek().kind) ||
            is_assignment_statement_start() ||
            (check(TokenKind::Identifier) && peek(1).kind == TokenKind::Colon)) {
            return parse_statement();
        }

        std::size_t saved_pos = pos_;
        bool statement_fallback = is_expression_statement_keyword(peek().kind);
        try {
            SourceLocation loc = peek().loc;
            ExprPtr value = parse_expression();
            if (match(TokenKind::Semicolon)) {
                auto stmt = std::make_unique<Stmt>();
                stmt->kind = StmtKind::ExprStmt;
                stmt->loc = loc;
                stmt->expr = std::move(value);
                return stmt;
            }
            if (check(TokenKind::RBrace)) {
                auto stmt = std::make_unique<Stmt>();
                stmt->kind = StmtKind::Return;
                stmt->loc = loc;
                stmt->expr = std::move(value);
                return stmt;
            }
            if (statement_fallback) {
                pos_ = saved_pos;
                return parse_statement();
            }
            fail(peek().loc, "expected ; after non-final expression statement");
        } catch (const CompileError&) {
            if (!statement_fallback) throw;
            pos_ = saved_pos;
            return parse_statement();
        }
    }

    static bool is_expression_statement_keyword(TokenKind kind) {
        return kind == TokenKind::KwIf ||
               kind == TokenKind::KwMatch ||
               kind == TokenKind::LBrace;
    }

    StmtPtr parse_statement() {
        if (match(TokenKind::LBrace)) {
            --pos_;
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = StmtKind::Block;
            set_stmt_statements(*stmt, parse_block());
            return stmt;
        }
        if (check(TokenKind::Identifier) && peek(1).kind == TokenKind::Colon) return parse_labeled_statement();
        if (match(TokenKind::KwLet)) return parse_let_or_variable();
        if (match(TokenKind::KwVar)) return parse_variable(true);
        if (match(TokenKind::KwReturn)) return parse_return();
        if (match(TokenKind::KwIf)) return parse_if();
        if (match(TokenKind::KwWhile)) return parse_while();
        if (match(TokenKind::KwFor)) return parse_for();
        if (match(TokenKind::KwInit)) return parse_init_while();
        if (match(TokenKind::KwContinue)) return parse_continue();
        if (match(TokenKind::KwBreak)) return parse_break();
        if (match(TokenKind::KwMatch)) return parse_match();
        if (match(TokenKind::KwDrop)) return parse_drop();

        if (is_assignment_statement_start()) {
            SourceLocation loc = peek().loc;
            ExprPtr target = parse_expression();
            Token op = tokens_[pos_++];
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = StmtKind::Assign;
            stmt->loc = loc;
            if (target->kind == ExprKind::Name) {
                set_stmt_assign_name(*stmt, target->name);
            } else if (is_assignment_target_expr(*target)) {
                set_stmt_assign_target(*stmt, std::move(target));
            } else {
                fail(loc, "assignment target must be a binding, field access, index access, or pointer dereference");
            }
            ExprPtr rhs = parse_expression();
            if (op.kind == TokenKind::Equal) {
                set_stmt_assign_rhs(*stmt, std::move(rhs));
            } else {
                const ExprPtr& assign_target = stmt_assign_target(*stmt);
                ExprPtr left = assign_target
                    ? clone_assignment_target(*assign_target)
                    : make_ast_name_expr(loc, stmt_assign_name(*stmt));
                set_stmt_assign_rhs(
                    *stmt,
                    make_ast_binary_expr(
                        op.loc,
                        compound_assignment_binary_operator(op.kind, op.loc),
                        std::move(left),
                        std::move(rhs)));
            }
            require_semicolon("expected ; after assignment");
            return stmt;
        }

        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::ExprStmt;
        stmt->loc = peek().loc;
        stmt->expr = parse_expression();
        require_semicolon("expected ; after expression statement");
        return stmt;
    }

    bool is_labelable_statement_start(TokenKind kind) const {
        return kind == TokenKind::KwWhile ||
               kind == TokenKind::KwFor ||
               kind == TokenKind::KwInit ||
               kind == TokenKind::LBrace;
    }

    StmtPtr parse_labeled_statement() {
        Token label = expect(TokenKind::Identifier, "expected label name");
        expect(TokenKind::Colon, "expected : after label");
        if (!is_labelable_statement_start(peek().kind)) {
            fail(peek().loc, "labels can only be attached to loops or blocks");
        }
        StmtPtr stmt = parse_statement();
        if (stmt->kind != StmtKind::While &&
            stmt->kind != StmtKind::WhileLet &&
            stmt->kind != StmtKind::For &&
            stmt->kind != StmtKind::InitWhile &&
            stmt->kind != StmtKind::Block) {
            fail(label.loc, "labels can only be attached to loops or blocks");
        }
        if (!stmt_label(*stmt).empty()) fail(label.loc, "statement already has a label");
        set_stmt_label(*stmt, label.text);
        return stmt;
    }

    Binding parse_named_binding(SourceLocation loc, bool mutable_binding) {
        Token name = expect(TokenKind::Identifier, "expected binding name");
        if (check(TokenKind::LParen)) {
            fail(peek().loc, "let/var pattern bindings are not supported yet; bind a name and use match for enum payloads");
        }
        Binding binding;
        binding.name = name.text;
        binding.loc = name.loc;
        binding.mutable_binding = mutable_binding;
        if (match(TokenKind::Colon)) {
            binding.type = parse_type();
            binding.has_type = true;
        }
        expect(TokenKind::Equal, "expected = in binding");
        binding.init = parse_expression();
        (void)loc;
        return binding;
    }

    StmtPtr parse_let_or_variable() {
        if (is_binding_pattern_start()) {
            return parse_pattern_variable(false);
        }

        Binding first = parse_named_binding(peek().loc, false);
        if ((check(TokenKind::Comma) || check(TokenKind::KwWhile)) &&
            first.init && peek().loc.line == first.init->loc.line) {
            fail(peek().loc, "let-while loop state was removed; use init ... while ... next");
        }

        require_semicolon("expected ; after binding");
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::VarDecl;
        stmt->loc = first.loc;
        stmt->binding = std::move(first);
        return stmt;
    }

    StmtPtr parse_variable(bool mutable_binding) {
        if (is_binding_pattern_start()) {
            return parse_pattern_variable(mutable_binding);
        }
        Token name = expect(TokenKind::Identifier, "expected variable name");
        if (check(TokenKind::LParen)) {
            fail(peek().loc, "let/var pattern bindings are not supported yet; bind a name and use match for enum payloads");
        }
        Binding binding;
        binding.name = name.text;
        binding.loc = name.loc;
        binding.mutable_binding = mutable_binding;
        if (match(TokenKind::Colon)) {
            binding.type = parse_type();
            binding.has_type = true;
        }
        expect(TokenKind::Equal, "expected = after variable name");
        binding.init = parse_expression();
        require_semicolon("expected ; after binding");

        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::VarDecl;
        stmt->loc = name.loc;
        stmt->binding = std::move(binding);
        return stmt;
    }

    bool is_binding_pattern_start() const {
        if (check(TokenKind::LBracket)) return true;
        if (check(TokenKind::LParen)) return true;
        if (check(TokenKind::KwRef)) return true;
        if (check(TokenKind::KwMut)) return true;
        if (check(TokenKind::Amp)) return true;
        if (!check(TokenKind::Identifier)) return false;
        if (peek().text == "_") return true;
        std::size_t look = pos_ + 1;
        while (look + 1 < tokens_.size() &&
               tokens_[look].kind == TokenKind::ColonColon &&
               tokens_[look + 1].kind == TokenKind::Identifier) {
            look += 2;
        }
        return look < tokens_.size() &&
               (tokens_[look].kind == TokenKind::At ||
                tokens_[look].kind == TokenKind::LBrace ||
                tokens_[look].kind == TokenKind::LParen);
    }

    void reject_pattern_binding_mode_start(const char* context) const {
        if (check(TokenKind::KwRef) || check(TokenKind::Amp)) {
            fail(peek().loc,
                 std::string("reference ") + context +
                     " binding modes are reserved but not supported yet; bind by value and borrow inside the body");
        }
        if (check(TokenKind::KwMut)) {
            fail(peek().loc,
                 std::string("mutable ") + context +
                     " binding modes are reserved but not supported yet; use var for mutable local bindings");
        }
    }

    StmtPtr parse_pattern_variable(bool mutable_binding) {
        Binding binding;
        binding.pattern = parse_pattern(true);
        binding.has_pattern = true;
        binding.mutable_binding = mutable_binding;
        binding.loc = binding.pattern.loc;
        if (match(TokenKind::Colon)) {
            binding.type = parse_type();
            binding.has_type = true;
        }
        expect(TokenKind::Equal, "expected = after pattern binding");
        binding.init = parse_expression();
        require_semicolon("expected ; after binding");

        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::VarDecl;
        stmt->loc = binding.loc;
        stmt->binding = std::move(binding);
        return stmt;
    }

    Pattern parse_binding_pattern() {
        Pattern pattern;
        pattern.loc = peek().loc;
        reject_pattern_binding_mode_start("pattern");
        if (match(TokenKind::LBracket)) {
            pattern.kind = PatternKind::Array;
            pattern.loc = tokens_[pos_ - 1].loc;
            while (!check(TokenKind::RBracket)) {
                if (match(TokenKind::DotDot)) {
                    if (pattern.has_rest) {
                        fail(tokens_[pos_ - 1].loc, "array binding pattern can contain only one '..' rest");
                    }
                    pattern.has_rest = true;
                    pattern.rest_index = pattern.elements.size();
                    if (!match(TokenKind::Comma)) break;
                    if (check(TokenKind::RBracket)) break;
                    continue;
                }
                pattern.elements.push_back(parse_binding_pattern());
                if (!match(TokenKind::Comma)) break;
                if (check(TokenKind::RBracket)) break;
            }
            expect(TokenKind::RBracket, "expected ] after array binding pattern");
            if (pattern.elements.empty() && !pattern.has_rest) {
                fail(pattern.loc, "array binding patterns must bind at least one element or use '..'");
            }
            return pattern;
        }
        if (match(TokenKind::LParen)) {
            pattern.kind = PatternKind::Tuple;
            pattern.loc = tokens_[pos_ - 1].loc;
            if (match(TokenKind::RParen)) return pattern;
            bool saw_comma = false;
            while (!check(TokenKind::RParen)) {
                if (match(TokenKind::DotDot)) {
                    if (pattern.has_rest) {
                        fail(tokens_[pos_ - 1].loc, "tuple binding pattern can contain only one '..' rest");
                    }
                    pattern.has_rest = true;
                    pattern.rest_index = pattern.elements.size();
                    if (!match(TokenKind::Comma)) break;
                    saw_comma = true;
                    if (check(TokenKind::RParen)) break;
                    continue;
                }
                pattern.elements.push_back(parse_binding_pattern());
                if (!match(TokenKind::Comma)) break;
                saw_comma = true;
                if (check(TokenKind::RParen)) break;
            }
            expect(TokenKind::RParen, "expected ) after tuple binding pattern");
            if (!saw_comma && !pattern.has_rest) fail(pattern.loc, "tuple binding patterns require a comma");
            return pattern;
        }

        Token name = expect(TokenKind::Identifier, "expected binding pattern");
        pattern.loc = name.loc;
        if (name.text == "_") {
            pattern.kind = PatternKind::Wildcard;
            return pattern;
        }
        std::string path = parse_path_after_first(name);
        if (match(TokenKind::LBrace)) {
            pattern.kind = PatternKind::Struct;
            pattern.case_name = std::move(path);
            while (!check(TokenKind::RBrace)) {
                if (match(TokenKind::DotDot)) {
                    if (pattern.has_rest) {
                        fail(tokens_[pos_ - 1].loc, "struct binding pattern can contain only one '..' rest");
                    }
                    pattern.has_rest = true;
                    pattern.rest_index = pattern.elements.size();
                    if (!match(TokenKind::Comma)) break;
                    if (check(TokenKind::RBrace)) break;
                    continue;
                }
                Token field = expect(TokenKind::Identifier, "expected struct pattern field name");
                pattern.field_names.push_back(field.text);
                if (match(TokenKind::Colon)) {
                    pattern.elements.push_back(parse_binding_pattern());
                } else {
                    Pattern field_pattern;
                    field_pattern.kind = PatternKind::Binding;
                    field_pattern.loc = field.loc;
                    field_pattern.payload_name = field.text;
                    pattern.elements.push_back(std::move(field_pattern));
                }
                if (!match(TokenKind::Comma)) break;
            }
            expect(TokenKind::RBrace, "expected } after struct binding pattern");
            return pattern;
        }
        if (match(TokenKind::LParen)) {
            pattern.kind = PatternKind::EnumCase;
            pattern.case_name = std::move(path);
            pattern.has_payload_pattern = true;
            Pattern payload = parse_binding_pattern();
            if (payload.kind == PatternKind::Binding) {
                pattern.has_payload_binding = true;
                pattern.payload_name = payload.payload_name;
            }
            pattern.payload_pattern = std::make_unique<Pattern>(std::move(payload));
            expect(TokenKind::RParen, "expected ) after enum payload binding pattern");
            return pattern;
        }
        pattern.kind = PatternKind::Binding;
        pattern.payload_name = path;
        return pattern;
    }

    StmtPtr parse_return() {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::Return;
        stmt->loc = peek().loc;
        if (!check(TokenKind::RBrace) && !check(TokenKind::Semicolon)) {
            stmt->expr = parse_expression();
        }
        require_semicolon("expected ; after return");
        return stmt;
    }

    StmtPtr parse_if() {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::If;
        stmt->loc = peek().loc;
        if (match(TokenKind::KwLet)) {
            stmt->has_condition_pattern = true;
            stmt->condition_pattern = std::make_unique<Pattern>(parse_pattern());
            expect(TokenKind::Equal, "expected = after if-let pattern");
            stmt->condition = parse_expression_without_struct_literals();
        } else {
            stmt->condition = parse_expression_without_struct_literals();
        }
        set_stmt_then_body(*stmt, parse_block());
        if (match(TokenKind::KwElse)) {
            if (match(TokenKind::KwIf)) {
                auto nested = parse_if();
                stmt_else_body(*stmt).push_back(std::move(nested));
            } else {
                set_stmt_else_body(*stmt, parse_block());
            }
        }
        return stmt;
    }

    StmtPtr parse_while() {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::While;
        stmt->loc = peek().loc;
        if (match(TokenKind::KwLet)) {
            stmt->kind = StmtKind::WhileLet;
            stmt->has_condition_pattern = true;
            stmt->condition_pattern = std::make_unique<Pattern>(parse_pattern());
            expect(TokenKind::Equal, "expected = after while-let pattern");
            stmt->condition = parse_expression_without_struct_literals();
        } else {
            stmt->condition = parse_expression_without_struct_literals();
        }
        set_stmt_loop_body(*stmt, parse_block());
        return stmt;
    }

    StmtPtr parse_for() {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::For;
        stmt->loc = peek().loc;
        stmt->for_pattern_filter = match(TokenKind::KwLet);
        stmt->for_pattern = std::make_unique<Pattern>(parse_for_pattern());
        expect(TokenKind::KwIn, "expected in after for pattern");
        stmt->for_iterable = parse_expression_without_struct_literals();
        set_stmt_loop_body(*stmt, parse_block());
        return stmt;
    }

    StmtPtr parse_init_while() {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::InitWhile;
        stmt->loc = peek().loc;
        do {
            Token name = expect(TokenKind::Identifier, "expected init binding name");
            Binding binding;
            binding.name = name.text;
            binding.loc = name.loc;
            binding.mutable_binding = true;
            if (match(TokenKind::Colon)) {
                binding.type = parse_type();
                binding.has_type = true;
            }
            expect(TokenKind::Equal, "expected = in init binding");
            binding.init = parse_expression();
            stmt->init_bindings.push_back(std::move(binding));
        } while (match(TokenKind::Comma));

        expect(TokenKind::KwWhile, "expected while after init bindings");
        stmt->condition = parse_expression_without_struct_literals();
        set_stmt_loop_body(*stmt, parse_block());
        if (match(TokenKind::KwNext)) {
            stmt->updates = parse_expression_list();
            if (stmt->updates.size() != stmt->init_bindings.size()) {
                fail(stmt->loc, "next value count must match init binding count");
            }
        }
        return stmt;
    }

    StmtPtr parse_continue() {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::Continue;
        stmt->loc = peek().loc;
        if (starts_expression(peek().kind)) {
            stmt->updates = parse_expression_list();
        }
        require_semicolon("expected ; after continue");
        return stmt;
    }

    StmtPtr parse_break() {
        Token break_token = tokens_[pos_ - 1];
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::Break;
        stmt->loc = break_token.loc;
        if (check(TokenKind::Identifier)) {
            set_stmt_break_label(*stmt, tokens_[pos_++].text);
        }
        if (starts_expression(peek().kind) && peek().loc.line == break_token.loc.line) {
            set_stmt_break_value(*stmt, parse_expression());
        }
        require_semicolon("expected ; after break");
        return stmt;
    }

    StmtPtr parse_match() {
        Token match_token = tokens_[pos_ - 1];
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::Match;
        stmt->loc = match_token.loc;
        stmt->match_value = parse_expression_without_struct_literals();
        expect(TokenKind::LBrace, "expected { after match value");
        while (!match(TokenKind::RBrace)) {
            if (check(TokenKind::End)) fail(peek().loc, "unterminated match");
            MatchArm arm;
            arm.pattern = parse_pattern();
            arm.loc = arm.pattern.loc;
            expect(TokenKind::FatArrow, "expected => after match pattern");
            arm.body = parse_block();
            ensure_stmt_match_arms(*stmt).push_back(std::move(arm));
            optional_separator();
        }
        return stmt;
    }

    Pattern parse_pattern(bool bare_identifier_is_binding = false) {
        Pattern pattern = parse_pattern_atom(bare_identifier_is_binding);
        if (!match(TokenKind::Pipe)) return pattern;

        Pattern or_pattern;
        or_pattern.kind = PatternKind::Or;
        or_pattern.loc = pattern.loc;
        append_or_alternative(or_pattern, std::move(pattern));
        do {
            append_or_alternative(or_pattern, parse_pattern_atom(bare_identifier_is_binding));
        } while (match(TokenKind::Pipe));
        return or_pattern;
    }

    Pattern parse_pattern_atom(bool bare_identifier_is_binding) {
        Pattern pattern;
        reject_pattern_binding_mode_start("pattern");
        if (match(TokenKind::LParen)) {
            SourceLocation loc = tokens_[pos_ - 1].loc;
            if (match(TokenKind::RParen)) {
                pattern.kind = PatternKind::Tuple;
                pattern.loc = loc;
                return pattern;
            }
            if (match(TokenKind::DotDot)) {
                pattern.kind = PatternKind::Tuple;
                pattern.loc = loc;
                pattern.has_rest = true;
                pattern.rest_index = 0;
                if (match(TokenKind::Comma) && !check(TokenKind::RParen)) {
                    do {
                        pattern.elements.push_back(parse_pattern(true));
                    } while (match(TokenKind::Comma) && !check(TokenKind::RParen));
                }
                expect(TokenKind::RParen, "expected ) after tuple pattern");
                return pattern;
            }
            Pattern first = parse_pattern(bare_identifier_is_binding);
            if (!match(TokenKind::Comma)) {
                expect(TokenKind::RParen, "expected ) after grouped pattern");
                return first;
            }

            pattern.kind = PatternKind::Tuple;
            pattern.loc = loc;
            pattern.elements.push_back(std::move(first));
            Pattern& first_element = pattern.elements.back();
            if (first_element.kind == PatternKind::EnumCase &&
                !first_element.has_payload_pattern &&
                first_element.case_name.find("::") == std::string::npos) {
                first_element.kind = PatternKind::Binding;
                first_element.payload_name = first_element.case_name;
                first_element.case_name.clear();
            }
            while (!check(TokenKind::RParen)) {
                if (match(TokenKind::DotDot)) {
                    if (pattern.has_rest) {
                        fail(tokens_[pos_ - 1].loc, "tuple pattern can contain only one '..' rest");
                    }
                    pattern.has_rest = true;
                    pattern.rest_index = pattern.elements.size();
                    if (!match(TokenKind::Comma)) break;
                    if (check(TokenKind::RParen)) break;
                    continue;
                }
                pattern.elements.push_back(parse_pattern(true));
                if (!match(TokenKind::Comma)) break;
                if (check(TokenKind::RParen)) break;
            }
            expect(TokenKind::RParen, "expected ) after tuple pattern");
            return pattern;
        }
        if (match(TokenKind::LBracket)) {
            pattern.kind = PatternKind::Array;
            pattern.loc = tokens_[pos_ - 1].loc;
            while (!check(TokenKind::RBracket)) {
                if (match(TokenKind::DotDot)) {
                    if (pattern.has_rest) {
                        fail(tokens_[pos_ - 1].loc, "array pattern can contain only one '..' rest");
                    }
                    pattern.has_rest = true;
                    pattern.rest_index = pattern.elements.size();
                    if (!match(TokenKind::Comma)) break;
                    if (check(TokenKind::RBracket)) break;
                    continue;
                }
                pattern.elements.push_back(parse_pattern(true));
                if (!match(TokenKind::Comma)) break;
                if (check(TokenKind::RBracket)) break;
            }
            expect(TokenKind::RBracket, "expected ] after array pattern");
            if (pattern.elements.empty() && !pattern.has_rest) {
                fail(pattern.loc, "array patterns must contain at least one element or '..'");
            }
            return pattern;
        }
        if (match(TokenKind::Minus)) {
            Token literal = expect(TokenKind::Integer, "expected integer literal after - in pattern");
            pattern.kind = PatternKind::IntegerLiteral;
            pattern.loc = literal.loc;
            pattern.int_value = literal.int_value;
            pattern.int_negative = true;
            pattern.literal_suffix = literal.literal_suffix;
            return finish_integer_pattern(std::move(pattern));
        }
        if (match(TokenKind::Integer)) {
            Token literal = tokens_[pos_ - 1];
            pattern.kind = PatternKind::IntegerLiteral;
            pattern.loc = literal.loc;
            pattern.int_value = literal.int_value;
            pattern.literal_suffix = literal.literal_suffix;
            return finish_integer_pattern(std::move(pattern));
        }
        if (match(TokenKind::KwTrue) || match(TokenKind::KwFalse)) {
            Token literal = tokens_[pos_ - 1];
            pattern.kind = PatternKind::BoolLiteral;
            pattern.loc = literal.loc;
            pattern.bool_value = literal.kind == TokenKind::KwTrue;
            return pattern;
        }

        Token name = expect(TokenKind::Identifier, "expected match pattern");
        pattern.loc = name.loc;
        if (name.text != "_" && match(TokenKind::At)) {
            pattern.kind = PatternKind::Alias;
            pattern.alias_name = name.text;
            pattern.alias_pattern = std::make_unique<Pattern>(parse_pattern_atom(bare_identifier_is_binding));
            return pattern;
        }
        if (name.text == "_") {
            pattern.kind = PatternKind::Wildcard;
            return pattern;
        }
        if (bare_identifier_is_binding &&
            !check(TokenKind::ColonColon) &&
            !check(TokenKind::Bang) &&
            !check(TokenKind::LParen) &&
            !check(TokenKind::LBrace) &&
            !check(TokenKind::At)) {
            pattern.kind = PatternKind::Binding;
            pattern.payload_name = name.text;
            return pattern;
        }

        pattern.kind = PatternKind::EnumCase;
        pattern.case_name = parse_path_after_first(name);
        if (match(TokenKind::Bang)) {
            Token bang = tokens_[pos_ - 1];
            parse_pattern_macro_token_tree(bang.loc);
            fail(pattern.loc, "pattern macro invocation '" + pattern.case_name + "!' requires compile-time pattern expansion");
        }
        if (match(TokenKind::At)) {
            fail(tokens_[pos_ - 1].loc, "pattern aliases must bind a plain name before @");
        }
        if (match(TokenKind::LBrace)) {
            pattern.kind = PatternKind::Struct;
            while (!check(TokenKind::RBrace)) {
                if (match(TokenKind::DotDot)) {
                    if (pattern.has_rest) {
                        fail(tokens_[pos_ - 1].loc, "struct pattern can contain only one '..' rest");
                    }
                    pattern.has_rest = true;
                    pattern.rest_index = pattern.elements.size();
                    if (!match(TokenKind::Comma)) break;
                    if (check(TokenKind::RBrace)) break;
                    continue;
                }
                Token field = expect(TokenKind::Identifier, "expected struct pattern field name");
                pattern.field_names.push_back(field.text);
                if (match(TokenKind::Colon)) {
                    pattern.elements.push_back(parse_pattern(true));
                } else {
                    Pattern field_pattern;
                    field_pattern.kind = PatternKind::Binding;
                    field_pattern.loc = field.loc;
                    field_pattern.payload_name = field.text;
                    pattern.elements.push_back(std::move(field_pattern));
                }
                if (!match(TokenKind::Comma)) break;
            }
            expect(TokenKind::RBrace, "expected } after struct pattern");
            return pattern;
        }
        if (match(TokenKind::LParen)) {
            pattern.has_payload_pattern = true;
            if (!check(TokenKind::RParen)) {
                if (match(TokenKind::DotDot)) {
                    Pattern rest;
                    rest.kind = PatternKind::Tuple;
                    rest.loc = tokens_[pos_ - 1].loc;
                    rest.has_rest = true;
                    rest.rest_index = 0;
                    if (match(TokenKind::Comma) && !check(TokenKind::RParen)) {
                        do {
                            rest.elements.push_back(parse_pattern(true));
                        } while (match(TokenKind::Comma) && !check(TokenKind::RParen));
                    }
                    pattern.payload_pattern = std::make_unique<Pattern>(std::move(rest));
                    expect(TokenKind::RParen, "expected ) after enum payload pattern");
                    return pattern;
                }
                Pattern first = parse_pattern(true);
                Pattern payload;
                if (match(TokenKind::Comma)) {
                    payload.kind = PatternKind::Tuple;
                    payload.loc = first.loc;
                    payload.elements.push_back(std::move(first));
                    while (!check(TokenKind::RParen)) {
                        if (match(TokenKind::DotDot)) {
                            if (payload.has_rest) {
                                fail(tokens_[pos_ - 1].loc, "tuple pattern can contain only one '..' rest");
                            }
                            payload.has_rest = true;
                            payload.rest_index = payload.elements.size();
                            if (!match(TokenKind::Comma)) break;
                            if (check(TokenKind::RParen)) break;
                            continue;
                        }
                        payload.elements.push_back(parse_pattern(true));
                        if (!match(TokenKind::Comma)) break;
                        if (check(TokenKind::RParen)) break;
                    }
                } else {
                    payload = std::move(first);
                }
                if (payload.kind == PatternKind::Binding) {
                    pattern.has_payload_binding = true;
                    pattern.payload_name = payload.payload_name;
                }
                pattern.payload_pattern = std::make_unique<Pattern>(std::move(payload));
            }
            expect(TokenKind::RParen, "expected ) after enum payload pattern");
        }
        return pattern;
    }

    static void append_or_alternative(Pattern& target, Pattern pattern) {
        if (pattern.kind == PatternKind::Or) {
            for (auto& alternative : pattern.alternatives) {
                target.alternatives.push_back(std::move(alternative));
            }
            return;
        }
        target.alternatives.push_back(std::move(pattern));
    }

    Pattern finish_integer_pattern(Pattern pattern) {
        if (!match(TokenKind::DotDot) && !match(TokenKind::DotDotEqual)) return pattern;
        Token op = tokens_[pos_ - 1];
        pattern.kind = PatternKind::Range;
        pattern.range_inclusive = op.kind == TokenKind::DotDotEqual;

        bool end_negative = false;
        if (match(TokenKind::Minus)) end_negative = true;
        Token end = expect(TokenKind::Integer, "expected integer literal after range operator in pattern");
        pattern.range_end_value = end.int_value;
        pattern.range_end_negative = end_negative;
        pattern.range_end_suffix = end.literal_suffix;
        return pattern;
    }

    Pattern parse_for_pattern() {
        return parse_pattern(true);
    }

    StmtPtr parse_drop() {
        Token name = expect(TokenKind::Identifier, "expected binding name after drop");
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::Drop;
        stmt->loc = name.loc;
        set_stmt_drop_name(*stmt, name.text);
        require_semicolon("expected ; after drop");
        return stmt;
    }

    std::vector<ExprPtr> parse_expression_list() {
        std::vector<ExprPtr> values;
        values.push_back(parse_expression());
        while (match(TokenKind::Comma)) {
            values.push_back(parse_expression());
        }
        return values;
    }

    static bool is_assignment_operator(TokenKind kind) {
        switch (kind) {
            case TokenKind::Equal:
            case TokenKind::PlusEqual:
            case TokenKind::MinusEqual:
            case TokenKind::StarEqual:
            case TokenKind::SlashEqual:
            case TokenKind::PercentEqual:
            case TokenKind::AmpEqual:
            case TokenKind::PipeEqual:
            case TokenKind::CaretEqual:
            case TokenKind::LessLessEqual:
            case TokenKind::GreaterGreaterEqual:
                return true;
            default:
                return false;
        }
    }

    static TokenKind compound_assignment_binary_operator(TokenKind kind, SourceLocation loc) {
        switch (kind) {
            case TokenKind::PlusEqual: return TokenKind::Plus;
            case TokenKind::MinusEqual: return TokenKind::Minus;
            case TokenKind::StarEqual: return TokenKind::Star;
            case TokenKind::SlashEqual: return TokenKind::Slash;
            case TokenKind::PercentEqual: return TokenKind::Percent;
            case TokenKind::AmpEqual: return TokenKind::Amp;
            case TokenKind::PipeEqual: return TokenKind::Pipe;
            case TokenKind::CaretEqual: return TokenKind::Caret;
            case TokenKind::LessLessEqual: return TokenKind::LessLess;
            case TokenKind::GreaterGreaterEqual: return TokenKind::GreaterGreater;
            default:
                fail(loc, "expected compound assignment operator");
        }
    }

    bool starts_expression(TokenKind kind) const {
        return kind == TokenKind::Integer ||
               kind == TokenKind::Float ||
               kind == TokenKind::String ||
               kind == TokenKind::Identifier ||
               kind == TokenKind::KwTrue ||
               kind == TokenKind::KwFalse ||
               kind == TokenKind::KwNull ||
               kind == TokenKind::KwRef ||
               kind == TokenKind::KwMatch ||
               kind == TokenKind::KwIf ||
               kind == TokenKind::LBracket ||
               kind == TokenKind::LBrace ||
               kind == TokenKind::LParen ||
               kind == TokenKind::Star ||
               kind == TokenKind::Minus ||
               kind == TokenKind::Bang ||
               kind == TokenKind::Tilde;
    }

    ExprPtr parse_expression(int min_prec = 1) {
        ExprPtr left = parse_unary();
        for (;;) {
            if (check(TokenKind::DotDot) || check(TokenKind::DotDotEqual)) {
                Token op = tokens_[pos_++];
                std::vector<ExprPtr> args;
                args.push_back(std::move(left));
                args.push_back(parse_expression(1));
                left = make_ast_call_expr(
                    op.loc,
                    op.kind == TokenKind::DotDot ? "range" : "range_inclusive",
                    nullptr,
                    std::move(args));
                continue;
            }
            reject_planned_operator(peek());
            int prec = precedence(peek().kind);
            if (prec < min_prec) break;
            Token op = tokens_[pos_++];
            ExprPtr right = parse_expression(op.kind == TokenKind::QuestionQuestion ? prec : prec + 1);
            if (op.kind == TokenKind::QuestionQuestion) {
                auto expr = std::make_unique<Expr>();
                expr->kind = ExprKind::NullCoalesce;
                expr->loc = op.loc;
                expr->op = op.kind;
                set_expr_left(*expr, std::move(left));
                set_expr_right(*expr, std::move(right));
                left = std::move(expr);
            } else {
                left = make_ast_binary_expr(op.loc, op.kind, std::move(left), std::move(right));
            }
        }
        return left;
    }

    ExprPtr parse_expression_without_struct_literals(int min_prec = 1) {
        bool previous = allow_struct_literals_;
        allow_struct_literals_ = false;
        ExprPtr expr = parse_expression(min_prec);
        allow_struct_literals_ = previous;
        return expr;
    }

    ExprPtr parse_unary() {
        if (match(TokenKind::Minus)) {
            Token minus = tokens_[pos_ - 1];
            if (check(TokenKind::Integer)) {
                Token number = tokens_[pos_++];
                return make_ast_integer_expr(minus.loc, number.int_value, true, number.literal_suffix);
            }
            if (check(TokenKind::Float)) {
                Token number = tokens_[pos_++];
                return make_ast_float_expr(minus.loc, -number.float_value, number.literal_suffix);
            }
            return make_ast_binary_expr(minus.loc, TokenKind::Minus, make_ast_integer_expr(minus.loc, 0), parse_unary());
        }
        if (match(TokenKind::Bang)) {
            Token bang = tokens_[pos_ - 1];
            return make_ast_unary_expr(bang.loc, TokenKind::Bang, parse_unary());
        }
        if (match(TokenKind::Tilde)) {
            Token tilde = tokens_[pos_ - 1];
            return make_ast_unary_expr(tilde.loc, TokenKind::Tilde, parse_unary());
        }
        if (match(TokenKind::Star)) {
            Token star = tokens_[pos_ - 1];
            return make_ast_unary_expr(star.loc, TokenKind::Star, parse_unary());
        }
        if (match(TokenKind::KwRef)) {
            Token ref = tokens_[pos_ - 1];
            bool mutable_borrow = match(TokenKind::KwMut);
            return make_ast_borrow_expr(ref.loc, parse_call(), mutable_borrow);
        }
        return parse_call();
    }

    static TokenKind matching_delimiter(TokenKind kind) {
        if (kind == TokenKind::LParen) return TokenKind::RParen;
        if (kind == TokenKind::LBrace) return TokenKind::RBrace;
        if (kind == TokenKind::LBracket) return TokenKind::RBracket;
        return TokenKind::End;
    }

    static bool is_closing_delimiter(TokenKind kind) {
        return kind == TokenKind::RParen || kind == TokenKind::RBrace || kind == TokenKind::RBracket;
    }

    std::vector<Token> parse_balanced_token_tree(SourceLocation loc,
                                                 const std::string& expected_open_message,
                                                 const std::string& context) {
        expect(TokenKind::LParen, expected_open_message);
        std::vector<Token> tokens;
        std::vector<TokenKind> closing_stack{TokenKind::RParen};
        while (!closing_stack.empty()) {
            if (check(TokenKind::End)) fail(loc, "unterminated " + context);
            Token token = peek();
            if (token.kind == closing_stack.back()) {
                ++pos_;
                closing_stack.pop_back();
                if (!closing_stack.empty()) tokens.push_back(token);
                continue;
            }
            TokenKind matching = matching_delimiter(token.kind);
            if (matching != TokenKind::End) {
                tokens.push_back(tokens_[pos_++]);
                closing_stack.push_back(matching);
                continue;
            }
            if (is_closing_delimiter(token.kind)) {
                fail(token.loc, "mismatched delimiter in " + context);
            }
            tokens.push_back(tokens_[pos_++]);
        }
        return tokens;
    }

    std::vector<Token> parse_attribute_token_tree(SourceLocation loc) {
        return parse_balanced_token_tree(loc, "expected ( after attribute name", "attribute arguments");
    }

    std::vector<Token> parse_pattern_macro_token_tree(SourceLocation loc) {
        return parse_balanced_token_tree(loc, "expected ( after pattern macro invocation name", "pattern macro invocation");
    }

    std::vector<Token> parse_macro_token_tree(SourceLocation loc) {
        return parse_balanced_token_tree(loc, "expected ( after macro invocation name", "macro invocation");
    }

    static std::string unqualified_name(const std::string& name) {
        std::size_t split = name.rfind("::");
        if (split == std::string::npos) return name;
        return name.substr(split + 2);
    }

    ExprPtr parse_matches_macro(SourceLocation loc) {
        expect(TokenKind::LParen, "expected ( after matches!");
        ExprPtr match_value = parse_expression();
        expect(TokenKind::Comma, "expected , after matches! value");

        std::vector<ExprMatchArm> arms;
        ExprMatchArm yes;
        yes.pattern = parse_pattern();
        yes.loc = yes.pattern.loc;
        yes.value = make_ast_bool_expr(yes.loc, true);
        arms.push_back(std::move(yes));

        if (match(TokenKind::Comma) && !check(TokenKind::RParen)) {
            fail(tokens_[pos_ - 1].loc, "matches! expects exactly an expression and a pattern");
        }
        expect(TokenKind::RParen, "expected ) after matches! pattern");

        Pattern wildcard;
        wildcard.kind = PatternKind::Wildcard;
        wildcard.loc = loc;
        ExprMatchArm no;
        no.pattern = std::move(wildcard);
        no.loc = loc;
        no.value = make_ast_bool_expr(loc, false);
        arms.push_back(std::move(no));
        return make_ast_match_expr(loc, std::move(match_value), std::move(arms));
    }

    bool is_legacy_bracket_type_arg_postfix() const {
        if (!check(TokenKind::LBracket)) return false;
        int depth = 0;
        for (std::size_t i = pos_; i < tokens_.size(); ++i) {
            TokenKind kind = tokens_[i].kind;
            if (kind == TokenKind::LBracket) {
                ++depth;
            } else if (kind == TokenKind::RBracket) {
                --depth;
                if (depth == 0) {
                    TokenKind next = peek(static_cast<int>(i - pos_ + 1)).kind;
                    return next == TokenKind::LParen || next == TokenKind::ColonColon;
                }
            } else if (kind == TokenKind::End || kind == TokenKind::Semicolon) {
                return false;
            }
        }
        return false;
    }

    bool is_angle_type_arg_postfix() const {
        if (!check(TokenKind::Less)) return false;
        int depth = 0;
        int parens = 0;
        int brackets = 0;
        for (std::size_t i = pos_; i < tokens_.size(); ++i) {
            TokenKind kind = tokens_[i].kind;
            if (kind == TokenKind::End || kind == TokenKind::Semicolon) return false;
            if (kind == TokenKind::LParen) {
                ++parens;
            } else if (kind == TokenKind::RParen) {
                if (parens > 0) --parens;
            } else if (kind == TokenKind::LBracket) {
                ++brackets;
            } else if (kind == TokenKind::RBracket) {
                if (brackets > 0) --brackets;
            } else if (parens == 0 && brackets == 0 && kind == TokenKind::Less) {
                ++depth;
            } else if (parens == 0 && brackets == 0 && kind == TokenKind::Greater) {
                --depth;
                if (depth == 0) {
                    TokenKind next = peek(static_cast<int>(i - pos_ + 1)).kind;
                    return next == TokenKind::LParen || next == TokenKind::LBrace ||
                           next == TokenKind::ColonColon;
                }
            }
        }
        return false;
    }

    void parse_call_type_arguments(TokenKind open, TokenKind close, const std::string& close_message, Expr& expr) {
        expect(open, "expected generic call type argument list");
        if (!expr_type_args(expr).empty()) {
            fail(tokens_[pos_ - 1].loc, "generic call type arguments were already provided");
        }
        ExprTypeArgs& type_args = ensure_expr_type_args(expr);
        if (!check(close)) {
            do {
                type_args.push_back(parse_type());
            } while (match(TokenKind::Comma));
        }
        expect(close, close_message);
    }

    ExprPtr parse_call() {
        ExprPtr expr = parse_primary();
        for (;;) {
            if (expr->kind == ExprKind::Name && is_angle_type_arg_postfix()) {
                parse_call_type_arguments(
                    TokenKind::Less,
                    TokenKind::Greater,
                    "expected > after generic call type arguments",
                    *expr);
                if (allow_struct_literals_ && check(TokenKind::LBrace)) {
                    expr = parse_struct_literal(std::move(expr));
                    continue;
                }
                if (match(TokenKind::ColonColon)) {
                    if (!expr_receiver_type_args(*expr).empty()) {
                        fail(expr->loc, "qualified expression type arguments were already provided");
                    }
                    set_expr_receiver_type_args(*expr, take_expr_type_args(*expr));
                    Token part = expect_identifier_or_contextual_name_keyword("expected name after ::");
                    expr->name += "::" + part.text;
                    continue;
                }
                if (!check(TokenKind::LParen)) {
                    fail(expr->loc, "generic type arguments in expression position must be followed by a call");
                }
                continue;
            }

            if (expr->kind == ExprKind::Name && is_legacy_bracket_type_arg_postfix()) {
                parse_call_type_arguments(
                    TokenKind::LBracket,
                    TokenKind::RBracket,
                    "expected ] after generic call type arguments",
                    *expr);
                if (allow_struct_literals_ && check(TokenKind::LBrace)) {
                    expr = parse_struct_literal(std::move(expr));
                    continue;
                }
                if (match(TokenKind::ColonColon)) {
                    if (!expr_receiver_type_args(*expr).empty()) {
                        fail(expr->loc, "qualified expression type arguments were already provided");
                    }
                    set_expr_receiver_type_args(*expr, take_expr_type_args(*expr));
                    Token part = expect_identifier_or_contextual_name_keyword("expected name after ::");
                    expr->name += "::" + part.text;
                    continue;
                }
                if (!check(TokenKind::LParen)) {
                    fail(expr->loc, "generic type arguments in expression position must be followed by a call");
                }
                continue;
            }

            if (match(TokenKind::Bang)) {
                Token bang = tokens_[pos_ - 1];
                if (expr->kind != ExprKind::Name) {
                    fail(bang.loc, "macro invocation syntax requires a named meta function before !");
                }
                if (unqualified_name(expr->name) == "matches") {
                    expr = parse_matches_macro(expr->loc);
                    continue;
                }
                expr = make_ast_macro_call_expr(
                    expr->loc,
                    expr->name,
                    parse_macro_token_tree(bang.loc)
                );
                continue;
            }

            if (match(TokenKind::LParen)) {
                SourceLocation call_loc = expr->loc;
                std::string call_name;
                ExprReceiverTypeArgs receiver_type_args;
                ExprTypeArgs type_args;
                ExprPtr operand;
                if (expr->kind == ExprKind::Name) {
                    call_name = expr->name;
                    receiver_type_args = take_expr_receiver_type_args(*expr);
                    type_args = take_expr_type_args(*expr);
                } else {
                    operand = std::move(expr);
                }
                std::vector<ExprPtr> args;
                if (!check(TokenKind::RParen)) {
                    do {
                        args.push_back(parse_expression());
                    } while (match(TokenKind::Comma));
                }
                expect(TokenKind::RParen, "expected ) after call arguments");
                auto call = make_ast_call_expr(call_loc, std::move(call_name), std::move(operand), std::move(args));
                set_expr_receiver_type_args(*call, std::move(receiver_type_args));
                set_expr_type_args(*call, std::move(type_args));
                expr = std::move(call);
                continue;
            }

            if (match(TokenKind::LBracket)) {
                Token open = tokens_[pos_ - 1];
                ExprPtr index = parse_expression();
                expect(TokenKind::RBracket, "expected ] after index expression");
                expr = make_ast_index_expr(open.loc, std::move(expr), std::move(index));
                continue;
            }

            if (match(TokenKind::KwAs)) {
                Token as = tokens_[pos_ - 1];
                expr = make_ast_cast_expr(as.loc, std::move(expr), parse_type());
                continue;
            }

            if (match(TokenKind::Question)) {
                Token question = tokens_[pos_ - 1];
                expr = make_ast_try_expr(question.loc, std::move(expr));
                continue;
            }

            if (match(TokenKind::Dot)) {
                SourceLocation dot_loc = tokens_[pos_ - 1].loc;
                if (check(TokenKind::Integer)) {
                    Token index = expect(TokenKind::Integer, "expected tuple index after .");
                    expr = make_ast_tuple_index_expr(dot_loc, std::move(expr), index.int_value);
                    continue;
                }
                Token field = expect_identifier_or_contextual_name_keyword("expected field name after .");
                std::vector<TypeRef> method_type_args;
                if (is_angle_type_arg_postfix()) {
                    Expr type_arg_holder;
                    parse_call_type_arguments(
                        TokenKind::Less,
                        TokenKind::Greater,
                        "expected > after generic method type arguments",
                        type_arg_holder);
                    method_type_args = take_expr_type_args(type_arg_holder);
                }
                if (match(TokenKind::LParen)) {
                    std::vector<ExprPtr> args;
                    if (!check(TokenKind::RParen)) {
                        do {
                            args.push_back(parse_expression());
                        } while (match(TokenKind::Comma));
                    }
                    expect(TokenKind::RParen, "expected ) after method call arguments");
                    expr = make_ast_method_call_expr(
                        dot_loc,
                        std::move(expr),
                        field.text,
                        std::move(method_type_args),
                        std::move(args));
                    continue;
                }
                if (!method_type_args.empty()) {
                    fail(dot_loc, "generic method type arguments must be followed by a call");
                }
                expr = make_ast_field_access_expr(dot_loc, std::move(expr), field.text);
                continue;
            }

            break;
        }
        return expr;
    }

    ExprPtr parse_primary() {
        Token token = tokens_[pos_++];
        switch (token.kind) {
            case TokenKind::Integer:
                return make_ast_integer_expr(token.loc, token.int_value, false, token.literal_suffix);
            case TokenKind::Float:
                return make_ast_float_expr(token.loc, token.float_value, token.literal_suffix);
            case TokenKind::String:
                return make_ast_string_expr(token.loc, token.text);
            case TokenKind::KwTrue:
                return make_ast_bool_expr(token.loc, true);
            case TokenKind::KwFalse:
                return make_ast_bool_expr(token.loc, false);
            case TokenKind::KwNull:
                return make_ast_null_expr(token.loc);
            case TokenKind::Identifier:
                if (check(TokenKind::Colon)) {
                    std::string label = token.text;
                    expect(TokenKind::Colon, "expected : after block expression label");
                    std::vector<StmtPtr> body;
                    ExprPtr value = parse_braced_value_block(
                        "expected { after block expression label",
                        "block expression",
                        "block expression must end with a value",
                        body);
                    return make_ast_block_expr(token.loc, std::move(label), std::move(body), std::move(value));
                }
                {
                    ExprPtr expr = make_ast_name_expr(token.loc, parse_path_after_first(token));
                    if (allow_struct_literals_ && check(TokenKind::LBrace)) {
                        return parse_struct_literal(std::move(expr));
                    }
                    return expr;
                }
            case TokenKind::LParen:
                if (match(TokenKind::RParen)) {
                    return make_ast_tuple_expr(token.loc);
                }
                {
                    ExprPtr expr = parse_expression();
                    if (match(TokenKind::Comma)) {
                        std::vector<ExprPtr> elements;
                        elements.push_back(std::move(expr));
                        if (!check(TokenKind::RParen)) {
                            do {
                                elements.push_back(parse_expression());
                            } while (match(TokenKind::Comma));
                        }
                        expect(TokenKind::RParen, "expected ) after tuple literal");
                        if (elements.size() == 1) fail(token.loc, "single-element tuple literals are not supported");
                        return make_ast_tuple_expr(token.loc, std::move(elements));
                    }
                    expect(TokenKind::RParen, "expected ) after expression");
                    return expr;
                }
            case TokenKind::LBracket:
                {
                    std::vector<ExprPtr> elements;
                    if (!check(TokenKind::RBracket)) {
                        do {
                            elements.push_back(parse_expression());
                        } while (match(TokenKind::Comma));
                    }
                    expect(TokenKind::RBracket, "expected ] after array/vector literal");
                    return make_ast_vector_expr(token.loc, std::move(elements));
                }
            case TokenKind::LBrace:
                return parse_block_expression(token.loc);
            case TokenKind::Question:
                fail(token.loc, "postfix ? must follow an Option/Result-style enum expression");
            case TokenKind::KwMatch:
                return parse_match_expression(token.loc);
            case TokenKind::KwIf:
                return parse_if_expression(token.loc);
            default:
                fail(token.loc, "expected expression");
        }
    }

    ExprPtr parse_struct_literal(ExprPtr name_expr) {
        expect(TokenKind::LBrace, "expected { after struct literal type");
        std::vector<std::string> field_names;
        std::vector<ExprPtr> field_values;
        if (!check(TokenKind::RBrace)) {
            do {
                Token field = expect(TokenKind::Identifier, "expected struct literal field name");
                expect(TokenKind::Colon, "expected : after struct literal field name");
                field_names.push_back(field.text);
                field_values.push_back(parse_expression());
            } while (match(TokenKind::Comma) && !check(TokenKind::RBrace));
        }
        expect(TokenKind::RBrace, "expected } after struct literal");
        return make_ast_struct_literal_expr(
            name_expr->loc,
            std::move(name_expr->name),
            take_expr_type_args(*name_expr),
            std::move(field_names),
            std::move(field_values));
    }

    ExprPtr parse_braced_value_after_open(
        const std::string& context,
        const std::string& missing_value_message,
        std::vector<StmtPtr>& body
    ) {
        while (!check(TokenKind::RBrace)) {
            if (check(TokenKind::End)) fail(peek().loc, "unterminated " + context);
            bool assignment = check(TokenKind::Identifier) && is_assignment_operator(peek(1).kind);
            if (starts_expression(peek().kind) && !assignment) {
                SourceLocation value_loc = peek().loc;
                ExprPtr value = parse_expression();
                if (match(TokenKind::Semicolon)) {
                    auto stmt = std::make_unique<Stmt>();
                    stmt->kind = StmtKind::ExprStmt;
                    stmt->loc = value_loc;
                    stmt->expr = std::move(value);
                    body.push_back(std::move(stmt));
                    continue;
                }
                match(TokenKind::Comma);
                if (check(TokenKind::RBrace)) {
                    expect(TokenKind::RBrace, "expected } after " + context);
                    return value;
                }
                fail(peek().loc, "expected ; after non-final expression statement");
            }
            body.push_back(parse_statement());
        }
        fail(peek().loc, missing_value_message);
    }

    ExprPtr parse_braced_value_block(
        const std::string& open_message,
        const std::string& context,
        const std::string& missing_value_message,
        std::vector<StmtPtr>& body
    ) {
        expect(TokenKind::LBrace, open_message);
        return parse_braced_value_after_open(context, missing_value_message, body);
    }

    ExprPtr parse_if_expression(SourceLocation loc) {
        ExprPtr condition;
        std::unique_ptr<Pattern> condition_pattern;
        if (match(TokenKind::KwLet)) {
            condition_pattern = std::make_unique<Pattern>(parse_pattern());
            expect(TokenKind::Equal, "expected = after if-let pattern");
            condition = parse_expression_without_struct_literals();
        } else {
            condition = parse_expression_without_struct_literals();
        }
        std::vector<StmtPtr> then_body;
        ExprPtr then_value = parse_braced_value_block(
            "expected { after if expression condition",
            "if expression arm",
            "if expression arm must end with a value",
            then_body);
        expect(TokenKind::KwElse, "if expression requires else");
        std::vector<StmtPtr> else_body;
        ExprPtr else_value;
        if (match(TokenKind::KwIf)) {
            else_value = parse_if_expression(tokens_[pos_ - 1].loc);
        } else {
            else_value = parse_braced_value_block(
                "expected { after else in if expression",
                "if expression arm",
                "if expression arm must end with a value",
                else_body);
        }
        return make_ast_if_expr(
            loc,
            std::move(condition),
            std::move(condition_pattern),
            std::move(then_body),
            std::move(then_value),
            std::move(else_body),
            std::move(else_value));
    }

    ExprPtr parse_block_expression(SourceLocation loc) {
        std::vector<StmtPtr> body;
        ExprPtr value = parse_braced_value_after_open(
            "block expression",
            "block expression must end with a value",
            body);
        return make_ast_block_expr(loc, {}, std::move(body), std::move(value));
    }

    ExprPtr parse_match_expression(SourceLocation loc) {
        ExprPtr match_value = parse_expression_without_struct_literals();
        expect(TokenKind::LBrace, "expected { after match value");
        std::vector<ExprMatchArm> arms;
        while (!match(TokenKind::RBrace)) {
            if (check(TokenKind::End)) fail(peek().loc, "unterminated match expression");
            ExprMatchArm arm;
            arm.pattern = parse_pattern();
            arm.loc = arm.pattern.loc;
            expect(TokenKind::FatArrow, "expected => after match pattern");
            arm.value = parse_expression();
            arms.push_back(std::move(arm));
            optional_separator();
        }
        return make_ast_match_expr(loc, std::move(match_value), std::move(arms));
    }

    static void reject_planned_operator(const Token& token) {
        switch (token.kind) {
            case TokenKind::DotDot:
            case TokenKind::DotDotEqual:
                fail(token.loc, "range operator syntax is planned; use range(start, end) for now");
            default:
                return;
        }
    }

    static int precedence(TokenKind kind) {
        switch (kind) {
            case TokenKind::QuestionQuestion:
                return 1;
            case TokenKind::PipePipe:
                return 2;
            case TokenKind::AmpAmp:
                return 3;
            case TokenKind::Pipe:
                return 4;
            case TokenKind::Caret:
                return 5;
            case TokenKind::Amp:
                return 6;
            case TokenKind::EqEq:
            case TokenKind::BangEq:
                return 7;
            case TokenKind::Less:
            case TokenKind::LessEq:
            case TokenKind::Greater:
            case TokenKind::GreaterEq:
                return 8;
            case TokenKind::LessLess:
            case TokenKind::GreaterGreater:
                return 9;
            case TokenKind::Plus:
            case TokenKind::Minus:
                return 10;
            case TokenKind::Star:
            case TokenKind::Slash:
            case TokenKind::Percent:
                return 11;
            default:
                return 0;
        }
    }
};

Program parse_tokens(std::vector<Token> tokens) {
    Parser parser(std::move(tokens));
    return parser.parse_program();
}

Program parse_tokens(std::vector<Token> tokens,
                     std::set<std::string> cfg_features,
                     std::string target_triple) {
    Parser parser(std::move(tokens), {}, std::move(cfg_features), std::move(target_triple));
    return parser.parse_program();
}

Program parse_tokens_in_module(std::vector<Token> tokens, std::vector<std::string> module_path) {
    Parser parser(std::move(tokens), std::move(module_path));
    return parser.parse_program();
}

Program parse_tokens_in_module(std::vector<Token> tokens,
                               std::vector<std::string> module_path,
                               std::set<std::string> cfg_features,
                               std::string target_triple) {
    Parser parser(std::move(tokens), std::move(module_path), std::move(cfg_features), std::move(target_triple));
    return parser.parse_program();
}

std::vector<ExprPtr> parse_macro_argument_expressions(std::vector<Token> tokens, SourceLocation loc) {
    Token end;
    end.kind = TokenKind::End;
    end.loc = loc;
    tokens.push_back(end);
    Parser parser(std::move(tokens));
    return parser.parse_expression_arguments_until_end(loc);
}

} // namespace ari
