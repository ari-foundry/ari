#pragma once

#include "common.hpp"
#include "types.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ari {

enum class IrPrimitiveKind {
    Unknown,
    Void,
    I8,
    I16,
    I32,
    I64,
    U8,
    U16,
    U32,
    U64,
    F32,
    F64,
    F128,
    Bool,
    String,
    Struct,
    Enum,
    Tuple,
    Array,
    Vector,
    Zone,
    Function,
    TraitObject,
    MetaType
};

struct IrType {
    TypeQualifier qualifier = TypeQualifier::Value;
    IrPrimitiveKind primitive = IrPrimitiveKind::Unknown;
    std::string name;
    std::vector<IrType> args;
    std::vector<std::string> field_names;
    std::vector<IrType> field_types;
    std::vector<bool> field_mutable;
    SourceLocation loc;
    std::uint64_t array_size = 0;
};

std::string type_name(const IrType& type);

enum class IrBinaryOp {
    LogicalOr,
    LogicalAnd,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    BitAnd,
    BitOr,
    BitXor,
    Shl,
    Shr,
    Eq,
    Ne,
    Lt,
    Le,
    Gt,
    Ge
};

enum class IrUnaryOp {
    Not,
    BitNot
};

struct IrExpr;
using IrExprPtr = std::unique_ptr<IrExpr>;
struct IrStmt;
using IrStmtPtr = std::unique_ptr<IrStmt>;

struct IrPayloadBinding {
    std::uint32_t index = 0;
    std::string name;
    IrType type;
    bool compact_enum_payload = false;
    IrType compact_enum_type;
    std::uint32_t compact_enum_payload_index = 0;
};

union IrPayloadLiteralValue {
    std::uint64_t integer;
    bool boolean;

    constexpr IrPayloadLiteralValue() : integer(0) {}
};

struct IrPayloadLiteralCondition {
    IrPayloadLiteralValue literal;
    std::uint32_t index = 0;
    bool is_bool = false;

    static IrPayloadLiteralCondition integer(std::uint32_t index, std::uint64_t value) {
        IrPayloadLiteralCondition condition;
        condition.index = index;
        condition.literal.integer = value;
        return condition;
    }

    static IrPayloadLiteralCondition boolean(std::uint32_t index, bool value) {
        IrPayloadLiteralCondition condition;
        condition.index = index;
        condition.is_bool = true;
        condition.literal.boolean = value;
        return condition;
    }

    std::uint64_t bits() const {
        return is_bool ? (literal.boolean ? 1ULL : 0ULL) : literal.integer;
    }

    bool bool_literal() const {
        return literal.boolean;
    }
};

static_assert(sizeof(IrPayloadLiteralCondition) <= 16,
              "IrPayloadLiteralCondition should keep mutually exclusive literals packed");

struct IrPayloadRangeCondition {
    std::uint32_t index = 0;
    std::uint64_t start_int = 0;
    bool start_negative = false;
    std::uint64_t end_int = 0;
    bool end_negative = false;
    bool inclusive = false;
    bool is_unsigned = false;
    IrType type;
    bool compact_enum_payload = false;
};

struct IrPayloadEnumCondition {
    std::uint32_t index = 0;
    IrType enum_type;
    std::uint32_t tag = 0;
    std::uint32_t nested_payload_index = 0;
    bool has_payload_literal = false;
    IrPayloadLiteralValue payload_literal;
    bool payload_literal_negative = false;
    bool payload_literal_is_bool = false;
    bool has_payload_range = false;
    std::uint64_t range_start_int = 0;
    bool range_start_negative = false;
    std::uint64_t range_end_int = 0;
    bool range_end_negative = false;
    bool range_inclusive = false;
    bool range_is_unsigned = false;
    IrType payload_type;
};

struct IrMatchExprArm {
    IrMatchExprArm() : literal_int(0) {}

    bool wildcard = false;
    bool has_literal = false;
    bool literal_is_bool = false;
    union {
        std::uint64_t literal_int;
        bool literal_bool;
    };
    bool literal_negative = false;
    bool has_range = false;
    std::uint64_t range_start_int = 0;
    bool range_start_negative = false;
    std::uint64_t range_end_int = 0;
    bool range_end_negative = false;
    bool range_inclusive = false;
    bool range_is_unsigned = false;
    std::vector<IrPayloadLiteralCondition> payload_literal_conditions;
    std::vector<IrPayloadRangeCondition> payload_range_conditions;
    std::vector<IrPayloadEnumCondition> payload_enum_conditions;
    std::string case_name;
    std::uint32_t enum_tag = 0;
    bool has_value_binding = false;
    std::string value_name;
    IrType value_type;
    bool has_payload_binding = false;
    std::string payload_name;
    IrType payload_type;
    std::uint32_t payload_index = 0;
    std::vector<IrPayloadBinding> payload_bindings;
    SourceLocation loc;
    std::vector<IrStmtPtr> body;
    IrExprPtr value;
};

struct IrExprBlockPayload {
    std::string label;
    std::vector<IrStmtPtr> body;
    IrExprPtr value;
};

struct IrExprIfPayload {
    IrExprPtr condition;
    std::vector<IrStmtPtr> then_body;
    IrExprPtr then_value;
    std::vector<IrStmtPtr> else_body;
    IrExprPtr else_value;
};

enum class IrExprKind {
    Integer,
    Float,
    String,
    Bool,
    Null,
    FunctionRef,
    Local,
    Borrow,
    Unary,
    Cast,
    PointerOffset,
    PointerAdd,
    PointerLoad,
    PointerStore,
    Try,
    NullCoalesce,
    EnumConstruct,
    Tuple,
    TupleIndex,
    Index,
    SliceRange,
    Vector,
    VectorPush,
    VectorPop,
    VectorReserve,
    VectorClear,
    VectorTruncate,
    VectorSet,
    VectorSwap,
    VectorRemove,
    VectorInsert,
    VectorContains,
    VectorIndexOf,
    VectorCount,
    Noop,
    FormatPrint,
    Match,
    If,
    Block,
    IndirectCall,
    TraitObjectCall,
    Binary,
    Call
};

struct IrExpr {
    IrExpr() : int_value(0) {}

    IrExprKind kind = IrExprKind::Integer;
    IrType type;
    SourceLocation loc;
    union {
        std::uint64_t int_value;
        double float_value;
        bool bool_value;
        std::uint64_t tuple_index;
    };
    bool int_negative = false;
    std::string string_value;
    std::string name;
    std::string label;
    std::string enum_name;
    std::string case_name;
    std::unique_ptr<std::vector<std::string>> format_parts;
    std::uint32_t enum_tag = 0;
    bool has_payload = false;
    bool print_newline = false;
    bool mutable_borrow = false;
    bool try_converts_residual = false;
    bool try_residual_has_payload = false;
    IrUnaryOp unary_op = IrUnaryOp::Not;
    IrBinaryOp op = IrBinaryOp::Add;
    IrType payload_type;
    IrType try_return_residual_payload_type;
    std::vector<IrType> call_param_types;
    std::uint32_t try_return_residual_tag = 0;
    std::unique_ptr<IrExpr> operand;
    std::unique_ptr<IrExpr> payload;
    std::unique_ptr<IrExpr> left;
    std::unique_ptr<IrExpr> right;
    std::unique_ptr<IrExprIfPayload> if_payload;
    std::unique_ptr<IrExprBlockPayload> block_payload;
    std::vector<IrStmtPtr> try_residual_cleanup;
    IrExprPtr match_value;
    std::vector<IrMatchExprArm> match_arms;
    std::vector<std::unique_ptr<IrExpr>> args;
};

struct IrMatchArm {
    IrMatchArm() : literal_int(0) {}

    bool wildcard = false;
    bool has_literal = false;
    bool literal_is_bool = false;
    union {
        std::uint64_t literal_int;
        bool literal_bool;
    };
    bool literal_negative = false;
    bool has_range = false;
    std::uint64_t range_start_int = 0;
    bool range_start_negative = false;
    std::uint64_t range_end_int = 0;
    bool range_end_negative = false;
    bool range_inclusive = false;
    bool range_is_unsigned = false;
    std::vector<IrPayloadLiteralCondition> payload_literal_conditions;
    std::vector<IrPayloadRangeCondition> payload_range_conditions;
    std::vector<IrPayloadEnumCondition> payload_enum_conditions;
    std::string case_name;
    std::uint32_t enum_tag = 0;
    bool has_value_binding = false;
    std::string value_name;
    IrType value_type;
    bool has_payload_binding = false;
    std::string payload_name;
    IrType payload_type;
    std::uint32_t payload_index = 0;
    std::vector<IrPayloadBinding> payload_bindings;
    SourceLocation loc;
    std::vector<IrStmtPtr> body;
};

using IrStmtMatchArms = std::vector<IrMatchArm>;

struct IrParam {
    std::string name;
    IrType type;
};

struct IrCRecordField {
    std::string name;
    IrType type;
    SourceLocation loc;
};

struct IrCRecord {
    std::string name;
    std::string c_name;
    std::vector<IrCRecordField> fields;
    SourceLocation loc;
    bool opaque = false;
};

struct IrCEnumCase {
    std::string name;
    std::string c_name;
    std::uint32_t tag = 0;
    SourceLocation loc;
};

struct IrCEnum {
    std::string name;
    std::string c_name;
    std::vector<IrCEnumCase> cases;
    SourceLocation loc;
};

enum class IrExternAbi {
    C,
    AriBuiltin,
};

struct IrExternFunction {
    std::string name;
    std::string link_name;
    IrExternAbi abi = IrExternAbi::C;
    std::vector<IrParam> params;
    IrType return_type;
    bool is_variadic = false;
    SourceLocation loc;
};

struct IrTraitObjectVTableMethod {
    std::string thunk_name;
    std::string impl_name;
    IrType concrete_receiver_type;
    IrType result_type;
    std::vector<IrType> erased_params;
    std::vector<IrType> impl_params;
};

struct IrTraitObjectVTable {
    std::string name;
    IrType object_type;
    IrType concrete_type;
    std::vector<IrTraitObjectVTableMethod> methods;
    SourceLocation loc;
};

enum class IrStmtKind {
    Block,
    VarDecl,
    Assign,
    ExprStmt,
    Return,
    If,
    While,
    WhileLet,
    ForRange,
    ForVector,
    InitWhile,
    Continue,
    Break,
    Match,
    Drop
};

struct IrBinding {
    std::string name;
    IrType type;
    IrExprPtr init;
    bool mutable_binding = true;
    SourceLocation loc;
};

struct IrBreakPayload {
    std::string label;
    IrExprPtr value;
};

struct IrAssignPayload {
    std::string name;
    IrExprPtr target;
    IrExprPtr rhs;
};

struct IrStmtBodyPayload {
    std::vector<std::unique_ptr<IrStmt>> statements;
    std::vector<std::unique_ptr<IrStmt>> then_body;
    std::vector<std::unique_ptr<IrStmt>> else_body;
    std::vector<std::unique_ptr<IrStmt>> loop_body;
};

struct IrStmt {
    IrStmtKind kind = IrStmtKind::ExprStmt;
    SourceLocation loc;
    std::unique_ptr<IrStmtBodyPayload> body_payload;
    IrBinding binding;
    std::unique_ptr<IrAssignPayload> assign_payload;
    IrExprPtr expr;
    IrExprPtr condition;
    bool while_let_continue_on_mismatch = false;
    std::string for_binding_name;
    std::string for_index_name;
    std::string for_end_name;
    IrType for_binding_type;
    IrExprPtr for_start;
    IrExprPtr for_end;
    bool for_inclusive = false;
    std::vector<IrExprPtr> for_values;
    IrExprPtr match_value;
    std::vector<IrBinding> init_bindings;
    std::vector<IrExprPtr> updates;
    std::unique_ptr<IrStmtMatchArms> match_arms;
    std::unique_ptr<std::string> drop_name;
    std::unique_ptr<std::string> label;
    std::unique_ptr<IrBreakPayload> break_payload;
};

inline const IrExprIfPayload& ir_expr_if_payload(const IrExpr& expr) {
    static const IrExprIfPayload empty;
    return expr.if_payload ? *expr.if_payload : empty;
}

inline IrExprIfPayload& ensure_ir_expr_if_payload(IrExpr& expr) {
    if (!expr.if_payload) expr.if_payload = std::make_unique<IrExprIfPayload>();
    return *expr.if_payload;
}

inline const IrExprPtr& ir_expr_if_condition(const IrExpr& expr) {
    return ir_expr_if_payload(expr).condition;
}

inline IrExprPtr& ir_expr_if_condition(IrExpr& expr) {
    return ensure_ir_expr_if_payload(expr).condition;
}

inline const std::vector<IrStmtPtr>& ir_expr_if_then_body(const IrExpr& expr) {
    return ir_expr_if_payload(expr).then_body;
}

inline std::vector<IrStmtPtr>& ir_expr_if_then_body(IrExpr& expr) {
    return ensure_ir_expr_if_payload(expr).then_body;
}

inline const IrExprPtr& ir_expr_if_then_value(const IrExpr& expr) {
    return ir_expr_if_payload(expr).then_value;
}

inline IrExprPtr& ir_expr_if_then_value(IrExpr& expr) {
    return ensure_ir_expr_if_payload(expr).then_value;
}

inline const std::vector<IrStmtPtr>& ir_expr_if_else_body(const IrExpr& expr) {
    return ir_expr_if_payload(expr).else_body;
}

inline std::vector<IrStmtPtr>& ir_expr_if_else_body(IrExpr& expr) {
    return ensure_ir_expr_if_payload(expr).else_body;
}

inline const IrExprPtr& ir_expr_if_else_value(const IrExpr& expr) {
    return ir_expr_if_payload(expr).else_value;
}

inline IrExprPtr& ir_expr_if_else_value(IrExpr& expr) {
    return ensure_ir_expr_if_payload(expr).else_value;
}

inline void set_ir_expr_if_payload(IrExpr& expr,
                                   IrExprPtr condition,
                                   std::vector<IrStmtPtr> then_body,
                                   IrExprPtr then_value,
                                   std::vector<IrStmtPtr> else_body,
                                   IrExprPtr else_value) {
    IrExprIfPayload& payload = ensure_ir_expr_if_payload(expr);
    payload.condition = std::move(condition);
    payload.then_body = std::move(then_body);
    payload.then_value = std::move(then_value);
    payload.else_body = std::move(else_body);
    payload.else_value = std::move(else_value);
}

inline const IrExprBlockPayload& ir_expr_block_payload(const IrExpr& expr) {
    static const IrExprBlockPayload empty;
    return expr.block_payload ? *expr.block_payload : empty;
}

inline IrExprBlockPayload& ensure_ir_expr_block_payload(IrExpr& expr) {
    if (!expr.block_payload) expr.block_payload = std::make_unique<IrExprBlockPayload>();
    return *expr.block_payload;
}

inline const std::string& ir_expr_block_label(const IrExpr& expr) {
    return ir_expr_block_payload(expr).label;
}

inline const std::vector<IrStmtPtr>& ir_expr_block_body(const IrExpr& expr) {
    return ir_expr_block_payload(expr).body;
}

inline std::vector<IrStmtPtr>& ir_expr_block_body(IrExpr& expr) {
    return ensure_ir_expr_block_payload(expr).body;
}

inline const IrExprPtr& ir_expr_block_value(const IrExpr& expr) {
    return ir_expr_block_payload(expr).value;
}

inline IrExprPtr& ir_expr_block_value(IrExpr& expr) {
    return ensure_ir_expr_block_payload(expr).value;
}

inline void set_ir_expr_block_label(IrExpr& expr, std::string label) {
    ensure_ir_expr_block_payload(expr).label = std::move(label);
}

inline void set_ir_expr_block_value(IrExpr& expr, IrExprPtr value) {
    ensure_ir_expr_block_payload(expr).value = std::move(value);
}

inline void set_ir_expr_block_payload(IrExpr& expr,
                                      std::string label,
                                      std::vector<IrStmtPtr> body,
                                      IrExprPtr value) {
    IrExprBlockPayload& payload = ensure_ir_expr_block_payload(expr);
    payload.label = std::move(label);
    payload.body = std::move(body);
    payload.value = std::move(value);
}

inline const IrStmtBodyPayload& ir_stmt_body_payload(const IrStmt& stmt) {
    static const IrStmtBodyPayload empty;
    return stmt.body_payload ? *stmt.body_payload : empty;
}

inline IrStmtBodyPayload& ensure_ir_stmt_body_payload(IrStmt& stmt) {
    if (!stmt.body_payload) stmt.body_payload = std::make_unique<IrStmtBodyPayload>();
    return *stmt.body_payload;
}

inline const std::vector<IrStmtPtr>& ir_stmt_statements(const IrStmt& stmt) {
    return ir_stmt_body_payload(stmt).statements;
}

inline std::vector<IrStmtPtr>& ir_stmt_statements(IrStmt& stmt) {
    return ensure_ir_stmt_body_payload(stmt).statements;
}

inline const std::vector<IrStmtPtr>& ir_stmt_then_body(const IrStmt& stmt) {
    return ir_stmt_body_payload(stmt).then_body;
}

inline std::vector<IrStmtPtr>& ir_stmt_then_body(IrStmt& stmt) {
    return ensure_ir_stmt_body_payload(stmt).then_body;
}

inline const std::vector<IrStmtPtr>& ir_stmt_else_body(const IrStmt& stmt) {
    return ir_stmt_body_payload(stmt).else_body;
}

inline std::vector<IrStmtPtr>& ir_stmt_else_body(IrStmt& stmt) {
    return ensure_ir_stmt_body_payload(stmt).else_body;
}

inline const std::vector<IrStmtPtr>& ir_stmt_loop_body(const IrStmt& stmt) {
    return ir_stmt_body_payload(stmt).loop_body;
}

inline std::vector<IrStmtPtr>& ir_stmt_loop_body(IrStmt& stmt) {
    return ensure_ir_stmt_body_payload(stmt).loop_body;
}

inline void set_ir_stmt_statements(IrStmt& stmt, std::vector<IrStmtPtr> statements) {
    ensure_ir_stmt_body_payload(stmt).statements = std::move(statements);
}

inline void set_ir_stmt_then_body(IrStmt& stmt, std::vector<IrStmtPtr> body) {
    ensure_ir_stmt_body_payload(stmt).then_body = std::move(body);
}

inline void set_ir_stmt_else_body(IrStmt& stmt, std::vector<IrStmtPtr> body) {
    ensure_ir_stmt_body_payload(stmt).else_body = std::move(body);
}

inline void set_ir_stmt_loop_body(IrStmt& stmt, std::vector<IrStmtPtr> body) {
    ensure_ir_stmt_body_payload(stmt).loop_body = std::move(body);
}

inline const IrAssignPayload& ir_stmt_assign_payload(const IrStmt& stmt) {
    static const IrAssignPayload empty;
    return stmt.assign_payload ? *stmt.assign_payload : empty;
}

inline IrAssignPayload& ensure_ir_stmt_assign_payload(IrStmt& stmt) {
    if (!stmt.assign_payload) stmt.assign_payload = std::make_unique<IrAssignPayload>();
    return *stmt.assign_payload;
}

inline const std::string& ir_stmt_assign_name(const IrStmt& stmt) {
    return ir_stmt_assign_payload(stmt).name;
}

inline const IrExprPtr& ir_stmt_assign_target(const IrStmt& stmt) {
    return ir_stmt_assign_payload(stmt).target;
}

inline const IrExprPtr& ir_stmt_assign_rhs(const IrStmt& stmt) {
    return ir_stmt_assign_payload(stmt).rhs;
}

inline void set_ir_stmt_assign_name(IrStmt& stmt, std::string name) {
    ensure_ir_stmt_assign_payload(stmt).name = std::move(name);
}

inline void set_ir_stmt_assign_target(IrStmt& stmt, IrExprPtr target) {
    ensure_ir_stmt_assign_payload(stmt).target = std::move(target);
}

inline void set_ir_stmt_assign_rhs(IrStmt& stmt, IrExprPtr rhs) {
    ensure_ir_stmt_assign_payload(stmt).rhs = std::move(rhs);
}

inline const IrStmtMatchArms& ir_stmt_match_arms(const IrStmt& stmt) {
    static const IrStmtMatchArms empty;
    return stmt.match_arms ? *stmt.match_arms : empty;
}

inline IrStmtMatchArms& ensure_ir_stmt_match_arms(IrStmt& stmt) {
    if (!stmt.match_arms) stmt.match_arms = std::make_unique<IrStmtMatchArms>();
    return *stmt.match_arms;
}

inline const std::string& ir_stmt_drop_name(const IrStmt& stmt) {
    static const std::string empty;
    return stmt.drop_name ? *stmt.drop_name : empty;
}

inline void set_ir_stmt_drop_name(IrStmt& stmt, std::string name) {
    stmt.drop_name = std::make_unique<std::string>(std::move(name));
}

inline const std::string& ir_stmt_label(const IrStmt& stmt) {
    static const std::string empty;
    return stmt.label ? *stmt.label : empty;
}

inline void set_ir_stmt_label(IrStmt& stmt, std::string label) {
    if (label.empty()) {
        stmt.label.reset();
        return;
    }
    stmt.label = std::make_unique<std::string>(std::move(label));
}

inline const IrBreakPayload& ir_stmt_break_payload(const IrStmt& stmt) {
    static const IrBreakPayload empty;
    return stmt.break_payload ? *stmt.break_payload : empty;
}

inline IrBreakPayload& ensure_ir_stmt_break_payload(IrStmt& stmt) {
    if (!stmt.break_payload) stmt.break_payload = std::make_unique<IrBreakPayload>();
    return *stmt.break_payload;
}

inline const std::string& ir_stmt_break_label(const IrStmt& stmt) {
    return ir_stmt_break_payload(stmt).label;
}

inline const IrExprPtr& ir_stmt_break_value(const IrStmt& stmt) {
    return ir_stmt_break_payload(stmt).value;
}

inline IrExprPtr& ir_stmt_break_value(IrStmt& stmt) {
    return ensure_ir_stmt_break_payload(stmt).value;
}

inline void set_ir_stmt_break_label(IrStmt& stmt, std::string label) {
    ensure_ir_stmt_break_payload(stmt).label = std::move(label);
}

inline void set_ir_stmt_break_value(IrStmt& stmt, IrExprPtr value) {
    ensure_ir_stmt_break_payload(stmt).value = std::move(value);
}

struct IrFunction {
    std::string name;
    std::string link_name;
    std::vector<IrParam> params;
    IrType return_type;
    std::vector<IrStmtPtr> body;
    SourceLocation loc;
    bool shared_export = false;
};

struct IrProgram {
    std::vector<IrExternFunction> extern_functions;
    std::vector<IrCRecord> c_records;
    std::vector<IrCEnum> c_enums;
    std::vector<IrTraitObjectVTable> trait_object_vtables;
    std::vector<IrFunction> functions;
    std::vector<std::string> warnings;
    std::string target_triple;
    bool require_main = true;
};

} // namespace ari
