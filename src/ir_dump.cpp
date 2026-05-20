#include "ir_dump.hpp"

#include <sstream>
#include <utility>

namespace ari {

namespace {

class IrDumper {
public:
    IrDumper(const IrProgram& program, std::string source_name)
        : program_(program), source_name_(std::move(source_name)) {}

    std::string dump() {
        // Keep this text structural and boring: artifact diffs should explain
        // sema/lowering changes without depending on LLVM formatting.
        line(0, "IrProgram source=" + source_name_ + " target=" + program_.target_triple +
                    " require_main=" + (program_.require_main ? "true" : "false"));
        for (const IrExternFunction& function : program_.extern_functions) dump_extern(function);
        for (const IrFunction& function : program_.functions) dump_function(function);
        return out_.str();
    }

private:
    const IrProgram& program_;
    std::string source_name_;
    std::ostringstream out_;

    void line(int indent, const std::string& text) {
        for (int i = 0; i < indent; ++i) out_ << "  ";
        out_ << text << "\n";
    }

    std::string loc(SourceLocation loc) const {
        return " @" + source_name_ + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.column);
    }

    static std::string quote(const std::string& text) {
        std::string escaped = "\"";
        for (unsigned char c : text) {
            switch (c) {
                case '\\': escaped += "\\\\"; break;
                case '"': escaped += "\\\""; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default:
                    escaped.push_back(static_cast<char>(c));
                    break;
            }
        }
        escaped += "\"";
        return escaped;
    }

    static std::string unary_op_name(IrUnaryOp op) {
        switch (op) {
            case IrUnaryOp::Not: return "!";
            case IrUnaryOp::BitNot: return "~";
        }
        return "unknown";
    }

    static std::string binary_op_name(IrBinaryOp op) {
        switch (op) {
            case IrBinaryOp::LogicalOr: return "||";
            case IrBinaryOp::LogicalAnd: return "&&";
            case IrBinaryOp::Add: return "+";
            case IrBinaryOp::Sub: return "-";
            case IrBinaryOp::Mul: return "*";
            case IrBinaryOp::Div: return "/";
            case IrBinaryOp::Mod: return "%";
            case IrBinaryOp::BitAnd: return "&";
            case IrBinaryOp::BitOr: return "|";
            case IrBinaryOp::BitXor: return "^";
            case IrBinaryOp::Shl: return "<<";
            case IrBinaryOp::Shr: return ">>";
            case IrBinaryOp::Eq: return "==";
            case IrBinaryOp::Ne: return "!=";
            case IrBinaryOp::Lt: return "<";
            case IrBinaryOp::Le: return "<=";
            case IrBinaryOp::Gt: return ">";
            case IrBinaryOp::Ge: return ">=";
        }
        return "unknown";
    }

    void dump_extern(const IrExternFunction& function) {
        std::string text = "Extern name=" + function.name + " link=" + function.link_name +
                           " return=" + type_name(function.return_type);
        if (function.is_variadic) text += " variadic";
        line(1, text + loc(function.loc));
        for (const IrParam& param : function.params) dump_param(param, 2);
    }

    void dump_function(const IrFunction& function) {
        std::string text = "Function name=" + function.name + " return=" + type_name(function.return_type);
        if (!function.link_name.empty()) text += " link=" + function.link_name;
        if (function.shared_export) text += " shared_export";
        if (!function.specialization.kind.empty()) {
            text += " specialization=" + function.specialization.kind;
            if (!function.specialization.origin.empty()) text += " origin=" + function.specialization.origin;
        }
        line(1, text + loc(function.loc));
        for (const IrParam& param : function.params) dump_param(param, 2);
        if (!function.body.empty()) {
            line(2, "Body");
            dump_statements(function.body, 3);
        }
    }

    void dump_param(const IrParam& param, int indent) {
        line(indent, "Param name=" + param.name + " type=" + type_name(param.type));
    }

    void dump_statements(const std::vector<IrStmtPtr>& statements, int indent) {
        for (const IrStmtPtr& stmt : statements) {
            if (stmt) dump_stmt(*stmt, indent);
        }
    }

    void dump_exprs(const IrExprArgs& args, int indent) {
        for (const IrExprPtr& arg : args) {
            if (arg) dump_expr(*arg, indent);
        }
    }

    void dump_stmt(const IrStmt& stmt, int indent) {
        switch (stmt.kind) {
            case IrStmtKind::Block:
                line(indent, "Block" + loc(stmt.loc));
                dump_statements(ir_stmt_statements(stmt), indent + 1);
                break;
            case IrStmtKind::VarDecl:
                dump_binding(stmt.binding, indent, "VarDecl");
                break;
            case IrStmtKind::Assign:
                line(indent, "Assign name=" + ir_stmt_assign_name(stmt) + loc(stmt.loc));
                if (ir_stmt_assign_target(stmt)) dump_expr(*ir_stmt_assign_target(stmt), indent + 1);
                if (ir_stmt_assign_rhs(stmt)) dump_expr(*ir_stmt_assign_rhs(stmt), indent + 1);
                break;
            case IrStmtKind::ExprStmt:
                line(indent, "ExprStmt" + loc(stmt.loc));
                if (stmt.expr) dump_expr(*stmt.expr, indent + 1);
                break;
            case IrStmtKind::Return:
                line(indent, "Return" + loc(stmt.loc));
                if (stmt.expr) dump_expr(*stmt.expr, indent + 1);
                break;
            case IrStmtKind::If:
                line(indent, "If" + loc(stmt.loc));
                if (stmt.condition) dump_expr(*stmt.condition, indent + 1);
                line(indent + 1, "Then");
                dump_statements(ir_stmt_then_body(stmt), indent + 2);
                if (!ir_stmt_else_body(stmt).empty()) {
                    line(indent + 1, "Else");
                    dump_statements(ir_stmt_else_body(stmt), indent + 2);
                }
                break;
            case IrStmtKind::While:
            case IrStmtKind::WhileLet:
                line(indent, std::string(stmt.kind == IrStmtKind::While ? "While" : "WhileLet") + loc(stmt.loc));
                if (stmt.condition) dump_expr(*stmt.condition, indent + 1);
                dump_statements(ir_stmt_loop_body(stmt), indent + 1);
                break;
            case IrStmtKind::ForRange:
            case IrStmtKind::ForVector:
                line(indent, std::string(stmt.kind == IrStmtKind::ForRange ? "ForRange" : "ForVector") +
                                 " binding=" + ir_stmt_for_binding_name(stmt) +
                                 " type=" + type_name(ir_stmt_for_binding_type(stmt)) + loc(stmt.loc));
                if (ir_stmt_for_start(stmt)) dump_expr(*ir_stmt_for_start(stmt), indent + 1);
                if (ir_stmt_for_end(stmt)) dump_expr(*ir_stmt_for_end(stmt), indent + 1);
                dump_statements(ir_stmt_loop_body(stmt), indent + 1);
                break;
            case IrStmtKind::InitWhile:
                line(indent, "InitWhile" + loc(stmt.loc));
                for (const IrBinding& binding : stmt.init_bindings) dump_binding(binding, indent + 1, "Init");
                if (stmt.condition) dump_expr(*stmt.condition, indent + 1);
                dump_statements(ir_stmt_loop_body(stmt), indent + 1);
                break;
            case IrStmtKind::Continue:
                line(indent, "Continue" + loc(stmt.loc));
                break;
            case IrStmtKind::Break:
                line(indent, "Break" + loc(stmt.loc));
                if (ir_stmt_break_value(stmt)) dump_expr(*ir_stmt_break_value(stmt), indent + 1);
                break;
            case IrStmtKind::Match:
                line(indent, "Match" + loc(stmt.loc));
                if (stmt.match_value) dump_expr(*stmt.match_value, indent + 1);
                for (const IrMatchArm& arm : ir_stmt_match_arms(stmt)) dump_match_arm(arm, indent + 1);
                break;
            case IrStmtKind::Drop:
                line(indent, "Drop name=" + ir_stmt_drop_name(stmt) + loc(stmt.loc));
                break;
        }
    }

    void dump_binding(const IrBinding& binding, int indent, const std::string& label) {
        std::string text = label + " name=" + binding.name + " type=" + type_name(binding.type);
        text += binding.mutable_binding ? " mut" : " let";
        line(indent, text + loc(binding.loc));
        if (binding.init) dump_expr(*binding.init, indent + 1);
    }

    template <typename Arm>
    void dump_match_arm_header(const Arm& arm, int indent) {
        std::string text = "Arm";
        if (arm.wildcard) text += " wildcard";
        if (arm.has_literal) {
            text += arm.literal_is_bool
                ? std::string(" bool=") + (arm.literal_bool ? "true" : "false")
                : " int=" + std::to_string(arm.literal_int);
        }
        if (!arm.case_name.empty()) text += " case=" + arm.case_name;
        line(indent, text + loc(arm.loc));
    }

    void dump_match_arm(const IrMatchArm& arm, int indent) {
        dump_match_arm_header(arm, indent);
        dump_statements(arm.body, indent + 1);
    }

    void dump_match_arm(const IrMatchExprArm& arm, int indent) {
        dump_match_arm_header(arm, indent);
        dump_statements(arm.body, indent + 1);
        if (arm.value) dump_expr(*arm.value, indent + 1);
    }

    void dump_expr(const IrExpr& expr, int indent) {
        switch (expr.kind) {
            case IrExprKind::Integer:
                line(indent, "Integer type=" + type_name(expr.type) +
                                 " value=" + std::to_string(expr.int_value) +
                                 (expr.int_negative ? " negative" : "") + loc(expr.loc));
                break;
            case IrExprKind::Float:
                line(indent, "Float type=" + type_name(expr.type) + loc(expr.loc));
                break;
            case IrExprKind::String:
                line(indent, "String type=" + type_name(expr.type) +
                                 " value=" + quote(ir_expr_string_value(expr)) + loc(expr.loc));
                break;
            case IrExprKind::Bool:
                line(indent, std::string("Bool type=") + type_name(expr.type) +
                                 " value=" + (expr.bool_value ? "true" : "false") + loc(expr.loc));
                break;
            case IrExprKind::Null:
                line(indent, "Null type=" + type_name(expr.type) + loc(expr.loc));
                break;
            case IrExprKind::FunctionRef:
                line(indent, "FunctionRef name=" + ir_expr_name(expr) +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                break;
            case IrExprKind::Local:
                line(indent, "Local name=" + ir_expr_name(expr) +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                break;
            case IrExprKind::Borrow:
                line(indent, std::string(expr.mutable_borrow ? "BorrowMut" : "Borrow") +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_operand(expr)) dump_expr(*ir_expr_operand(expr), indent + 1);
                break;
            case IrExprKind::Unary:
                line(indent, "Unary op=" + quote(unary_op_name(expr.unary_op)) +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_operand(expr)) dump_expr(*ir_expr_operand(expr), indent + 1);
                break;
            case IrExprKind::Cast:
                line(indent, "Cast type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_operand(expr)) dump_expr(*ir_expr_operand(expr), indent + 1);
                break;
            case IrExprKind::Try:
                line(indent, "Try type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_operand(expr)) dump_expr(*ir_expr_operand(expr), indent + 1);
                break;
            case IrExprKind::NullCoalesce:
                line(indent, "NullCoalesce type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_left(expr)) dump_expr(*ir_expr_left(expr), indent + 1);
                if (ir_expr_right(expr)) dump_expr(*ir_expr_right(expr), indent + 1);
                break;
            case IrExprKind::EnumTag:
                line(indent, "EnumTag enum=" + ir_expr_enum_name(expr) +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_operand(expr)) dump_expr(*ir_expr_operand(expr), indent + 1);
                break;
            case IrExprKind::EnumConstruct:
                line(indent, "EnumConstruct enum=" + ir_expr_enum_name(expr) +
                                 " case=" + ir_expr_case_name(expr) +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_payload(expr)) dump_expr(*ir_expr_payload(expr), indent + 1);
                break;
            case IrExprKind::Tuple:
                line(indent, "Tuple type=" + type_name(expr.type) + loc(expr.loc));
                dump_exprs(expr.args, indent + 1);
                break;
            case IrExprKind::TupleIndex:
                line(indent, "TupleIndex index=" + std::to_string(expr.tuple_index) +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_operand(expr)) dump_expr(*ir_expr_operand(expr), indent + 1);
                break;
            case IrExprKind::Index:
            case IrExprKind::SliceRange:
                line(indent, std::string(expr.kind == IrExprKind::Index ? "Index" : "SliceRange") +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_left(expr)) dump_expr(*ir_expr_left(expr), indent + 1);
                if (ir_expr_right(expr)) dump_expr(*ir_expr_right(expr), indent + 1);
                break;
            case IrExprKind::Vector:
                line(indent, "Vector type=" + type_name(expr.type) + loc(expr.loc));
                dump_exprs(expr.args, indent + 1);
                break;
            case IrExprKind::Noop:
                line(indent, "Noop type=" + type_name(expr.type) + loc(expr.loc));
                break;
            case IrExprKind::FormatPrint:
                line(indent, "FormatPrint type=" + type_name(expr.type) +
                                 (ir_expr_format_print_newline(expr) ? " newline" : "") + loc(expr.loc));
                dump_exprs(expr.args, indent + 1);
                break;
            case IrExprKind::Match:
                line(indent, "MatchExpr type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_match_value(expr)) dump_expr(*ir_expr_match_value(expr), indent + 1);
                for (const IrMatchExprArm& arm : ir_expr_match_arms(expr)) dump_match_arm(arm, indent + 1);
                break;
            case IrExprKind::If:
                line(indent, "IfExpr type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_if_condition(expr)) dump_expr(*ir_expr_if_condition(expr), indent + 1);
                dump_statements(ir_expr_if_then_body(expr), indent + 1);
                if (ir_expr_if_then_value(expr)) dump_expr(*ir_expr_if_then_value(expr), indent + 1);
                dump_statements(ir_expr_if_else_body(expr), indent + 1);
                if (ir_expr_if_else_value(expr)) dump_expr(*ir_expr_if_else_value(expr), indent + 1);
                break;
            case IrExprKind::Block:
                line(indent, "BlockExpr type=" + type_name(expr.type) + loc(expr.loc));
                dump_statements(ir_expr_block_body(expr), indent + 1);
                if (ir_expr_block_value(expr)) dump_expr(*ir_expr_block_value(expr), indent + 1);
                break;
            case IrExprKind::Binary:
                line(indent, "Binary op=" + quote(binary_op_name(expr.op)) +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_left(expr)) dump_expr(*ir_expr_left(expr), indent + 1);
                if (ir_expr_right(expr)) dump_expr(*ir_expr_right(expr), indent + 1);
                break;
            case IrExprKind::Call:
            case IrExprKind::IndirectCall:
            case IrExprKind::TraitObjectCall:
                line(indent, call_like_name(expr.kind) + " name=" + ir_expr_name(expr) +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                dump_exprs(expr.args, indent + 1);
                break;
            default:
                line(indent, expr_kind_name(expr.kind) + " type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_operand(expr)) dump_expr(*ir_expr_operand(expr), indent + 1);
                if (ir_expr_left(expr)) dump_expr(*ir_expr_left(expr), indent + 1);
                if (ir_expr_right(expr)) dump_expr(*ir_expr_right(expr), indent + 1);
                dump_exprs(expr.args, indent + 1);
                break;
        }
    }

    static std::string call_like_name(IrExprKind kind) {
        switch (kind) {
            case IrExprKind::Call: return "Call";
            case IrExprKind::IndirectCall: return "IndirectCall";
            case IrExprKind::TraitObjectCall: return "TraitObjectCall";
            default: return "Call";
        }
    }

    static std::string expr_kind_name(IrExprKind kind) {
        switch (kind) {
            case IrExprKind::PointerOffset: return "PointerOffset";
            case IrExprKind::PointerAdd: return "PointerAdd";
            case IrExprKind::PointerLoad: return "PointerLoad";
            case IrExprKind::PointerStore: return "PointerStore";
            case IrExprKind::VectorPush: return "VectorPush";
            case IrExprKind::VectorPop: return "VectorPop";
            case IrExprKind::VectorReserve: return "VectorReserve";
            case IrExprKind::VectorClear: return "VectorClear";
            case IrExprKind::VectorTruncate: return "VectorTruncate";
            case IrExprKind::VectorSet: return "VectorSet";
            case IrExprKind::VectorSwap: return "VectorSwap";
            case IrExprKind::VectorRemove: return "VectorRemove";
            case IrExprKind::VectorInsert: return "VectorInsert";
            case IrExprKind::VectorContains: return "VectorContains";
            case IrExprKind::VectorIndexOf: return "VectorIndexOf";
            case IrExprKind::VectorCount: return "VectorCount";
            case IrExprKind::TraitObjectDrop: return "TraitObjectDrop";
            default: return "Expr";
        }
    }
};

} // namespace

std::string dump_ir_program(const IrProgram& program, const std::string& source_name) {
    return IrDumper(program, source_name).dump();
}

} // namespace ari
