#include "module_ir_summary.hpp"

#include "common.hpp"
#include "module_ir_type_summary.hpp"
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

struct IrSummaryParseResult {
    IrSummaryCounts counts;
    std::vector<ModuleCacheIrFunctionSummary> functions;
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
    append_field(out, module_cache_ir_type_name(type));
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

    IrSummaryParseResult parse() {
        expect(kModuleIrSummaryPayloadHeader);
        result_.counts.function_count = read_count();
        for (std::uint64_t i = 0; i < result_.counts.function_count; ++i) read_function();
        if (pos_ != text_.size()) fail("trailing data in IR summary");
        return std::move(result_);
    }

private:
    std::string text_;
    std::size_t pos_ = 0;
    IrSummaryParseResult result_;

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
        ModuleCacheIrFunctionSummary function;
        function.name = read_field();
        function.module_name = read_field();
        function.link_name = read_field();
        function.return_type = read_field();
        std::uint64_t param_count = read_count();
        for (std::uint64_t i = 0; i < param_count; ++i) {
            ModuleCacheIrParamSummary param;
            param.name = read_field();
            param.type = read_field();
            function.params.push_back(std::move(param));
        }
        function.body_statement_count = read_count();
        function.body = read_module_cache_ir_summary_body_payload(text_, pos_);
        if (function.body_statement_count != function.body.statements.size()) {
            fail("function '" + function.name + "' records body statement count " +
                 std::to_string(function.body_statement_count) + " but body tree contains " +
                 std::to_string(function.body.statements.size()));
        }
        function.shared_export = read_bool_field("shared-export");
        result_.functions.push_back(std::move(function));
    }
};

IrSummaryParseResult parse_ir_summary_payload(const ModuleCacheIrSummary& summary) {
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

IrSummaryParseResult parse_validated_ir_summary_payload(const ModuleCacheIrSummary& summary,
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
        IrSummaryParseResult result = parse_ir_summary_payload(summary);
        require_count_match(summary.function_count, result.counts.function_count, "function", summary);
        return result;
    } catch (const CompileError& error) {
        throw CompileError("invalid module cache '" + display_path + "': " + error.what());
    }
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
    (void)parse_validated_ir_summary_payload(summary, display_path);
}

std::vector<ModuleCacheIrFunctionSummary>
materialize_module_cache_ir_summary_functions(const ModuleCacheIrSummary& summary,
                                              const std::string& display_path) {
    IrSummaryParseResult result = parse_validated_ir_summary_payload(summary, display_path);
    return std::move(result.functions);
}

} // namespace ari
