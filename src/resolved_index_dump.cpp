#include "resolved_index_dump.hpp"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ari {

namespace {

class ResolvedIndexDumper {
public:
    ResolvedIndexDumper(const IrProgram& program, std::string source_name)
        : program_(program), source_name_(std::move(source_name)) {}

    std::string dump() {
        for (const IrExternFunction& function : program_.extern_functions) {
            line(1, "Extern name=" + function.name +
                        " link=" + function.link_name +
                        " return=" + type_name(function.return_type) +
                        loc(function.loc));
            for (const IrParam& param : function.params) {
                line(2, "Param name=" + param.name + " type=" + type_name(param.type));
            }
        }
        for (const IrFunction& function : program_.functions) dump_function(function);

        std::ostringstream out;
        out << "ResolvedIndex source=" << source_name_
            << " target=" << program_.target_triple
            << " externs=" << program_.extern_functions.size()
            << " functions=" << program_.functions.size()
            << " refs=" << reference_count_ << "\n";
        for (const std::string& item : lines_) out << item;
        return out.str();
    }

private:
    const IrProgram& program_;
    std::string source_name_;
    std::vector<std::string> lines_;
    std::size_t reference_count_ = 0;

    void line(int indent, const std::string& text) {
        std::string item;
        for (int i = 0; i < indent; ++i) item += "  ";
        item += text;
        item += "\n";
        lines_.push_back(std::move(item));
    }

    std::string loc(SourceLocation loc) const {
        if (span_has_source(loc.span) && span_has_valid_order(loc.span)) {
            SourceLocation span_loc = source_location_for_span(loc.span);
            if (has_source_name(span_loc)) loc = std::move(span_loc);
        } else if (loc.source_name.empty() && valid_source_id(loc.source_id)) {
            if (const SourceFile* source = find_source_file(loc.source_id)) {
                loc.source_name = source->display_name;
            }
        }
        const std::string& display_source = loc.source_name.empty() ? source_name_ : loc.source_name;
        std::string text = " @" + display_source + ":" + std::to_string(loc.line) + ":" +
                           std::to_string(loc.column);
        if (valid_source_id(loc.source_id)) text += " source_id=" + source_id_text(loc.source_id);
        if (has_byte_span(loc)) {
            Span span = span_from_location(loc);
            text += " bytes=" + std::to_string(span.start) + ".." + std::to_string(span.end);
        }
        return text;
    }

    static std::string type_list(const std::vector<IrType>& types) {
        std::string text = "[";
        for (std::size_t i = 0; i < types.size(); ++i) {
            if (i != 0) text += ", ";
            text += type_name(types[i]);
        }
        text += "]";
        return text;
    }

    void symbol(int indent, const std::string& kind, const std::string& name,
                const IrType& type, SourceLocation location) {
        ++reference_count_;
        line(indent, "Symbol kind=" + kind +
                         " name=" + (name.empty() ? "<anonymous>" : name) +
                         " type=" + type_name(type) +
                         loc(location));
    }

    void dump_function(const IrFunction& function) {
        std::string text = "Function name=" + function.name +
                           " return=" + type_name(function.return_type);
        if (!function.specialization.kind.empty()) {
            text += " specialization=" + function.specialization.kind;
            if (!function.specialization.origin.empty()) text += " origin=" + function.specialization.origin;
        }
        line(1, text + loc(function.loc));
        for (const IrParam& param : function.params) {
            line(2, "Param name=" + param.name + " type=" + type_name(param.type));
        }
        dump_statements(function.body, 2);
    }

    void dump_binding(const IrBinding& binding, int indent) {
        symbol(indent, "local-def", binding.name, binding.type, binding.loc);
        if (binding.init) dump_expr(*binding.init, indent + 1);
    }

    void dump_statements(const std::vector<IrStmtPtr>& statements, int indent) {
        for (const IrStmtPtr& statement : statements) {
            if (statement) dump_stmt(*statement, indent);
        }
    }

    void dump_stmt(const IrStmt& stmt, int indent) {
        switch (stmt.kind) {
            case IrStmtKind::Block:
                dump_statements(ir_stmt_statements(stmt), indent);
                break;
            case IrStmtKind::VarDecl:
                dump_binding(stmt.binding, indent);
                break;
            case IrStmtKind::Assign:
                if (!ir_stmt_assign_name(stmt).empty()) {
                    ++reference_count_;
                    line(indent, "Symbol kind=assign name=" + ir_stmt_assign_name(stmt) + loc(stmt.loc));
                }
                if (ir_stmt_assign_target(stmt)) dump_expr(*ir_stmt_assign_target(stmt), indent);
                if (ir_stmt_assign_rhs(stmt)) dump_expr(*ir_stmt_assign_rhs(stmt), indent);
                break;
            case IrStmtKind::ExprStmt:
            case IrStmtKind::Return:
                if (stmt.expr) dump_expr(*stmt.expr, indent);
                break;
            case IrStmtKind::If:
                if (stmt.condition) dump_expr(*stmt.condition, indent);
                dump_statements(ir_stmt_then_body(stmt), indent);
                dump_statements(ir_stmt_else_body(stmt), indent);
                break;
            case IrStmtKind::While:
            case IrStmtKind::WhileLet:
                if (stmt.condition) dump_expr(*stmt.condition, indent);
                dump_statements(ir_stmt_loop_body(stmt), indent);
                break;
            case IrStmtKind::ForRange:
            case IrStmtKind::ForVector:
                symbol(indent, "local-def", ir_stmt_for_binding_name(stmt),
                       ir_stmt_for_binding_type(stmt), stmt.loc);
                if (ir_stmt_for_start(stmt)) dump_expr(*ir_stmt_for_start(stmt), indent + 1);
                if (ir_stmt_for_end(stmt)) dump_expr(*ir_stmt_for_end(stmt), indent + 1);
                for (const IrExprPtr& value : ir_stmt_for_values(stmt)) {
                    if (value) dump_expr(*value, indent + 1);
                }
                dump_statements(ir_stmt_loop_body(stmt), indent + 1);
                break;
            case IrStmtKind::InitWhile:
                for (const IrBinding& binding : stmt.init_bindings) dump_binding(binding, indent);
                if (stmt.condition) dump_expr(*stmt.condition, indent);
                for (const IrExprPtr& update : stmt.updates) {
                    if (update) dump_expr(*update, indent);
                }
                dump_statements(ir_stmt_loop_body(stmt), indent);
                break;
            case IrStmtKind::Continue:
                break;
            case IrStmtKind::Break:
                if (ir_stmt_break_value(stmt)) dump_expr(*ir_stmt_break_value(stmt), indent);
                break;
            case IrStmtKind::Match:
                if (stmt.match_value) dump_expr(*stmt.match_value, indent);
                for (const IrMatchArm& arm : ir_stmt_match_arms(stmt)) dump_match_arm(arm, indent);
                break;
            case IrStmtKind::Drop:
                line(indent, "Drop name=" + ir_stmt_drop_name(stmt) + loc(stmt.loc));
                break;
        }
    }

    void dump_payload_bindings(const std::vector<IrPayloadBinding>& bindings, int indent, SourceLocation loc) {
        for (const IrPayloadBinding& binding : bindings) {
            symbol(indent, "pattern-bind", binding.name, binding.type, loc);
        }
    }

    void dump_match_arm(const IrMatchArm& arm, int indent) {
        if (arm.has_value_binding) symbol(indent, "pattern-bind", arm.value_name, arm.value_type, arm.loc);
        if (!arm.payload_bindings.empty()) {
            dump_payload_bindings(arm.payload_bindings, indent, arm.loc);
        } else if (arm.has_payload_binding) {
            symbol(indent, "pattern-bind", arm.payload_name, arm.payload_type, arm.loc);
        }
        dump_statements(arm.body, indent + 1);
    }

    void dump_match_arm(const IrMatchExprArm& arm, int indent) {
        if (arm.has_value_binding) symbol(indent, "pattern-bind", arm.value_name, arm.value_type, arm.loc);
        if (!arm.payload_bindings.empty()) {
            dump_payload_bindings(arm.payload_bindings, indent, arm.loc);
        } else if (arm.has_payload_binding) {
            symbol(indent, "pattern-bind", arm.payload_name, arm.payload_type, arm.loc);
        }
        dump_statements(arm.body, indent + 1);
        if (arm.value) dump_expr(*arm.value, indent + 1);
    }

    void dump_exprs(const IrExprArgs& args, int indent) {
        for (const IrExprPtr& arg : args) {
            if (arg) dump_expr(*arg, indent);
        }
    }

    void dump_expr(const IrExpr& expr, int indent) {
        switch (expr.kind) {
            case IrExprKind::FunctionRef:
                symbol(indent, "function-ref", ir_expr_name(expr), expr.type, expr.loc);
                break;
            case IrExprKind::Local:
                symbol(indent, "local-use", ir_expr_name(expr), expr.type, expr.loc);
                break;
            case IrExprKind::Call:
            case IrExprKind::IndirectCall:
            case IrExprKind::TraitObjectCall:
                ++reference_count_;
                line(indent, "Symbol kind=" + call_kind(expr.kind) +
                                 " name=" + (ir_expr_name(expr).empty() ? "<callable>" : ir_expr_name(expr)) +
                                 " result=" + type_name(expr.type) +
                                 " params=" + type_list(ir_expr_call_param_types(expr)) +
                                 loc(expr.loc));
                dump_exprs(expr.args, indent + 1);
                break;
            case IrExprKind::EnumTag:
                ++reference_count_;
                line(indent, "Symbol kind=enum-tag enum=" + ir_expr_enum_name(expr) +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_operand(expr)) dump_expr(*ir_expr_operand(expr), indent + 1);
                break;
            case IrExprKind::EnumConstruct:
                ++reference_count_;
                line(indent, "Symbol kind=enum-case enum=" + ir_expr_enum_name(expr) +
                                 " case=" + ir_expr_case_name(expr) +
                                 " type=" + type_name(expr.type) + loc(expr.loc));
                if (ir_expr_payload(expr)) dump_expr(*ir_expr_payload(expr), indent + 1);
                break;
            case IrExprKind::Match:
                if (ir_expr_match_value(expr)) dump_expr(*ir_expr_match_value(expr), indent);
                for (const IrMatchExprArm& arm : ir_expr_match_arms(expr)) dump_match_arm(arm, indent);
                break;
            case IrExprKind::If:
                if (ir_expr_if_condition(expr)) dump_expr(*ir_expr_if_condition(expr), indent);
                dump_statements(ir_expr_if_then_body(expr), indent);
                if (ir_expr_if_then_value(expr)) dump_expr(*ir_expr_if_then_value(expr), indent);
                dump_statements(ir_expr_if_else_body(expr), indent);
                if (ir_expr_if_else_value(expr)) dump_expr(*ir_expr_if_else_value(expr), indent);
                break;
            case IrExprKind::Block:
                dump_statements(ir_expr_block_body(expr), indent);
                if (ir_expr_block_value(expr)) dump_expr(*ir_expr_block_value(expr), indent);
                break;
            default:
                if (ir_expr_operand(expr)) dump_expr(*ir_expr_operand(expr), indent);
                if (ir_expr_left(expr)) dump_expr(*ir_expr_left(expr), indent);
                if (ir_expr_right(expr)) dump_expr(*ir_expr_right(expr), indent);
                if (ir_expr_payload(expr)) dump_expr(*ir_expr_payload(expr), indent);
                dump_exprs(expr.args, indent);
                break;
        }
    }

    static std::string call_kind(IrExprKind kind) {
        switch (kind) {
            case IrExprKind::IndirectCall: return "indirect-call";
            case IrExprKind::TraitObjectCall: return "trait-object-call";
            default: return "call";
        }
    }
};

} // namespace

std::string dump_resolved_index(const IrProgram& program, const std::string& source_name) {
    return ResolvedIndexDumper(program, source_name).dump();
}

} // namespace ari
