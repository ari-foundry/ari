#include "module_ir_summary.hpp"

#include "common.hpp"
#include "module_cache_format.hpp"
#include "module_ir_summary_body.hpp"
#include "module_metadata.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace ari {

namespace {

struct IrSummaryCounts {
    std::uint64_t function_count = 0;
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
    append_ir_summary_body_shape(out, fn.body);
    append_ir_summary_body_tree(out, fn.body);
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

    bool read_bool_field(const std::string& label) {
        std::string value = read_field();
        if (value != "0" && value != "1") fail("expected " + label + " boolean 0 or 1");
        return value == "1";
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
        read_body_shape();
        read_body_tree();
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

    bool read_null_tree() {
        if (!peek("N;")) return false;
        expect("N;");
        return true;
    }

    std::string read_non_empty_field(const std::string& label) {
        std::string value = read_field();
        if (value.empty()) fail(label + " cannot be empty");
        return value;
    }

    void read_type_list() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) read_field();
    }

    void read_expr_tree_list() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) read_expr_tree();
    }

    void read_stmt_tree_list() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) read_stmt_tree();
    }

    void read_binding_snapshot() {
        read_field(); // name
        read_field(); // type
        read_bool_field("binding mutability");
        read_expr_tree();
    }

    void read_payload_binding() {
        read_count(); // index
        read_field(); // name
        read_field(); // type
        read_bool_field("compact enum payload");
        read_field(); // compact enum type
        read_count(); // compact enum payload index
    }

    void read_payload_bindings() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) read_payload_binding();
    }

    void read_payload_literal_conditions() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) {
            read_count(); // index
            read_bool_field("payload literal is-bool");
            read_field(); // literal payload
        }
    }

    void read_payload_range_conditions() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) {
            read_count(); // index
            read_field(); // start
            read_field(); // end
            read_bool_field("payload range inclusive");
            read_bool_field("payload range unsigned");
            read_field(); // type
            read_bool_field("compact enum payload");
        }
    }

    void read_payload_enum_conditions() {
        std::uint64_t count = read_count();
        for (std::uint64_t i = 0; i < count; ++i) {
            read_count(); // index
            read_field(); // enum type
            read_count(); // tag
            read_count(); // nested payload index
            read_bool_field("payload enum literal presence");
            read_bool_field("payload enum literal is-bool");
            read_field(); // payload literal
            read_bool_field("payload enum literal negative");
            read_bool_field("payload enum range presence");
            read_field(); // range start
            read_field(); // range end
            read_bool_field("payload enum range inclusive");
            read_bool_field("payload enum range unsigned");
            read_field(); // payload type
        }
    }

    void read_match_arm_pattern() {
        read_bool_field("match-arm wildcard");
        read_bool_field("match-arm literal presence");
        read_bool_field("match-arm literal is-bool");
        read_field(); // literal payload
        read_bool_field("match-arm literal negative");
        read_bool_field("match-arm range presence");
        read_field(); // range start
        read_field(); // range end
        read_bool_field("match-arm range inclusive");
        read_bool_field("match-arm range unsigned");
        read_field(); // case name
        read_count(); // enum tag
        read_bool_field("match-arm value binding presence");
        read_field(); // value binding name
        read_field(); // value binding type
        read_bool_field("match-arm payload binding presence");
        read_field(); // payload binding name
        read_field(); // payload binding type
        read_count(); // payload index
        read_payload_bindings();
        read_payload_literal_conditions();
        read_payload_range_conditions();
        read_payload_enum_conditions();
    }

    void read_stmt_match_arm_tree() {
        read_match_arm_pattern();
        read_stmt_tree_list();
    }

    void read_expr_match_arm_tree() {
        read_match_arm_pattern();
        read_stmt_tree_list();
        read_expr_tree();
    }

    void read_format_print_payload() {
        std::uint64_t part_count = read_count();
        for (std::uint64_t i = 0; i < part_count; ++i) read_field();
        std::uint64_t spec_count = read_count();
        for (std::uint64_t i = 0; i < spec_count; ++i) read_field();
        read_bool_field("format print newline");
    }

    void read_expr_tree() {
        if (read_null_tree()) return;
        expect("E;");
        read_non_empty_field("expression kind");
        read_field(); // result type
        read_field(); // name
        read_field(); // label
        read_field(); // string value
        read_field(); // borrow source name
        read_field(); // borrow source path
        read_field(); // enum name
        read_field(); // case name
        read_field(); // scalar payload
        read_field(); // unary op
        read_field(); // binary op
        read_bool_field("mutable borrow");
        read_count(); // enum tag
        read_bool_field("enum payload presence");
        read_field(); // enum payload type
        read_expr_tree(); // operand
        read_expr_tree(); // left
        read_expr_tree(); // right
        read_expr_tree(); // payload
        read_expr_tree_list(); // args
        read_type_list(); // call param types
        read_expr_tree(); // if condition
        read_stmt_tree_list(); // then body
        read_expr_tree(); // then value
        read_stmt_tree_list(); // else body
        read_expr_tree(); // else value
        read_field(); // block label
        read_stmt_tree_list(); // block body
        read_expr_tree(); // block value
        read_expr_tree(); // match value
        std::uint64_t arm_count = read_count();
        for (std::uint64_t i = 0; i < arm_count; ++i) read_expr_match_arm_tree();
        read_format_print_payload();
        read_bool_field("try residual conversion");
        read_bool_field("try residual payload presence");
        read_field(); // try return residual payload type
        read_count(); // try return residual tag
        read_stmt_tree_list(); // try residual cleanup
    }

    void read_stmt_tree() {
        if (read_null_tree()) return;
        expect("S;");
        read_non_empty_field("statement kind");
        read_field(); // label
        read_field(); // drop name
        read_field(); // assign name
        read_binding_snapshot();
        read_expr_tree(); // assign target
        read_expr_tree(); // assign rhs
        read_expr_tree(); // expression
        read_expr_tree(); // condition
        read_stmt_tree_list(); // statements
        read_stmt_tree_list(); // then body
        read_stmt_tree_list(); // else body
        read_stmt_tree_list(); // loop body
        read_field(); // for binding name
        read_field(); // for index name
        read_field(); // for end name
        read_field(); // for binding type
        read_bool_field("for inclusivity");
        read_expr_tree(); // for start
        read_expr_tree(); // for end
        read_expr_tree_list(); // for values
        read_expr_tree(); // match value
        std::uint64_t init_binding_count = read_count();
        for (std::uint64_t i = 0; i < init_binding_count; ++i) read_binding_snapshot();
        read_expr_tree_list(); // updates
        std::uint64_t arm_count = read_count();
        for (std::uint64_t i = 0; i < arm_count; ++i) read_stmt_match_arm_tree();
        read_field(); // break label
        read_expr_tree(); // break value
    }

    void read_body_tree() {
        expect("T;");
        read_stmt_tree_list();
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
