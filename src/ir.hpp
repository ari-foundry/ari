#pragma once

#include "common.hpp"
#include "types.hpp"

#include <cstdint>
#include <memory>
#include <string>
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
    std::uint64_t payload_literal_int = 0;
    bool payload_literal_negative = false;
    bool payload_literal_is_bool = false;
    bool payload_literal_bool = false;
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
    bool wildcard = false;
    bool has_literal = false;
    bool literal_is_bool = false;
    std::uint64_t literal_int = 0;
    bool literal_negative = false;
    bool literal_bool = false;
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
    std::vector<std::string> format_parts;
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
    IrExprPtr condition;
    std::vector<IrStmtPtr> then_body;
    IrExprPtr then_value;
    std::vector<IrStmtPtr> else_body;
    IrExprPtr else_value;
    std::vector<IrStmtPtr> block_body;
    IrExprPtr block_value;
    std::vector<IrStmtPtr> try_residual_cleanup;
    IrExprPtr match_value;
    std::vector<IrMatchExprArm> match_arms;
    std::vector<std::unique_ptr<IrExpr>> args;
};

struct IrMatchArm {
    bool wildcard = false;
    bool has_literal = false;
    bool literal_is_bool = false;
    std::uint64_t literal_int = 0;
    bool literal_negative = false;
    bool literal_bool = false;
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

struct IrStmt {
    IrStmtKind kind = IrStmtKind::ExprStmt;
    SourceLocation loc;
    std::vector<std::unique_ptr<IrStmt>> statements;
    IrBinding binding;
    std::string assign_name;
    IrExprPtr expr;
    IrExprPtr rhs;
    IrExprPtr assign_target;
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
    std::vector<std::unique_ptr<IrStmt>> then_body;
    std::vector<std::unique_ptr<IrStmt>> else_body;
    std::vector<std::unique_ptr<IrStmt>> loop_body;
    std::vector<IrBinding> init_bindings;
    std::vector<IrExprPtr> updates;
    std::vector<IrMatchArm> match_arms;
    std::string drop_name;
    std::string label;
    std::string break_label;
    IrExprPtr break_value;
};

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
