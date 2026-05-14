#pragma once

#include "lazy_vector.hpp"
#include "token.hpp"
#include "types.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
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
    bool is_macro_invocation = false;
    std::vector<Token> macro_tokens;
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

struct ItemMacroInvocation {
    std::string name;
    std::string module_name;
    bool is_public = false;
    std::vector<Attribute> attributes;
    std::vector<Token> tokens;
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
    bool is_macro_invocation = false;
    std::vector<Token> macro_tokens;
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

struct ExprBlockPayload {
    std::string label;
    std::vector<StmtPtr> body;
    ExprPtr value;
};

struct ExprIfPayload {
    ExprPtr condition;
    std::unique_ptr<Pattern> condition_pattern;
    std::vector<StmtPtr> then_body;
    ExprPtr then_value;
    std::vector<StmtPtr> else_body;
    ExprPtr else_value;
};

struct ExprMatchPayload {
    ExprPtr value;
    std::vector<ExprMatchArm> arms;
};

struct ExprChildPayload {
    ExprPtr operand;
    ExprPtr left;
    ExprPtr right;
};

using ExprReceiverTypeArgs = std::vector<TypeRef>;
using ExprTypeArgs = std::vector<TypeRef>;
using ExprFieldNames = std::vector<std::string>;
using ExprArgs = LazyVector<ExprPtr>;

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
    bool mutable_borrow = false;
    TokenKind op = TokenKind::End;
    TypeRef cast_type;
    std::unique_ptr<ExprChildPayload> child_payload;
    std::unique_ptr<ExprIfPayload> if_payload;
    std::unique_ptr<ExprBlockPayload> block_payload;
    ExprArgs args;
    std::unique_ptr<ExprReceiverTypeArgs> receiver_type_args;
    std::unique_ptr<ExprTypeArgs> type_args;
    std::unique_ptr<ExprFieldNames> field_names;
    std::unique_ptr<ExprMatchPayload> match_payload;
    std::unique_ptr<std::vector<Token>> macro_tokens;
};

inline const ExprChildPayload& expr_child_payload(const Expr& expr) {
    static const ExprChildPayload empty;
    return expr.child_payload ? *expr.child_payload : empty;
}

inline ExprChildPayload& ensure_expr_child_payload(Expr& expr) {
    if (!expr.child_payload) expr.child_payload = std::make_unique<ExprChildPayload>();
    return *expr.child_payload;
}

inline void clear_empty_expr_child_payload(Expr& expr) {
    if (expr.child_payload &&
        !expr.child_payload->operand &&
        !expr.child_payload->left &&
        !expr.child_payload->right) {
        expr.child_payload.reset();
    }
}

inline const ExprPtr& expr_operand(const Expr& expr) {
    return expr_child_payload(expr).operand;
}

inline ExprPtr& mutable_expr_operand(Expr& expr) {
    return ensure_expr_child_payload(expr).operand;
}

inline void set_expr_operand(Expr& expr, ExprPtr operand) {
    if (!operand && !expr.child_payload) return;
    ensure_expr_child_payload(expr).operand = std::move(operand);
    clear_empty_expr_child_payload(expr);
}

inline ExprPtr take_expr_operand(Expr& expr) {
    if (!expr.child_payload) return nullptr;
    ExprPtr operand = std::move(expr.child_payload->operand);
    clear_empty_expr_child_payload(expr);
    return operand;
}

inline const ExprPtr& expr_left(const Expr& expr) {
    return expr_child_payload(expr).left;
}

inline ExprPtr& mutable_expr_left(Expr& expr) {
    return ensure_expr_child_payload(expr).left;
}

inline void set_expr_left(Expr& expr, ExprPtr left) {
    if (!left && !expr.child_payload) return;
    ensure_expr_child_payload(expr).left = std::move(left);
    clear_empty_expr_child_payload(expr);
}

inline ExprPtr take_expr_left(Expr& expr) {
    if (!expr.child_payload) return nullptr;
    ExprPtr left = std::move(expr.child_payload->left);
    clear_empty_expr_child_payload(expr);
    return left;
}

inline const ExprPtr& expr_right(const Expr& expr) {
    return expr_child_payload(expr).right;
}

inline ExprPtr& mutable_expr_right(Expr& expr) {
    return ensure_expr_child_payload(expr).right;
}

inline void set_expr_right(Expr& expr, ExprPtr right) {
    if (!right && !expr.child_payload) return;
    ensure_expr_child_payload(expr).right = std::move(right);
    clear_empty_expr_child_payload(expr);
}

inline ExprPtr take_expr_right(Expr& expr) {
    if (!expr.child_payload) return nullptr;
    ExprPtr right = std::move(expr.child_payload->right);
    clear_empty_expr_child_payload(expr);
    return right;
}

struct MatchArm {
    Pattern pattern;
    std::vector<StmtPtr> body;
    SourceLocation loc;
};

using StmtMatchArms = std::vector<MatchArm>;

struct StmtBreakPayload {
    std::string label;
    ExprPtr value;
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

struct StmtAssignPayload {
    std::string name;
    ExprPtr target;
    ExprPtr rhs;
};

struct StmtBodyPayload {
    std::vector<std::unique_ptr<Stmt>> statements;
    std::vector<std::unique_ptr<Stmt>> then_body;
    std::vector<std::unique_ptr<Stmt>> else_body;
    std::vector<std::unique_ptr<Stmt>> loop_body;
};

struct Stmt {
    StmtKind kind = StmtKind::ExprStmt;
    SourceLocation loc;
    std::unique_ptr<StmtBodyPayload> body_payload;
    Binding binding;
    std::unique_ptr<StmtAssignPayload> assign_payload;
    ExprPtr expr;
    ExprPtr condition;
    bool has_condition_pattern = false;
    std::unique_ptr<Pattern> condition_pattern;
    std::unique_ptr<Pattern> for_pattern;
    bool for_pattern_filter = false;
    ExprPtr for_iterable;
    ExprPtr match_value;
    std::vector<Binding> init_bindings;
    std::vector<ExprPtr> updates;
    std::unique_ptr<StmtMatchArms> match_arms;
    std::unique_ptr<std::string> drop_name;
    std::unique_ptr<std::string> label;
    std::unique_ptr<StmtBreakPayload> break_payload;
};

inline const ExprReceiverTypeArgs& expr_receiver_type_args(const Expr& expr) {
    static const ExprReceiverTypeArgs empty;
    return expr.receiver_type_args ? *expr.receiver_type_args : empty;
}

inline ExprReceiverTypeArgs& ensure_expr_receiver_type_args(Expr& expr) {
    if (!expr.receiver_type_args) expr.receiver_type_args = std::make_unique<ExprReceiverTypeArgs>();
    return *expr.receiver_type_args;
}

inline void set_expr_receiver_type_args(Expr& expr, ExprReceiverTypeArgs type_args) {
    if (type_args.empty()) {
        expr.receiver_type_args.reset();
        return;
    }
    ensure_expr_receiver_type_args(expr) = std::move(type_args);
}

inline ExprReceiverTypeArgs take_expr_receiver_type_args(Expr& expr) {
    if (!expr.receiver_type_args) return {};
    ExprReceiverTypeArgs type_args = std::move(*expr.receiver_type_args);
    expr.receiver_type_args.reset();
    return type_args;
}

inline const ExprTypeArgs& expr_type_args(const Expr& expr) {
    static const ExprTypeArgs empty;
    return expr.type_args ? *expr.type_args : empty;
}

inline ExprTypeArgs& ensure_expr_type_args(Expr& expr) {
    if (!expr.type_args) expr.type_args = std::make_unique<ExprTypeArgs>();
    return *expr.type_args;
}

inline void set_expr_type_args(Expr& expr, ExprTypeArgs type_args) {
    if (type_args.empty()) {
        expr.type_args.reset();
        return;
    }
    ensure_expr_type_args(expr) = std::move(type_args);
}

inline ExprTypeArgs take_expr_type_args(Expr& expr) {
    if (!expr.type_args) return {};
    ExprTypeArgs type_args = std::move(*expr.type_args);
    expr.type_args.reset();
    return type_args;
}

inline const ExprFieldNames& expr_field_names(const Expr& expr) {
    static const ExprFieldNames empty;
    return expr.field_names ? *expr.field_names : empty;
}

inline ExprFieldNames& ensure_expr_field_names(Expr& expr) {
    if (!expr.field_names) expr.field_names = std::make_unique<ExprFieldNames>();
    return *expr.field_names;
}

inline void set_expr_field_names(Expr& expr, ExprFieldNames field_names) {
    if (field_names.empty()) {
        expr.field_names.reset();
        return;
    }
    ensure_expr_field_names(expr) = std::move(field_names);
}

inline const ExprIfPayload& expr_if_payload(const Expr& expr) {
    static const ExprIfPayload empty;
    return expr.if_payload ? *expr.if_payload : empty;
}

inline ExprIfPayload& ensure_expr_if_payload(Expr& expr) {
    if (!expr.if_payload) expr.if_payload = std::make_unique<ExprIfPayload>();
    return *expr.if_payload;
}

inline const ExprPtr& expr_if_condition(const Expr& expr) {
    return expr_if_payload(expr).condition;
}

inline ExprPtr& expr_if_condition(Expr& expr) {
    return ensure_expr_if_payload(expr).condition;
}

inline bool expr_if_has_condition_pattern(const Expr& expr) {
    return static_cast<bool>(expr_if_payload(expr).condition_pattern);
}

inline const std::unique_ptr<Pattern>& expr_if_condition_pattern(const Expr& expr) {
    return expr_if_payload(expr).condition_pattern;
}

inline const std::vector<StmtPtr>& expr_if_then_body(const Expr& expr) {
    return expr_if_payload(expr).then_body;
}

inline std::vector<StmtPtr>& expr_if_then_body(Expr& expr) {
    return ensure_expr_if_payload(expr).then_body;
}

inline const ExprPtr& expr_if_then_value(const Expr& expr) {
    return expr_if_payload(expr).then_value;
}

inline ExprPtr& expr_if_then_value(Expr& expr) {
    return ensure_expr_if_payload(expr).then_value;
}

inline const std::vector<StmtPtr>& expr_if_else_body(const Expr& expr) {
    return expr_if_payload(expr).else_body;
}

inline std::vector<StmtPtr>& expr_if_else_body(Expr& expr) {
    return ensure_expr_if_payload(expr).else_body;
}

inline const ExprPtr& expr_if_else_value(const Expr& expr) {
    return expr_if_payload(expr).else_value;
}

inline ExprPtr& expr_if_else_value(Expr& expr) {
    return ensure_expr_if_payload(expr).else_value;
}

inline void set_expr_if_payload(Expr& expr,
                                ExprPtr condition,
                                std::unique_ptr<Pattern> condition_pattern,
                                std::vector<StmtPtr> then_body,
                                ExprPtr then_value,
                                std::vector<StmtPtr> else_body,
                                ExprPtr else_value) {
    ExprIfPayload& payload = ensure_expr_if_payload(expr);
    payload.condition = std::move(condition);
    payload.condition_pattern = std::move(condition_pattern);
    payload.then_body = std::move(then_body);
    payload.then_value = std::move(then_value);
    payload.else_body = std::move(else_body);
    payload.else_value = std::move(else_value);
}

inline const ExprMatchPayload& expr_match_payload(const Expr& expr) {
    static const ExprMatchPayload empty;
    return expr.match_payload ? *expr.match_payload : empty;
}

inline ExprMatchPayload& ensure_expr_match_payload(Expr& expr) {
    if (!expr.match_payload) expr.match_payload = std::make_unique<ExprMatchPayload>();
    return *expr.match_payload;
}

inline const ExprPtr& expr_match_value(const Expr& expr) {
    return expr_match_payload(expr).value;
}

inline ExprPtr& expr_match_value(Expr& expr) {
    return ensure_expr_match_payload(expr).value;
}

inline const std::vector<ExprMatchArm>& expr_match_arms(const Expr& expr) {
    return expr_match_payload(expr).arms;
}

inline std::vector<ExprMatchArm>& expr_match_arms(Expr& expr) {
    return ensure_expr_match_payload(expr).arms;
}

inline void set_expr_match_payload(Expr& expr, ExprPtr value, std::vector<ExprMatchArm> arms) {
    ExprMatchPayload& payload = ensure_expr_match_payload(expr);
    payload.value = std::move(value);
    payload.arms = std::move(arms);
}

inline const ExprBlockPayload& expr_block_payload(const Expr& expr) {
    static const ExprBlockPayload empty;
    return expr.block_payload ? *expr.block_payload : empty;
}

inline ExprBlockPayload& ensure_expr_block_payload(Expr& expr) {
    if (!expr.block_payload) expr.block_payload = std::make_unique<ExprBlockPayload>();
    return *expr.block_payload;
}

inline const std::string& expr_block_label(const Expr& expr) {
    return expr_block_payload(expr).label;
}

inline const std::vector<StmtPtr>& expr_block_body(const Expr& expr) {
    return expr_block_payload(expr).body;
}

inline std::vector<StmtPtr>& expr_block_body(Expr& expr) {
    return ensure_expr_block_payload(expr).body;
}

inline const ExprPtr& expr_block_value(const Expr& expr) {
    return expr_block_payload(expr).value;
}

inline ExprPtr& expr_block_value(Expr& expr) {
    return ensure_expr_block_payload(expr).value;
}

inline void set_expr_block_payload(Expr& expr,
                                   std::string label,
                                   std::vector<StmtPtr> body,
                                   ExprPtr value) {
    ExprBlockPayload& payload = ensure_expr_block_payload(expr);
    payload.label = std::move(label);
    payload.body = std::move(body);
    payload.value = std::move(value);
}

inline const StmtBodyPayload& stmt_body_payload(const Stmt& stmt) {
    static const StmtBodyPayload empty;
    return stmt.body_payload ? *stmt.body_payload : empty;
}

inline StmtBodyPayload& ensure_stmt_body_payload(Stmt& stmt) {
    if (!stmt.body_payload) stmt.body_payload = std::make_unique<StmtBodyPayload>();
    return *stmt.body_payload;
}

inline const std::vector<StmtPtr>& stmt_statements(const Stmt& stmt) {
    return stmt_body_payload(stmt).statements;
}

inline std::vector<StmtPtr>& stmt_statements(Stmt& stmt) {
    return ensure_stmt_body_payload(stmt).statements;
}

inline const std::vector<StmtPtr>& stmt_then_body(const Stmt& stmt) {
    return stmt_body_payload(stmt).then_body;
}

inline std::vector<StmtPtr>& stmt_then_body(Stmt& stmt) {
    return ensure_stmt_body_payload(stmt).then_body;
}

inline const std::vector<StmtPtr>& stmt_else_body(const Stmt& stmt) {
    return stmt_body_payload(stmt).else_body;
}

inline std::vector<StmtPtr>& stmt_else_body(Stmt& stmt) {
    return ensure_stmt_body_payload(stmt).else_body;
}

inline const std::vector<StmtPtr>& stmt_loop_body(const Stmt& stmt) {
    return stmt_body_payload(stmt).loop_body;
}

inline std::vector<StmtPtr>& stmt_loop_body(Stmt& stmt) {
    return ensure_stmt_body_payload(stmt).loop_body;
}

inline void set_stmt_statements(Stmt& stmt, std::vector<StmtPtr> statements) {
    ensure_stmt_body_payload(stmt).statements = std::move(statements);
}

inline void set_stmt_then_body(Stmt& stmt, std::vector<StmtPtr> body) {
    ensure_stmt_body_payload(stmt).then_body = std::move(body);
}

inline void set_stmt_else_body(Stmt& stmt, std::vector<StmtPtr> body) {
    ensure_stmt_body_payload(stmt).else_body = std::move(body);
}

inline void set_stmt_loop_body(Stmt& stmt, std::vector<StmtPtr> body) {
    ensure_stmt_body_payload(stmt).loop_body = std::move(body);
}

inline const StmtAssignPayload& stmt_assign_payload(const Stmt& stmt) {
    static const StmtAssignPayload empty;
    return stmt.assign_payload ? *stmt.assign_payload : empty;
}

inline StmtAssignPayload& ensure_stmt_assign_payload(Stmt& stmt) {
    if (!stmt.assign_payload) stmt.assign_payload = std::make_unique<StmtAssignPayload>();
    return *stmt.assign_payload;
}

inline const std::string& stmt_assign_name(const Stmt& stmt) {
    return stmt_assign_payload(stmt).name;
}

inline const ExprPtr& stmt_assign_target(const Stmt& stmt) {
    return stmt_assign_payload(stmt).target;
}

inline const ExprPtr& stmt_assign_rhs(const Stmt& stmt) {
    return stmt_assign_payload(stmt).rhs;
}

inline void set_stmt_assign_name(Stmt& stmt, std::string name) {
    ensure_stmt_assign_payload(stmt).name = std::move(name);
}

inline void set_stmt_assign_target(Stmt& stmt, ExprPtr target) {
    ensure_stmt_assign_payload(stmt).target = std::move(target);
}

inline void set_stmt_assign_rhs(Stmt& stmt, ExprPtr rhs) {
    ensure_stmt_assign_payload(stmt).rhs = std::move(rhs);
}

inline const StmtMatchArms& stmt_match_arms(const Stmt& stmt) {
    static const StmtMatchArms empty;
    return stmt.match_arms ? *stmt.match_arms : empty;
}

inline StmtMatchArms& ensure_stmt_match_arms(Stmt& stmt) {
    if (!stmt.match_arms) stmt.match_arms = std::make_unique<StmtMatchArms>();
    return *stmt.match_arms;
}

inline const std::string& stmt_drop_name(const Stmt& stmt) {
    static const std::string empty;
    return stmt.drop_name ? *stmt.drop_name : empty;
}

inline void set_stmt_drop_name(Stmt& stmt, std::string name) {
    stmt.drop_name = std::make_unique<std::string>(std::move(name));
}

inline const std::string& stmt_label(const Stmt& stmt) {
    static const std::string empty;
    return stmt.label ? *stmt.label : empty;
}

inline void set_stmt_label(Stmt& stmt, std::string label) {
    if (label.empty()) {
        stmt.label.reset();
        return;
    }
    stmt.label = std::make_unique<std::string>(std::move(label));
}

inline const StmtBreakPayload& stmt_break_payload(const Stmt& stmt) {
    static const StmtBreakPayload empty;
    return stmt.break_payload ? *stmt.break_payload : empty;
}

inline StmtBreakPayload& ensure_stmt_break_payload(Stmt& stmt) {
    if (!stmt.break_payload) stmt.break_payload = std::make_unique<StmtBreakPayload>();
    return *stmt.break_payload;
}

inline const std::string& stmt_break_label(const Stmt& stmt) {
    return stmt_break_payload(stmt).label;
}

inline const ExprPtr& stmt_break_value(const Stmt& stmt) {
    return stmt_break_payload(stmt).value;
}

inline void set_stmt_break_label(Stmt& stmt, std::string label) {
    ensure_stmt_break_payload(stmt).label = std::move(label);
}

inline void set_stmt_break_value(Stmt& stmt, ExprPtr value) {
    ensure_stmt_break_payload(stmt).value = std::move(value);
}

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
    bool has_body_summary = false;
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
    std::vector<ItemMacroInvocation> item_macros;
    std::vector<ConstDecl> constants;
    std::vector<FunctionDecl> functions;
    std::vector<StructDecl> structs;
    std::vector<EnumDecl> enums;
    std::vector<TraitDecl> traits;
    std::vector<ImplDecl> impls;
};

} // namespace ari
