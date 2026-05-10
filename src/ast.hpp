#pragma once

#include "token.hpp"
#include "types.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ari {

struct TypeRef {
    TypeQualifier qualifier = TypeQualifier::Value;
    std::string name;
    std::vector<TypeRef> args;
    SourceLocation loc;
    std::uint64_t array_size = 0;
    bool is_dyn_object = false;
    bool nullable = false;
};

struct UseDecl {
    std::string path;
    std::string alias;
    std::string module_name;
    bool is_public = false;
    bool is_glob = false;
    SourceLocation loc;
};

struct ModuleImport {
    std::string name;
    std::string local_name;
    std::string module_name;
    bool is_public = false;
    SourceLocation loc;
};

struct ModuleDecl {
    std::string name;
    std::string module_name;
    bool is_public = false;
    SourceLocation loc;
};

struct GenericParam {
    std::string name;
    TypeRef constraint;
    bool has_constraint = false;
    SourceLocation loc;
};

struct Attribute {
    std::string name;
    std::vector<Token> args;
    bool has_args = false;
    SourceLocation loc;
};

struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

struct ConstDecl {
    std::string name;
    std::string module_name;
    bool is_public = false;
    TypeRef type;
    ExprPtr init;
    SourceLocation loc;
};

enum class PatternKind {
    Wildcard,
    Binding,
    EnumCase,
    IntegerLiteral,
    BoolLiteral,
    Range,
    Or,
    Alias,
    Tuple,
    Array,
    Struct
};

struct Pattern {
    Pattern() : int_value(0) {}

    PatternKind kind = PatternKind::EnumCase;
    std::string case_name;
    bool has_payload_pattern = false;
    bool has_payload_binding = false;
    std::string payload_name;
    std::unique_ptr<Pattern> payload_pattern;
    union {
        std::uint64_t int_value;
        bool bool_value;
    };
    bool int_negative = false;
    std::string literal_suffix;
    std::uint64_t range_end_value = 0;
    bool range_end_negative = false;
    std::string range_end_suffix;
    bool range_inclusive = false;
    std::vector<Pattern> alternatives;
    std::vector<std::string> field_names;
    std::vector<Pattern> elements;
    bool has_rest = false;
    std::size_t rest_index = 0;
    std::string alias_name;
    std::unique_ptr<Pattern> alias_pattern;
    SourceLocation loc;
};

struct Param {
    std::string name;
    TypeRef type;
    bool has_pattern = false;
    Pattern pattern;
};

struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

struct ExprMatchArm {
    Pattern pattern;
    ExprPtr value;
    SourceLocation loc;
};

enum class ExprKind {
    Integer,
    Float,
    String,
    Bool,
    Null,
    Name,
    Borrow,
    Unary,
    Cast,
    Try,
    NullCoalesce,
    Tuple,
    TupleIndex,
    Index,
    FieldAccess,
    StructLiteral,
    Vector,
    MacroCall,
    MethodCall,
    Match,
    If,
    Block,
    Binary,
    Call
};

struct Expr {
    Expr() : int_value(0) {}

    ExprKind kind = ExprKind::Integer;
    SourceLocation loc;
    union {
        std::uint64_t int_value;
        double float_value;
        bool bool_value;
        std::uint64_t tuple_index;
    };
    bool int_negative = false;
    std::string literal_suffix;
    std::string string_value;
    std::string name;
    std::string label;
    bool mutable_borrow = false;
    TokenKind op = TokenKind::End;
    TypeRef cast_type;
    std::unique_ptr<Expr> operand;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    ExprPtr condition;
    bool has_condition_pattern = false;
    Pattern condition_pattern;
    std::vector<StmtPtr> then_body;
    ExprPtr then_value;
    std::vector<StmtPtr> else_body;
    ExprPtr else_value;
    std::vector<StmtPtr> block_body;
    ExprPtr block_value;
    ExprPtr match_value;
    std::vector<std::unique_ptr<Expr>> args;
    std::vector<TypeRef> receiver_type_args;
    std::vector<TypeRef> type_args;
    std::vector<std::string> field_names;
    std::vector<ExprMatchArm> match_arms;
    std::unique_ptr<std::vector<Token>> macro_tokens;
};

struct MatchArm {
    Pattern pattern;
    std::vector<StmtPtr> body;
    SourceLocation loc;
};

enum class StmtKind {
    Block,
    VarDecl,
    Assign,
    ExprStmt,
    Return,
    If,
    While,
    WhileLet,
    For,
    InitWhile,
    Continue,
    Break,
    Match,
    Drop
};

struct Binding {
    std::string name;
    bool has_pattern = false;
    Pattern pattern;
    TypeRef type;
    bool has_type = false;
    ExprPtr init;
    bool mutable_binding = true;
    SourceLocation loc;
};

struct Stmt {
    StmtKind kind = StmtKind::ExprStmt;
    SourceLocation loc;
    std::vector<std::unique_ptr<Stmt>> statements;
    Binding binding;
    std::string assign_name;
    ExprPtr assign_target;
    ExprPtr expr;
    ExprPtr rhs;
    ExprPtr condition;
    bool has_condition_pattern = false;
    Pattern condition_pattern;
    Pattern for_pattern;
    bool for_pattern_filter = false;
    ExprPtr for_iterable;
    ExprPtr match_value;
    std::vector<std::unique_ptr<Stmt>> then_body;
    std::vector<std::unique_ptr<Stmt>> else_body;
    std::vector<std::unique_ptr<Stmt>> loop_body;
    std::vector<Binding> init_bindings;
    std::vector<ExprPtr> updates;
    std::vector<MatchArm> match_arms;
    std::string drop_name;
    std::string label;
    std::string break_label;
    ExprPtr break_value;
};

struct FunctionDecl {
    std::string name;
    std::string module_name;
    bool meta = false;
    bool is_extern = false;
    bool is_public = false;
    bool is_variadic = false;
    std::string extern_abi = "C";
    std::string extern_link_name;
    std::vector<Attribute> attributes;
    std::vector<GenericParam> generics;
    std::vector<Param> params;
    TypeRef return_type;
    bool has_return_type = false;
    bool has_body = false;
    std::vector<StmtPtr> body;
    SourceLocation loc;
    SourceLocation variadic_loc;
};

struct StructField {
    std::string name;
    TypeRef type;
    bool mutable_field = false;
    SourceLocation loc;
};

struct StructDecl {
    std::string name;
    std::string module_name;
    bool is_public = false;
    bool tuple_struct = false;
    std::vector<Attribute> attributes;
    std::vector<GenericParam> generics;
    std::vector<StructField> fields;
    SourceLocation loc;
};

struct EnumCase {
    std::string name;
    std::vector<TypeRef> payloads;
    SourceLocation loc;
};

struct EnumDecl {
    std::string name;
    std::string module_name;
    bool is_public = false;
    std::vector<Attribute> attributes;
    std::vector<GenericParam> generics;
    std::vector<EnumCase> cases;
    SourceLocation loc;
};

struct TraitDecl {
    std::string name;
    std::string module_name;
    bool is_public = false;
    std::vector<Attribute> attributes;
    std::vector<GenericParam> generics;
    std::vector<TypeRef> supertraits;
    std::vector<FunctionDecl> methods;
    SourceLocation loc;
};

struct ImplDecl {
    TypeRef trait_type;
    TypeRef for_type;
    std::string module_name;
    bool is_public = false;
    bool has_trait = false;
    std::vector<GenericParam> generics;
    std::vector<Attribute> attributes;
    std::vector<FunctionDecl> methods;
};

struct Program {
    std::vector<UseDecl> uses;
    std::vector<ModuleImport> module_imports;
    std::vector<ModuleDecl> modules;
    std::vector<ConstDecl> constants;
    std::vector<FunctionDecl> functions;
    std::vector<StructDecl> structs;
    std::vector<EnumDecl> enums;
    std::vector<TraitDecl> traits;
    std::vector<ImplDecl> impls;
};

} // namespace ari
