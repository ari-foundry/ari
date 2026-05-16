#include "module_ir_summary.hpp"

#include "common.hpp"
#include "module_cache_format.hpp"
#include "module_metadata.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace ari {

namespace {

struct IrSummaryCounts {
    std::uint64_t function_count = 0;
};

struct IrBodyShape {
    std::map<std::string, std::uint64_t> statements;
    std::map<std::string, std::uint64_t> expressions;
};

std::string bool_key(bool value) {
    return value ? "1" : "0";
}

std::string display_module_name(const std::string& module_name) {
    return module_name.empty() ? "<root>" : module_name;
}

std::string summary_display(const ModuleCacheIrSummary& summary) {
    return "'" + display_module_name(summary.module_name) + "' at '" + summary.path + "'";
}

void append_count(std::string& out, std::uint64_t value) {
    out += std::to_string(value);
    out.push_back(';');
}

void append_field(std::string& out, const std::string& value) {
    out += std::to_string(value.size());
    out.push_back(':');
    out += value;
    out.push_back(';');
}

void append_type(std::string& out, const IrType& type) {
    append_field(out, type_name(type));
}

std::string stmt_kind_name(IrStmtKind kind) {
    switch (kind) {
        case IrStmtKind::Block: return "block";
        case IrStmtKind::VarDecl: return "var";
        case IrStmtKind::Assign: return "assign";
        case IrStmtKind::ExprStmt: return "expr";
        case IrStmtKind::Return: return "return";
        case IrStmtKind::If: return "if";
        case IrStmtKind::While: return "while";
        case IrStmtKind::WhileLet: return "while-let";
        case IrStmtKind::ForRange: return "for-range";
        case IrStmtKind::ForVector: return "for-vector";
        case IrStmtKind::InitWhile: return "init-while";
        case IrStmtKind::Continue: return "continue";
        case IrStmtKind::Break: return "break";
        case IrStmtKind::Match: return "match";
        case IrStmtKind::Drop: return "drop";
    }
    return "unknown";
}

std::string expr_kind_name(IrExprKind kind) {
    switch (kind) {
        case IrExprKind::Integer: return "integer";
        case IrExprKind::Float: return "float";
        case IrExprKind::String: return "string";
        case IrExprKind::Bool: return "bool";
        case IrExprKind::Null: return "null";
        case IrExprKind::FunctionRef: return "function-ref";
        case IrExprKind::Local: return "local";
        case IrExprKind::Borrow: return "borrow";
        case IrExprKind::Unary: return "unary";
        case IrExprKind::Cast: return "cast";
        case IrExprKind::PointerOffset: return "pointer-offset";
        case IrExprKind::PointerAdd: return "pointer-add";
        case IrExprKind::PointerLoad: return "pointer-load";
        case IrExprKind::PointerStore: return "pointer-store";
        case IrExprKind::Try: return "try";
        case IrExprKind::NullCoalesce: return "null-coalesce";
        case IrExprKind::EnumTag: return "enum-tag";
        case IrExprKind::EnumConstruct: return "enum-construct";
        case IrExprKind::Tuple: return "tuple";
        case IrExprKind::TupleIndex: return "tuple-index";
        case IrExprKind::Index: return "index";
        case IrExprKind::SliceRange: return "slice-range";
        case IrExprKind::Vector: return "vector";
        case IrExprKind::VectorPush: return "vector-push";
        case IrExprKind::VectorPop: return "vector-pop";
        case IrExprKind::VectorReserve: return "vector-reserve";
        case IrExprKind::VectorClear: return "vector-clear";
        case IrExprKind::VectorTruncate: return "vector-truncate";
        case IrExprKind::VectorSet: return "vector-set";
        case IrExprKind::VectorSwap: return "vector-swap";
        case IrExprKind::VectorRemove: return "vector-remove";
        case IrExprKind::VectorInsert: return "vector-insert";
        case IrExprKind::VectorContains: return "vector-contains";
        case IrExprKind::VectorIndexOf: return "vector-index-of";
        case IrExprKind::VectorCount: return "vector-count";
        case IrExprKind::Noop: return "noop";
        case IrExprKind::FormatPrint: return "format-print";
        case IrExprKind::Match: return "match";
        case IrExprKind::If: return "if";
        case IrExprKind::Block: return "block";
        case IrExprKind::IndirectCall: return "indirect-call";
        case IrExprKind::TraitObjectCall: return "trait-object-call";
        case IrExprKind::Binary: return "binary";
        case IrExprKind::Call: return "call";
    }
    return "unknown";
}

void collect_body_shape_expr(IrBodyShape& shape, const IrExprPtr& expr);
void collect_body_shape_stmts(IrBodyShape& shape, const std::vector<IrStmtPtr>& statements);

void collect_body_shape_exprs(IrBodyShape& shape, const std::vector<IrExprPtr>& expressions) {
    for (const auto& expr : expressions) collect_body_shape_expr(shape, expr);
}

void collect_body_shape_stmts(IrBodyShape& shape, const std::vector<IrStmtPtr>& statements) {
    for (const auto& statement : statements) {
        if (!statement) continue;
        ++shape.statements[stmt_kind_name(statement->kind)];
        collect_body_shape_expr(shape, statement->binding.init);
        collect_body_shape_expr(shape, ir_stmt_assign_target(*statement));
        collect_body_shape_expr(shape, ir_stmt_assign_rhs(*statement));
        collect_body_shape_expr(shape, statement->expr);
        collect_body_shape_expr(shape, statement->condition);
        collect_body_shape_stmts(shape, ir_stmt_statements(*statement));
        collect_body_shape_stmts(shape, ir_stmt_then_body(*statement));
        collect_body_shape_stmts(shape, ir_stmt_else_body(*statement));
        collect_body_shape_stmts(shape, ir_stmt_loop_body(*statement));
        collect_body_shape_expr(shape, ir_stmt_for_start(*statement));
        collect_body_shape_expr(shape, ir_stmt_for_end(*statement));
        collect_body_shape_exprs(shape, ir_stmt_for_values(*statement));
        collect_body_shape_expr(shape, statement->match_value);
        for (const auto& binding : statement->init_bindings) collect_body_shape_expr(shape, binding.init);
        collect_body_shape_exprs(shape, statement->updates);
        for (const auto& arm : ir_stmt_match_arms(*statement)) collect_body_shape_stmts(shape, arm.body);
        collect_body_shape_expr(shape, ir_stmt_break_value(*statement));
    }
}

void collect_body_shape_expr(IrBodyShape& shape, const IrExprPtr& expr) {
    if (!expr) return;
    ++shape.expressions[expr_kind_name(expr->kind)];
    collect_body_shape_expr(shape, ir_expr_operand(*expr));
    collect_body_shape_expr(shape, ir_expr_left(*expr));
    collect_body_shape_expr(shape, ir_expr_right(*expr));
    collect_body_shape_expr(shape, ir_expr_payload(*expr));
    for (const auto& arg : expr->args) collect_body_shape_expr(shape, arg);
    collect_body_shape_expr(shape, ir_expr_if_condition(*expr));
    collect_body_shape_stmts(shape, ir_expr_if_then_body(*expr));
    collect_body_shape_expr(shape, ir_expr_if_then_value(*expr));
    collect_body_shape_stmts(shape, ir_expr_if_else_body(*expr));
    collect_body_shape_expr(shape, ir_expr_if_else_value(*expr));
    collect_body_shape_expr(shape, ir_expr_block_value(*expr));
    collect_body_shape_stmts(shape, ir_expr_block_body(*expr));
    collect_body_shape_expr(shape, ir_expr_match_value(*expr));
    for (const auto& arm : ir_expr_match_arms(*expr)) {
        collect_body_shape_stmts(shape, arm.body);
        collect_body_shape_expr(shape, arm.value);
    }
    collect_body_shape_stmts(shape, ir_expr_try_residual_cleanup(*expr));
}

void append_count_map(std::string& out, const std::map<std::string, std::uint64_t>& counts) {
    append_count(out, counts.size());
    for (const auto& item : counts) {
        append_field(out, item.first);
        append_count(out, item.second);
    }
}

void append_body_shape(std::string& out, const std::vector<IrStmtPtr>& body) {
    IrBodyShape shape;
    collect_body_shape_stmts(shape, body);
    out += "B;";
    append_count_map(out, shape.statements);
    append_count_map(out, shape.expressions);
}

void append_function_summary(std::string& out, const IrFunction& fn) {
    out += "F;";
    append_field(out, fn.name);
    append_field(out, fn.module_name);
    append_field(out, fn.link_name);
    append_type(out, fn.return_type);
    append_count(out, fn.params.size());
    for (const auto& param : fn.params) {
        append_field(out, param.name);
        append_type(out, param.type);
    }
    append_count(out, fn.body.size());
    append_body_shape(out, fn.body);
    append_field(out, bool_key(fn.shared_export));
}

std::string ir_summary_payload(const std::vector<const IrFunction*>& functions) {
    std::string out = kModuleIrSummaryPayloadHeader;
    append_count(out, functions.size());
    for (const IrFunction* fn : functions) append_function_summary(out, *fn);
    return out;
}

class IrSummaryReader {
public:
    explicit IrSummaryReader(std::string text)
        : text_(std::move(text)) {}

    IrSummaryCounts parse() {
        expect(kModuleIrSummaryPayloadHeader);
        IrSummaryCounts counts;
        counts.function_count = read_count();
        for (std::uint64_t i = 0; i < counts.function_count; ++i) read_function();
        if (pos_ != text_.size()) fail("trailing data in IR summary");
        return counts;
    }

private:
    std::string text_;
    std::size_t pos_ = 0;

    [[noreturn]] void fail(const std::string& message) const {
        throw CompileError("IR summary " + message);
    }

    void expect(const std::string& expected) {
        if (text_.compare(pos_, expected.size(), expected) != 0) {
            fail("expected '" + expected + "' header");
        }
        pos_ += expected.size();
    }

    void expect_char(char expected) {
        if (pos_ >= text_.size() || text_[pos_] != expected) {
            fail(std::string("expected '") + expected + "'");
        }
        ++pos_;
    }

    std::uint64_t read_unsigned_until(char delimiter, const std::string& label) {
        if (pos_ >= text_.size() || !std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
            fail("expected " + label);
        }
        std::uint64_t value = 0;
        while (pos_ < text_.size() && text_[pos_] != delimiter) {
            char c = text_[pos_++];
            if (!std::isdigit(static_cast<unsigned char>(c))) fail("expected " + label);
            std::uint64_t digit = static_cast<std::uint64_t>(c - '0');
            if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10) {
                fail(label + " is too large");
            }
            value = value * 10 + digit;
        }
        if (pos_ >= text_.size()) fail("unterminated " + label);
        expect_char(delimiter);
        return value;
    }

    std::uint64_t read_count() {
        return read_unsigned_until(';', "count");
    }

    std::string read_field() {
        std::uint64_t length = read_unsigned_until(':', "field length");
        if (length > text_.size() - pos_) fail("field extends past end of summary");
        std::size_t start = pos_;
        pos_ += static_cast<std::size_t>(length);
        expect_char(';');
        return text_.substr(start, static_cast<std::size_t>(length));
    }

    void read_function() {
        expect("F;");
        read_field(); // name
        read_field(); // module name
        read_field(); // link name
        read_field(); // return type
        std::uint64_t param_count = read_count();
        for (std::uint64_t i = 0; i < param_count; ++i) {
            read_field(); // parameter name
            read_field(); // parameter type
        }
        read_count(); // body statement count
        if (peek("B;")) read_body_shape();
        std::string shared_export = read_field();
        if (shared_export != "0" && shared_export != "1") {
            fail("expected shared-export boolean 0 or 1");
        }
    }

    bool peek(const std::string& expected) const {
        return text_.compare(pos_, expected.size(), expected) == 0;
    }

    void read_count_map(const std::string& label) {
        std::uint64_t entry_count = read_count();
        for (std::uint64_t i = 0; i < entry_count; ++i) {
            std::string name = read_field();
            if (name.empty()) fail(label + " kind cannot be empty");
            read_count();
        }
    }

    void read_body_shape() {
        expect("B;");
        read_count_map("statement");
        read_count_map("expression");
    }
};

IrSummaryCounts parse_ir_summary_payload(const ModuleCacheIrSummary& summary) {
    IrSummaryReader reader(summary.ir_summary);
    return reader.parse();
}

void require_count_match(std::uint64_t recorded,
                         std::uint64_t parsed,
                         const std::string& label,
                         const ModuleCacheIrSummary& summary) {
    if (recorded == parsed) return;
    throw CompileError("module cache IR summary for " + summary_display(summary) +
                       " has " + label + " count " + std::to_string(recorded) +
                       " but IR summary contains " + std::to_string(parsed));
}

std::vector<const IrFunction*> functions_for_module(const IrProgram& ir,
                                                    const std::string& module_name) {
    std::vector<const IrFunction*> functions;
    for (const auto& fn : ir.functions) {
        if (fn.module_name == module_name) functions.push_back(&fn);
    }
    std::sort(functions.begin(), functions.end(), [](const IrFunction* left, const IrFunction* right) {
        if (left->name != right->name) return left->name < right->name;
        return left->link_name < right->link_name;
    });
    return functions;
}

} // namespace

ModuleCacheIrSummary make_module_cache_ir_summary(const ModuleCacheSource& source,
                                                  const IrProgram& ir) {
    std::vector<const IrFunction*> functions = functions_for_module(ir, source.module_name);

    ModuleCacheIrSummary summary;
    summary.module_name = source.module_name;
    summary.path = source.path;
    summary.content_hash = source.content_hash;
    summary.ir_summary = ir_summary_payload(functions);
    summary.ir_hash = module_metadata_source_hash(summary.ir_summary);
    summary.is_root = source.is_root;
    summary.function_count = functions.size();
    return summary;
}

void attach_module_cache_ir_summaries(ModuleCache& cache, const IrProgram& ir) {
    cache.ir_summaries.clear();
    cache.ir_summaries.reserve(cache.sources.size());
    for (const auto& source : cache.sources) {
        cache.ir_summaries.push_back(make_module_cache_ir_summary(source, ir));
    }
}

void require_valid_module_cache_ir_summary_payload(const ModuleCacheIrSummary& summary,
                                                   const std::string& display_path) {
    if (summary.ir_summary.empty()) {
        throw CompileError("invalid module cache '" + display_path +
                           "': IR summary for " + summary_display(summary) +
                           " is missing an IR summary");
    }
    if (summary.ir_summary.compare(
            0,
            std::strlen(kModuleIrSummaryPayloadHeader),
            kModuleIrSummaryPayloadHeader
        ) != 0) {
        throw CompileError("invalid module cache '" + display_path +
                           "': IR summary for " + summary_display(summary) +
                           " expected '" + std::string(kModuleIrSummaryPayloadHeader) + "' header");
    }
    std::string hash = module_metadata_source_hash(summary.ir_summary);
    if (hash != summary.ir_hash) {
        throw CompileError("invalid module cache '" + display_path +
                           "': IR summary for " + summary_display(summary) +
                           " hashes to '" + hash + "' instead of recorded '" +
                           summary.ir_hash + "'");
    }
    try {
        IrSummaryCounts counts = parse_ir_summary_payload(summary);
        require_count_match(summary.function_count, counts.function_count, "function", summary);
    } catch (const CompileError& error) {
        throw CompileError("invalid module cache '" + display_path + "': " + error.what());
    }
}

} // namespace ari
