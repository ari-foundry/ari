#include "syntax_dump.hpp"

#include <sstream>
#include <utility>

namespace ari {

namespace {

class SyntaxDumper {
public:
    SyntaxDumper(const Program& program, std::string source_name)
        : program_(program), source_name_(std::move(source_name)) {}

    std::string dump() {
        line(0, "Program source=" + source_name_);
        for (const UseDecl& use : program_.uses) dump_use(use);
        for (const ModuleImport& import : program_.module_imports) dump_module_import(import);
        for (const ModuleDecl& module : program_.modules) dump_module(module);
        for (const ConstDecl& constant : program_.constants) dump_const(constant);
        for (const TypeAliasDecl& alias : program_.type_aliases) dump_type_alias(alias);
        for (const StructDecl& structure : program_.structs) dump_struct(structure);
        for (const EnumDecl& enumeration : program_.enums) dump_enum(enumeration);
        for (const TraitDecl& trait : program_.traits) dump_trait(trait);
        for (const ImplDecl& impl : program_.impls) dump_impl(impl);
        for (const FunctionDecl& function : program_.functions) dump_function(function, 1);
        for (const ItemMacroInvocation& macro : program_.item_macros) dump_item_macro(macro);
        return out_.str();
    }

private:
    const Program& program_;
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
                default: escaped.push_back(static_cast<char>(c)); break;
            }
        }
        escaped += "\"";
        return escaped;
    }

    static std::string qualifier_name(TypeQualifier qualifier) {
        switch (qualifier) {
            case TypeQualifier::Value: return "";
            case TypeQualifier::Own: return "own ";
            case TypeQualifier::Ref: return "ref ";
            case TypeQualifier::MutRef: return "ref mut ";
            case TypeQualifier::Ptr: return "ptr ";
        }
        return "";
    }

    static std::string binding_mode_name(BindingMode mode) {
        switch (mode) {
            case BindingMode::Value: return "value";
            case BindingMode::Ref: return "ref";
            case BindingMode::RefMut: return "ref-mut";
        }
        return "value";
    }

    static std::string op_name(TokenKind kind) {
        switch (kind) {
            case TokenKind::Plus: return "+";
            case TokenKind::Minus: return "-";
            case TokenKind::Star: return "*";
            case TokenKind::Slash: return "/";
            case TokenKind::Percent: return "%";
            case TokenKind::Amp: return "&";
            case TokenKind::AmpAmp: return "&&";
            case TokenKind::Pipe: return "|";
            case TokenKind::PipePipe: return "||";
            case TokenKind::Caret: return "^";
            case TokenKind::Bang: return "!";
            case TokenKind::Tilde: return "~";
            case TokenKind::EqEq: return "==";
            case TokenKind::BangEq: return "!=";
            case TokenKind::Less: return "<";
            case TokenKind::LessEq: return "<=";
            case TokenKind::LessLess: return "<<";
            case TokenKind::Greater: return ">";
            case TokenKind::GreaterEq: return ">=";
            case TokenKind::GreaterGreater: return ">>";
            case TokenKind::Question: return "?";
            case TokenKind::QuestionQuestion: return "??";
            default: return "unknown";
        }
    }

    static std::string type_ref(const TypeRef& type) {
        std::string text = qualifier_name(type.qualifier);
        if (type.is_dyn_object) text += "dyn ";
        text += type.name.empty() ? "<infer>" : type.name;
        if (!type.args.empty()) {
            text += "[";
            for (std::size_t i = 0; i < type.args.size(); ++i) {
                if (i != 0) text += ", ";
                text += type_ref(type.args[i]);
            }
            text += "]";
        }
        if (type.has_associated_projection) text += "::" + type.associated_projection;
        if (type.array_size != 0) text += "[" + std::to_string(type.array_size) + "]";
        if (type.nullable) text += "?";
        if (type.is_macro_invocation) text += "!";
        return text;
    }

    static std::string generic_params(const std::vector<GenericParam>& generics) {
        if (generics.empty()) return "";
        std::string text = "[";
        for (std::size_t i = 0; i < generics.size(); ++i) {
            if (i != 0) text += ", ";
            text += generics[i].name;
            if (generics[i].has_constraint) text += ": " + type_ref(generics[i].constraint);
        }
        text += "]";
        return text;
    }

    void dump_use(const UseDecl& use) {
        std::string text = "Use path=" + use.path;
        if (!use.alias.empty()) text += " alias=" + use.alias;
        if (use.is_glob) text += " glob";
        if (use.is_public) text += " pub";
        line(1, text + loc(use.loc));
    }

    void dump_module_import(const ModuleImport& import) {
        std::string text = "ModuleImport name=" + import.name;
        if (!import.local_name.empty()) text += " local=" + import.local_name;
        if (import.is_public) text += " pub";
        line(1, text + loc(import.loc));
    }

    void dump_module(const ModuleDecl& module) {
        std::string text = "Module name=" + module.name;
        if (module.is_public) text += " pub";
        line(1, text + loc(module.loc));
    }

    void dump_const(const ConstDecl& constant) {
        line(1, "Const name=" + constant.name + " type=" + type_ref(constant.type) + loc(constant.loc));
        if (constant.init) dump_expr(*constant.init, 2);
    }

    void dump_type_alias(const TypeAliasDecl& alias) {
        line(1, "TypeAlias name=" + alias.name + generic_params(alias.generics) +
                    " target=" + type_ref(alias.target) + loc(alias.loc));
    }

    void dump_struct(const StructDecl& structure) {
        std::string text = "Struct name=" + structure.name + generic_params(structure.generics);
        if (structure.tuple_struct) text += " tuple";
        if (structure.is_public) text += " pub";
        line(1, text + loc(structure.loc));
        for (const StructField& field : structure.fields) {
            std::string field_text = "Field name=" + field.name + " type=" + type_ref(field.type);
            if (field.mutable_field) field_text += " mut";
            line(2, field_text + loc(field.loc));
        }
    }

    void dump_enum(const EnumDecl& enumeration) {
        std::string text = "Enum name=" + enumeration.name + generic_params(enumeration.generics);
        if (enumeration.is_public) text += " pub";
        line(1, text + loc(enumeration.loc));
        for (const EnumCase& enum_case : enumeration.cases) {
            std::string case_text = "Case name=" + enum_case.name;
            if (!enum_case.payloads.empty()) {
                case_text += " payloads=";
                for (std::size_t i = 0; i < enum_case.payloads.size(); ++i) {
                    if (i != 0) case_text += ", ";
                    case_text += type_ref(enum_case.payloads[i]);
                }
            }
            line(2, case_text + loc(enum_case.loc));
        }
    }

    void dump_trait(const TraitDecl& trait) {
        std::string text = "Trait name=" + trait.name + generic_params(trait.generics);
        if (trait.is_public) text += " pub";
        line(1, text + loc(trait.loc));
        for (const auto& associated : trait.associated_types) {
            line(2, "AssociatedType name=" + associated.name + loc(associated.loc));
        }
        for (const FunctionDecl& method : trait.methods) dump_function(method, 2);
    }

    void dump_impl(const ImplDecl& impl) {
        std::string text = "Impl";
        if (impl.has_trait) text += " trait=" + type_ref(impl.trait_type);
        text += " for=" + type_ref(impl.for_type);
        line(1, text + loc(impl.loc));
        for (const auto& witness : impl.associated_type_witnesses) {
            line(2, "AssociatedTypeWitness name=" + witness.name +
                        " type=" + type_ref(witness.type) + loc(witness.loc));
        }
        for (const FunctionDecl& method : impl.methods) dump_function(method, 2);
    }

    void dump_function(const FunctionDecl& function, int indent) {
        std::string text = "Function name=" + function.name + generic_params(function.generics);
        if (function.is_public) text += " pub";
        if (function.meta) text += " meta";
        if (function.is_extern) text += " extern=\"" + function.extern_abi + "\"";
        if (!function.extern_link_name.empty()) text += " link=" + function.extern_link_name;
        text += " return=" + (function.has_return_type ? type_ref(function.return_type) : "<none>");
        line(indent, text + loc(function.loc));
        for (const Param& param : function.params) {
            std::string param_text = "Param name=" + param.name + " type=" + type_ref(param.type);
            if (param.has_pattern) param_text += " pattern";
            if (param.binding_mode != BindingMode::Value) {
                param_text += " binding=" + binding_mode_name(param.binding_mode);
            }
            line(indent + 1, param_text);
        }
        if (function.has_body) {
            line(indent + 1, "Body");
            dump_statements(function.body, indent + 2);
        }
    }

    void dump_item_macro(const ItemMacroInvocation& macro) {
        line(1, "ItemMacro name=" + macro.name + loc(macro.loc));
    }

    void dump_statements(const std::vector<StmtPtr>& statements, int indent) {
        for (const StmtPtr& stmt : statements) {
            if (stmt) dump_stmt(*stmt, indent);
        }
    }

    void dump_binding(const Binding& binding, int indent, const std::string& label) {
        std::string text = label + " name=" + binding.name;
        text += binding.mutable_binding ? " mut" : " let";
        if (binding.has_type) text += " type=" + type_ref(binding.type);
        if (binding.has_pattern) text += " pattern";
        if (binding.binding_mode != BindingMode::Value) {
            text += " binding=" + binding_mode_name(binding.binding_mode);
        }
        line(indent, text + loc(binding.loc));
        if (binding.init) dump_expr(*binding.init, indent + 1);
    }

    void dump_stmt(const Stmt& stmt, int indent) {
        switch (stmt.kind) {
            case StmtKind::Block:
                line(indent, "Block" + loc(stmt.loc));
                dump_statements(stmt_statements(stmt), indent + 1);
                break;
            case StmtKind::VarDecl:
                dump_binding(stmt.binding, indent, "VarDecl");
                break;
            case StmtKind::Assign:
                line(indent, "Assign name=" + stmt_assign_name(stmt) + loc(stmt.loc));
                if (stmt_assign_target(stmt)) dump_expr(*stmt_assign_target(stmt), indent + 1);
                if (stmt_assign_rhs(stmt)) dump_expr(*stmt_assign_rhs(stmt), indent + 1);
                break;
            case StmtKind::ExprStmt:
                line(indent, "ExprStmt" + loc(stmt.loc));
                if (stmt.expr) dump_expr(*stmt.expr, indent + 1);
                break;
            case StmtKind::Return:
                line(indent, "Return" + loc(stmt.loc));
                if (stmt.expr) dump_expr(*stmt.expr, indent + 1);
                break;
            case StmtKind::If:
                line(indent, "If" + loc(stmt.loc));
                if (stmt.condition) dump_expr(*stmt.condition, indent + 1);
                line(indent + 1, "Then");
                dump_statements(stmt_then_body(stmt), indent + 2);
                if (!stmt_else_body(stmt).empty()) {
                    line(indent + 1, "Else");
                    dump_statements(stmt_else_body(stmt), indent + 2);
                }
                break;
            case StmtKind::While:
            case StmtKind::WhileLet:
                line(indent, std::string(stmt.kind == StmtKind::While ? "While" : "WhileLet") + loc(stmt.loc));
                if (stmt.condition) dump_expr(*stmt.condition, indent + 1);
                dump_statements(stmt_loop_body(stmt), indent + 1);
                break;
            case StmtKind::For:
                line(indent, "For" + loc(stmt.loc));
                if (stmt.for_iterable) dump_expr(*stmt.for_iterable, indent + 1);
                dump_statements(stmt_loop_body(stmt), indent + 1);
                break;
            case StmtKind::InitWhile:
                line(indent, "InitWhile" + loc(stmt.loc));
                for (const Binding& binding : stmt.init_bindings) dump_binding(binding, indent + 1, "Init");
                if (stmt.condition) dump_expr(*stmt.condition, indent + 1);
                dump_statements(stmt_loop_body(stmt), indent + 1);
                break;
            case StmtKind::Continue:
                line(indent, "Continue" + loc(stmt.loc));
                break;
            case StmtKind::Break:
                line(indent, "Break" + loc(stmt.loc));
                if (stmt_break_value(stmt)) dump_expr(*stmt_break_value(stmt), indent + 1);
                break;
            case StmtKind::Match:
                line(indent, "Match" + loc(stmt.loc));
                if (stmt.match_value) dump_expr(*stmt.match_value, indent + 1);
                for (const MatchArm& arm : stmt_match_arms(stmt)) {
                    line(indent + 1, "Arm" + loc(arm.loc));
                    dump_pattern(arm.pattern, indent + 2);
                    dump_statements(arm.body, indent + 2);
                }
                break;
            case StmtKind::Drop:
                line(indent, "Drop name=" + stmt_drop_name(stmt) + loc(stmt.loc));
                break;
            case StmtKind::Forget:
                line(indent, "Forget name=" + stmt_drop_name(stmt) + loc(stmt.loc));
                break;
        }
    }

    void dump_expr(const Expr& expr, int indent) {
        switch (expr.kind) {
            case ExprKind::Integer:
                line(indent, "Integer value=" + std::to_string(expr.int_value) +
                                 (expr.int_negative ? " negative" : "") +
                                 (expr.literal_suffix.empty() ? "" : " suffix=" + expr.literal_suffix) +
                                 loc(expr.loc));
                break;
            case ExprKind::Float:
                line(indent, "Float suffix=" + expr.literal_suffix + loc(expr.loc));
                break;
            case ExprKind::String:
                line(indent, "String value=" + quote(expr.string_value) + loc(expr.loc));
                break;
            case ExprKind::Bool:
                line(indent, std::string("Bool value=") + (expr.bool_value ? "true" : "false") + loc(expr.loc));
                break;
            case ExprKind::Null:
                line(indent, "Null" + loc(expr.loc));
                break;
            case ExprKind::Name:
                line(indent, "Name value=" + expr.name + loc(expr.loc));
                break;
            case ExprKind::Borrow:
                line(indent, std::string(expr.mutable_borrow ? "BorrowMut" : "Borrow") + loc(expr.loc));
                if (expr_operand(expr)) dump_expr(*expr_operand(expr), indent + 1);
                break;
            case ExprKind::Unary:
                line(indent, "Unary op=" + quote(op_name(expr.op)) + loc(expr.loc));
                if (expr_operand(expr)) dump_expr(*expr_operand(expr), indent + 1);
                break;
            case ExprKind::Cast:
                line(indent, "Cast type=" + type_ref(expr.cast_type) + loc(expr.loc));
                if (expr_operand(expr)) dump_expr(*expr_operand(expr), indent + 1);
                break;
            case ExprKind::Try:
                line(indent, "Try" + loc(expr.loc));
                if (expr_operand(expr)) dump_expr(*expr_operand(expr), indent + 1);
                break;
            case ExprKind::NullCoalesce:
                line(indent, "NullCoalesce" + loc(expr.loc));
                if (expr_left(expr)) dump_expr(*expr_left(expr), indent + 1);
                if (expr_right(expr)) dump_expr(*expr_right(expr), indent + 1);
                break;
            case ExprKind::Tuple:
                line(indent, "Tuple" + loc(expr.loc));
                dump_args(expr.args, indent + 1);
                break;
            case ExprKind::TupleIndex:
                line(indent, "TupleIndex index=" + std::to_string(expr.tuple_index) + loc(expr.loc));
                if (expr_operand(expr)) dump_expr(*expr_operand(expr), indent + 1);
                break;
            case ExprKind::Index:
                line(indent, "Index" + loc(expr.loc));
                if (expr_left(expr)) dump_expr(*expr_left(expr), indent + 1);
                if (expr_right(expr)) dump_expr(*expr_right(expr), indent + 1);
                break;
            case ExprKind::FieldAccess:
                line(indent, "Field name=" + expr.name + loc(expr.loc));
                if (expr_operand(expr)) dump_expr(*expr_operand(expr), indent + 1);
                break;
            case ExprKind::StructLiteral:
                line(indent, "StructLiteral name=" + expr.name + loc(expr.loc));
                dump_args(expr.args, indent + 1);
                break;
            case ExprKind::Vector:
                line(indent, "Vector" + loc(expr.loc));
                dump_args(expr.args, indent + 1);
                break;
            case ExprKind::MacroCall:
                line(indent, "MacroCall name=" + expr.name + loc(expr.loc));
                break;
            case ExprKind::MethodCall:
                line(indent, "MethodCall name=" + expr.name + loc(expr.loc));
                if (expr_operand(expr)) dump_expr(*expr_operand(expr), indent + 1);
                dump_args(expr.args, indent + 1);
                break;
            case ExprKind::Match:
                line(indent, "MatchExpr" + loc(expr.loc));
                if (expr_match_value(expr)) dump_expr(*expr_match_value(expr), indent + 1);
                for (const ExprMatchArm& arm : expr_match_arms(expr)) {
                    line(indent + 1, "Arm" + loc(arm.loc));
                    dump_pattern(arm.pattern, indent + 2);
                    if (arm.value) dump_expr(*arm.value, indent + 2);
                }
                break;
            case ExprKind::If:
                line(indent, "IfExpr" + loc(expr.loc));
                if (expr_if_condition(expr)) dump_expr(*expr_if_condition(expr), indent + 1);
                dump_statements(expr_if_then_body(expr), indent + 1);
                if (expr_if_then_value(expr)) dump_expr(*expr_if_then_value(expr), indent + 1);
                dump_statements(expr_if_else_body(expr), indent + 1);
                if (expr_if_else_value(expr)) dump_expr(*expr_if_else_value(expr), indent + 1);
                break;
            case ExprKind::Block:
                line(indent, "BlockExpr" + loc(expr.loc));
                dump_statements(expr_block_body(expr), indent + 1);
                if (expr_block_value(expr)) dump_expr(*expr_block_value(expr), indent + 1);
                break;
            case ExprKind::Lambda:
                line(indent, "LambdaExpr" + loc(expr.loc));
                for (const Param& param : expr_lambda_params(expr)) {
                    line(indent + 1, "Param name=" + quote(param.name));
                }
                dump_statements(expr_lambda_body(expr), indent + 1);
                if (expr_lambda_value(expr)) dump_expr(*expr_lambda_value(expr), indent + 1);
                break;
            case ExprKind::Binary:
                line(indent, "Binary op=" + quote(op_name(expr.op)) + loc(expr.loc));
                if (expr_left(expr)) dump_expr(*expr_left(expr), indent + 1);
                if (expr_right(expr)) dump_expr(*expr_right(expr), indent + 1);
                break;
            case ExprKind::Call:
                line(indent, "Call name=" + expr.name + loc(expr.loc));
                dump_args(expr.args, indent + 1);
                break;
        }
    }

    void dump_args(const ExprArgs& args, int indent) {
        for (const ExprPtr& arg : args) {
            if (arg) dump_expr(*arg, indent);
        }
    }

    void dump_pattern(const Pattern& pattern, int indent) {
        switch (pattern.kind) {
            case PatternKind::Wildcard:
                line(indent, "Pattern Wildcard" + loc(pattern.loc));
                break;
            case PatternKind::Binding:
                line(indent, "Pattern Binding name=" + pattern.case_name +
                                 " mode=" + binding_mode_name(pattern.binding_mode) + loc(pattern.loc));
                break;
            case PatternKind::EnumCase:
                line(indent, "Pattern EnumCase name=" + pattern.case_name + loc(pattern.loc));
                break;
            case PatternKind::IntegerLiteral:
                line(indent, "Pattern Integer value=" + std::to_string(pattern.int_value) + loc(pattern.loc));
                break;
            case PatternKind::BoolLiteral:
                line(indent, std::string("Pattern Bool value=") +
                                 (pattern.bool_value ? "true" : "false") + loc(pattern.loc));
                break;
            default:
                line(indent, "Pattern Complex" + loc(pattern.loc));
                break;
        }
    }
};

} // namespace

std::string dump_syntax(const Program& program, const std::string& source_name) {
    return SyntaxDumper(program, source_name).dump();
}

} // namespace ari
