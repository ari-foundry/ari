#include "folding.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace ari::lsp {
namespace {

struct FoldingRange {
    int start_line = 0;
    int end_line = 0;
};

std::string folding_range_json(const FoldingRange& range) {
    std::ostringstream out;
    out << "{";
    out << "\"startLine\":" << range.start_line << ",";
    out << "\"endLine\":" << range.end_line;
    out << "}";
    return out.str();
}

std::vector<FoldingRange> collect_brace_folds(const std::string& text) {
    std::vector<FoldingRange> ranges;
    std::vector<int> stack;
    int line_number = 0;
    for (char c : text) {
        if (c == '{') {
            stack.push_back(line_number);
        } else if (c == '}') {
            if (!stack.empty()) {
                int start_line = stack.back();
                stack.pop_back();
                if (line_number > start_line) {
                    ranges.push_back(FoldingRange{start_line, line_number});
                }
            }
        } else if (c == '\n') {
            ++line_number;
        }
    }
    return ranges;
}

} // namespace

std::string folding_ranges_response(const std::string& id, const std::string& text) {
    std::ostringstream out;
    out << "{";
    out << "\"jsonrpc\":\"2.0\",";
    out << "\"id\":" << (id.empty() ? "null" : id) << ",";
    out << "\"result\":[";
    bool first = true;
    for (const FoldingRange& range : collect_brace_folds(text)) {
        if (!first) out << ",";
        first = false;
        out << folding_range_json(range);
    }
    out << "]}";
    return out.str();
}

} // namespace ari::lsp
