#include "json_rpc.hpp"

#include <cctype>
#include <iostream>
#include <sstream>

namespace ari::lsp {
namespace {

std::optional<std::size_t> parse_content_length(const std::string& line) {
    const std::string prefix = "Content-Length:";
    if (line.rfind(prefix, 0) != 0) return std::nullopt;
    std::size_t pos = prefix.size();
    while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) ++pos;
    return static_cast<std::size_t>(std::stoul(line.substr(pos)));
}

std::string decode_json_string(const std::string& text, std::size_t& pos) {
    std::string out;
    if (pos >= text.size() || text[pos] != '"') return out;
    ++pos;
    while (pos < text.size()) {
        char c = text[pos++];
        if (c == '"') break;
        if (c != '\\') {
            out.push_back(c);
            continue;
        }
        if (pos >= text.size()) break;
        char escaped = text[pos++];
        switch (escaped) {
            case '"': out.push_back('"'); break;
            case '\\': out.push_back('\\'); break;
            case '/': out.push_back('/'); break;
            case 'b': out.push_back('\b'); break;
            case 'f': out.push_back('\f'); break;
            case 'n': out.push_back('\n'); break;
            case 'r': out.push_back('\r'); break;
            case 't': out.push_back('\t'); break;
            default: out.push_back(escaped); break;
        }
    }
    return out;
}

std::size_t find_field_value(const std::string& json, const std::string& key) {
    std::string quoted = "\"" + key + "\"";
    std::size_t pos = json.find(quoted);
    if (pos == std::string::npos) return std::string::npos;
    pos = json.find(':', pos + quoted.size());
    if (pos == std::string::npos) return std::string::npos;
    ++pos;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) ++pos;
    return pos;
}

} // namespace

std::optional<RpcMessage> read_message(std::istream& in) {
    std::string line;
    std::optional<std::size_t> content_length;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;
        if (auto length = parse_content_length(line)) content_length = length;
    }
    if (!in && !content_length) return std::nullopt;
    if (!content_length) return std::nullopt;

    std::string body(*content_length, '\0');
    in.read(&body[0], static_cast<std::streamsize>(*content_length));
    if (in.gcount() != static_cast<std::streamsize>(*content_length)) return std::nullopt;
    return RpcMessage{body};
}

void write_message(std::ostream& out, const std::string& body) {
    out << "Content-Length: " << body.size() << "\r\n\r\n" << body;
    out.flush();
}

std::string json_string_field(const std::string& json, const std::string& key) {
    std::size_t pos = find_field_value(json, key);
    if (pos == std::string::npos || pos >= json.size() || json[pos] != '"') return "";
    return decode_json_string(json, pos);
}

std::string json_raw_field(const std::string& json, const std::string& key) {
    std::size_t pos = find_field_value(json, key);
    if (pos == std::string::npos) return "";
    std::size_t end = pos;
    int object_depth = 0;
    int array_depth = 0;
    bool in_string = false;
    bool escaped = false;
    while (end < json.size()) {
        char c = json[end];
        if (in_string) {
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') in_string = false;
            ++end;
            continue;
        }
        if (c == '"') in_string = true;
        else if (c == '{') ++object_depth;
        else if (c == '}') {
            if (object_depth == 0) break;
            --object_depth;
        } else if (c == '[') ++array_depth;
        else if (c == ']') {
            if (array_depth == 0) break;
            --array_depth;
        } else if (c == ',' && object_depth == 0 && array_depth == 0) {
            break;
        }
        ++end;
    }
    return json.substr(pos, end - pos);
}

} // namespace ari::lsp
