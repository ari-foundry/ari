#include "source_map_dump.hpp"

#include <algorithm>
#include <sstream>

namespace ari {

namespace {

struct SourceLine {
    std::size_t number = 1;
    std::size_t byte_start = 0;
    std::size_t byte_len = 0;
    std::string newline;
    std::string text;
};

std::string bool_text(bool value) {
    return value ? "true" : "false";
}

std::string module_name_text(const std::string& module_name) {
    return module_name.empty() ? "<root>" : module_name;
}

std::string escape_text(const std::string& text) {
    std::string escaped;
    for (unsigned char c : text) {
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (c < 0x20 || c == 0x7f) {
                    const char* digits = "0123456789abcdef";
                    escaped += "\\x";
                    escaped.push_back(digits[(c >> 4) & 0xf]);
                    escaped.push_back(digits[c & 0xf]);
                } else {
                    escaped.push_back(static_cast<char>(c));
                }
                break;
        }
    }
    return escaped;
}

std::vector<SourceLine> source_lines(const std::string& text) {
    std::vector<SourceLine> lines;
    if (text.empty()) {
        lines.push_back(SourceLine{1, 0, 0, "none", ""});
        return lines;
    }

    std::size_t line_start = 0;
    std::size_t line_number = 1;
    while (line_start < text.size()) {
        std::size_t newline_pos = text.find('\n', line_start);
        if (newline_pos == std::string::npos) {
            std::string line_text = text.substr(line_start);
            lines.push_back(SourceLine{line_number, line_start, line_text.size(), "none", std::move(line_text)});
            break;
        }

        std::size_t line_end = newline_pos;
        std::string newline = "lf";
        if (line_end > line_start && text[line_end - 1] == '\r') {
            --line_end;
            newline = "crlf";
        }
        std::string line_text = text.substr(line_start, line_end - line_start);
        lines.push_back(SourceLine{line_number, line_start, line_text.size(), newline, std::move(line_text)});
        line_start = newline_pos + 1;
        ++line_number;
    }
    return lines;
}

std::string source_key(const SourceMapDumpFile& file) {
    return file.module_name + "\t" + file.path + "\t" + (file.is_root ? "1" : "0");
}

} // namespace

std::string dump_source_map(const std::string& source_name, std::vector<SourceMapDumpFile> files) {
    std::sort(files.begin(), files.end(), [](const auto& left, const auto& right) {
        return source_key(left) < source_key(right);
    });

    std::ostringstream out;
    out << "SourceMap source=" << source_name << " files=" << files.size() << "\n";
    for (const auto& file : files) {
        std::vector<SourceLine> lines = source_lines(file.text);
        bool trailing_newline = !file.text.empty() && file.text.back() == '\n';
        out << "  File module=" << module_name_text(file.module_name)
            << " root=" << bool_text(file.is_root)
            << " path=" << file.path
            << " bytes=" << file.text.size()
            << " lines=" << lines.size()
            << " trailing_newline=" << bool_text(trailing_newline) << "\n";
        for (const auto& line : lines) {
            out << "    Line number=" << line.number
                << " byte_start=" << line.byte_start
                << " byte_len=" << line.byte_len
                << " newline=" << line.newline
                << " text=\"" << escape_text(line.text) << "\"\n";
        }
    }
    return out.str();
}

} // namespace ari
