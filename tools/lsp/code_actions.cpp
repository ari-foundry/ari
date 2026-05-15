#include "code_actions.hpp"

#include "../ari_tooling/diagnostic.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace ari::lsp {
namespace {

struct TextEdit {
    int start_line = 0;
    int start_character = 0;
    int end_line = 0;
    int end_character = 0;
    std::string new_text;
};

bool trailing_space(char c) {
    return c == ' ' || c == '\t';
}

std::string text_edit_json(const TextEdit& edit) {
    std::ostringstream out;
    out << "{";
    out << "\"range\":{";
    out << "\"start\":{\"line\":" << edit.start_line << ",\"character\":" << edit.start_character << "},";
    out << "\"end\":{\"line\":" << edit.end_line << ",\"character\":" << edit.end_character << "}";
    out << "},";
    out << "\"newText\":\"" << tooling::json_escape(edit.new_text) << "\"";
    out << "}";
    return out.str();
}

std::string workspace_edit_json(const std::string& uri, const std::vector<TextEdit>& edits) {
    std::ostringstream out;
    out << "{";
    out << "\"changes\":{";
    out << "\"" << tooling::json_escape(uri) << "\":[";
    bool first = true;
    for (const TextEdit& edit : edits) {
        if (!first) out << ",";
        first = false;
        out << text_edit_json(edit);
    }
    out << "]";
    out << "}";
    out << "}";
    return out.str();
}

std::string code_action_json(const std::string& title,
                             const std::string& kind,
                             const std::string& uri,
                             const std::vector<TextEdit>& edits) {
    std::ostringstream out;
    out << "{";
    out << "\"title\":\"" << tooling::json_escape(title) << "\",";
    out << "\"kind\":\"" << tooling::json_escape(kind) << "\",";
    out << "\"edit\":" << workspace_edit_json(uri, edits);
    out << "}";
    return out.str();
}

std::vector<TextEdit> trailing_whitespace_edits(const std::string& text) {
    std::vector<TextEdit> edits;
    std::istringstream in(text);
    std::string line;
    int line_number = 0;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::size_t end = line.size();
        while (end > 0 && trailing_space(line[end - 1])) --end;
        if (end != line.size()) {
            edits.push_back(TextEdit{
                line_number,
                static_cast<int>(end),
                line_number,
                static_cast<int>(line.size()),
                "",
            });
        }
        ++line_number;
    }
    return edits;
}

TextEdit final_newline_edit(const std::string& text) {
    int line = 0;
    int character = 0;
    for (char c : text) {
        if (c == '\n') {
            ++line;
            character = 0;
        } else {
            ++character;
        }
    }
    return TextEdit{line, character, line, character, "\n"};
}

bool needs_final_newline(const std::string& text) {
    return !text.empty() && text.back() != '\n';
}

} // namespace

std::string code_actions_response(const std::string& id,
                                  const std::string& uri,
                                  const std::string& text) {
    std::vector<TextEdit> trailing = trailing_whitespace_edits(text);
    std::vector<TextEdit> final_newline;
    if (needs_final_newline(text)) final_newline.push_back(final_newline_edit(text));

    std::vector<TextEdit> all = trailing;
    all.insert(all.end(), final_newline.begin(), final_newline.end());

    std::ostringstream out;
    out << "{";
    out << "\"jsonrpc\":\"2.0\",";
    out << "\"id\":" << (id.empty() ? "null" : id) << ",";
    out << "\"result\":[";
    bool first = true;
    if (!trailing.empty()) {
        out << code_action_json("Remove trailing whitespace", "quickfix", uri, trailing);
        first = false;
    }
    if (!final_newline.empty()) {
        if (!first) out << ",";
        out << code_action_json("Insert final newline", "quickfix", uri, final_newline);
        first = false;
    }
    if (!all.empty()) {
        if (!first) out << ",";
        out << code_action_json("Apply Ari lint fixes", "source.fixAll.ari", uri, all);
    }
    out << "]}";
    return out.str();
}

} // namespace ari::lsp
