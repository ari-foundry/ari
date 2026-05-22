#include "common.hpp"

#include <algorithm>
#include <cctype>
#include <limits>
#include <map>
#include <sstream>
#include <utility>
#include <vector>

namespace ari {

namespace {

std::vector<SourceFile>& source_files() {
    static std::vector<SourceFile> sources;
    return sources;
}

std::map<std::string, SourceId>& source_path_index() {
    static std::map<std::string, SourceId> index;
    return index;
}

std::map<std::string, SourceId>& source_display_index() {
    static std::map<std::string, SourceId> index;
    return index;
}

std::size_t& in_memory_source_count() {
    static std::size_t count = 0;
    return count;
}

bool source_id_in_bounds(SourceId id) {
    return id.value >= 0 &&
           static_cast<std::size_t>(id.value) < source_files().size();
}

std::vector<std::size_t> build_line_starts(const std::string& text) {
    std::vector<std::size_t> starts;
    starts.push_back(0);
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') starts.push_back(i + 1);
    }
    return starts;
}

void reset_source_file_text(SourceFile& file, const std::string& text) {
    file.text = text;
    file.eof_offset = file.text.size();
    file.line_starts = build_line_starts(file.text);
}

void erase_source_display_index(SourceId id, const std::string& display_name) {
    auto found = source_display_index().find(display_name);
    if (found != source_display_index().end() && found->second.value == id.value) {
        source_display_index().erase(found);
    }
}

std::string fallback_display_name(const std::string& path,
                                  const std::string& display_name) {
    if (!display_name.empty()) return display_name;
    if (!path.empty()) return path;
    return "<memory>";
}

std::string fallback_source_path(const std::string& path,
                                 const std::string& display_name) {
    if (!path.empty()) return path;
    if (!display_name.empty()) return display_name;
    return "<memory>";
}

const SourceFile* source_file_for_location(SourceLocation& loc) {
    if (!valid_source_id(loc.source_id) && !loc.source_name.empty()) {
        loc.source_id = source_id_for_name(loc.source_name);
    }
    const SourceFile* file = find_source_file(loc.source_id);
    if (file != nullptr && loc.source_name.empty()) {
        loc.source_name = file->display_name;
    }
    return file;
}

std::size_t line_index_for_offset(const SourceFile& file, std::size_t offset) {
    offset = std::min(offset, file.eof_offset);
    auto found = std::upper_bound(file.line_starts.begin(), file.line_starts.end(), offset);
    if (found == file.line_starts.begin()) return 0;
    return static_cast<std::size_t>((found - file.line_starts.begin()) - 1);
}

std::size_t line_start_for_offset(const SourceFile& file, std::size_t offset) {
    return file.line_starts[line_index_for_offset(file, offset)];
}

std::size_t line_end_for_start(const SourceFile& file, std::size_t line_start) {
    std::size_t line_end = file.text.find('\n', line_start);
    if (line_end == std::string::npos) line_end = file.eof_offset;
    if (line_end > line_start && file.text[line_end - 1] == '\r') --line_end;
    return line_end;
}

bool decimal_text(const std::string& text) {
    if (text.empty()) return false;
    for (unsigned char c : text) {
        if (!std::isdigit(c)) return false;
    }
    return true;
}

bool parse_size(const std::string& text, std::size_t& value) {
    if (!decimal_text(text)) return false;
    value = 0;
    for (unsigned char c : text) {
        std::size_t digit = static_cast<std::size_t>(c - '0');
        if (value > (static_cast<std::size_t>(-1) - digit) / 10) return false;
        value = value * 10 + digit;
    }
    return true;
}

bool parse_int(const std::string& text, int& value) {
    std::size_t parsed = 0;
    if (!parse_size(text, parsed)) return false;
    if (parsed > static_cast<std::size_t>(std::numeric_limits<int>::max())) return false;
    value = static_cast<int>(parsed);
    return true;
}

bool split_location_prefix(const std::string& prefix,
                           std::string& source_name,
                           int& line,
                           int& column) {
    std::size_t column_colon = prefix.rfind(':');
    if (column_colon == std::string::npos) return false;
    std::size_t line_colon = prefix.rfind(':', column_colon - 1);
    std::string line_text;
    std::string column_text = prefix.substr(column_colon + 1);

    if (line_colon == std::string::npos) {
        source_name.clear();
        line_text = prefix.substr(0, column_colon);
    } else {
        source_name = prefix.substr(0, line_colon);
        line_text = prefix.substr(line_colon + 1, column_colon - line_colon - 1);
    }
    return parse_int(line_text, line) && parse_int(column_text, column);
}

bool parse_legacy_location_prefix(const std::string& raw,
                                  SourceLocation& loc,
                                  std::string& message) {
    std::size_t message_colon = raw.rfind(':');
    while (message_colon != std::string::npos) {
        std::string source_name;
        int line = 0;
        int column = 0;
        if (split_location_prefix(raw.substr(0, message_colon), source_name, line, column)) {
            SourceId source_id = source_id_for_name(source_name);
            if (!source_name.empty() && !valid_source_id(source_id)) {
                if (message_colon == 0) break;
                message_colon = raw.rfind(':', message_colon - 1);
                continue;
            }
            std::size_t text_start = message_colon + 1;
            if (text_start < raw.size() && raw[text_start] == ' ') ++text_start;
            loc.line = line;
            loc.column = column;
            loc.source_name = std::move(source_name);
            loc.source_id = source_id;
            message = raw.substr(text_start);
            return true;
        }
        if (message_colon == 0) break;
        message_colon = raw.rfind(':', message_colon - 1);
    }
    return false;
}

bool parse_structured_location_prefix(const std::string& raw,
                                      SourceLocation& loc,
                                      std::string& message) {
    const std::string marker = ":bytes=";
    std::size_t marker_pos = raw.find(marker);
    if (marker_pos == std::string::npos) return false;

    std::size_t range_start = marker_pos + marker.size();
    std::size_t range_split = raw.find("..", range_start);
    if (range_split == std::string::npos) return false;
    std::size_t range_end = raw.find(':', range_split + 2);
    if (range_end == std::string::npos) return false;

    std::string source_name;
    int line = 0;
    int column = 0;
    if (!split_location_prefix(raw.substr(0, marker_pos), source_name, line, column)) {
        return false;
    }

    std::size_t byte_start = 0;
    std::size_t byte_end = 0;
    if (!parse_size(raw.substr(range_start, range_split - range_start), byte_start) ||
        !parse_size(raw.substr(range_split + 2, range_end - range_split - 2), byte_end)) {
        return false;
    }

    std::size_t text_start = range_end + 1;
    if (text_start < raw.size() && raw[text_start] == ' ') ++text_start;
    loc.line = line;
    loc.column = column;
    loc.byte_start = byte_start;
    loc.byte_end = byte_end;
    loc.has_byte_range = true;
    loc.source_name = std::move(source_name);
    loc.source_id = source_id_for_name(loc.source_name);
    message = raw.substr(text_start);
    return true;
}

std::string render_compile_error(const SourceLocation* loc, const std::string& message) {
    if (loc == nullptr) return message;

    std::ostringstream out;
    SourceLocation normalized = *loc;
    (void)source_file_for_location(normalized);
    if (has_source_name(normalized)) out << normalized.source_name << ":";
    out << normalized.line << ":" << normalized.column << ": " << message;
    if (has_byte_span(normalized)) {
        out << "\n  source id: " << source_id_text(normalized.source_id)
            << "\n  byte span: [" << normalized.byte_start << ", " << normalized.byte_end << ")";
    }
    std::string snippet = render_source_snippet(normalized);
    if (!snippet.empty()) out << "\n" << snippet;
    return out.str();
}

} // namespace

CompileError::CompileError(std::string message) {
    SourceLocation parsed_loc;
    std::string parsed_message;
    if (parse_structured_location_prefix(message, parsed_loc, parsed_message) ||
        parse_legacy_location_prefix(message, parsed_loc, parsed_message)) {
        loc_ = std::move(parsed_loc);
        message_ = std::move(parsed_message);
        has_location_ = true;
        rendered_ = render_compile_error(&loc_, message_);
        return;
    }

    message_ = std::move(message);
    rendered_ = message_;
}

CompileError::CompileError(SourceLocation loc, std::string message)
    : message_(std::move(message)), loc_(std::move(loc)), has_location_(true) {
    (void)source_file_for_location(loc_);
    rendered_ = render_compile_error(&loc_, message_);
}

const char* CompileError::what() const noexcept {
    return rendered_.c_str();
}

const std::string& CompileError::message() const {
    return message_;
}

bool CompileError::has_location() const {
    return has_location_;
}

const SourceLocation& CompileError::location() const {
    return loc_;
}

std::string where(const SourceLocation& loc) {
    SourceLocation normalized = loc;
    (void)source_file_for_location(normalized);
    std::string text;
    if (has_source_name(normalized)) text += normalized.source_name + ":";
    text += std::to_string(normalized.line) + ":" + std::to_string(normalized.column);
    if (has_byte_span(normalized)) {
        text += ":bytes=" + std::to_string(normalized.byte_start) + ".." +
                std::to_string(normalized.byte_end);
    }
    return text;
}

bool valid_source_id(SourceId id) {
    return source_id_in_bounds(id);
}

std::string source_id_text(SourceId id) {
    if (!valid_source_id(id)) return "invalid";
    return std::to_string(id.value);
}

std::string source_kind_text(SourceKind kind) {
    switch (kind) {
        case SourceKind::Unknown: return "unknown";
        case SourceKind::File: return "file";
        case SourceKind::Generated: return "generated";
        case SourceKind::Builtin: return "builtin";
    }
    return "unknown";
}

bool has_source_name(const SourceLocation& loc) {
    return !loc.source_name.empty() || valid_source_id(loc.source_id);
}

bool has_byte_span(const SourceLocation& loc) {
    return loc.has_byte_range;
}

SourceId register_source_file(const std::string& path,
                              const std::string& display_name,
                              const std::string& text,
                              SourceKind kind) {
    std::string source_path = fallback_source_path(path, display_name);
    if (source_path.empty()) return SourceId{};
    std::string source_display = fallback_display_name(source_path, display_name);

    auto found = source_path_index().find(source_path);
    if (found != source_path_index().end()) {
        SourceId id = found->second;
        if (SourceFile* file = const_cast<SourceFile*>(find_source_file(id))) {
            erase_source_display_index(id, file->display_name);
            file->kind = kind;
            file->path = source_path;
            file->display_name = source_display;
            reset_source_file_text(*file, text);
            source_display_index()[file->display_name] = id;
        }
        return id;
    }

    SourceId id{static_cast<int>(source_files().size())};
    SourceFile file;
    file.id = id;
    file.kind = kind;
    file.path = std::move(source_path);
    file.display_name = std::move(source_display);
    reset_source_file_text(file, text);
    source_files().push_back(std::move(file));
    const SourceFile& stored = source_files().back();
    source_path_index().emplace(stored.path, id);
    source_display_index()[stored.display_name] = id;
    return id;
}

SourceId register_in_memory_source(const std::string& display_name,
                                   const std::string& text,
                                   SourceKind kind) {
    std::size_t index = in_memory_source_count()++;
    std::string source_path = "<memory:" + std::to_string(index) + ">";
    std::string source_display = display_name.empty() ? source_path : display_name;
    return register_source_file(source_path, source_display, text, kind);
}

SourceId register_source_text(const std::string& source_name,
                              const std::string& text,
                              SourceKind kind) {
    if (source_name.empty()) {
        SourceKind memory_kind = kind == SourceKind::File ? SourceKind::Unknown : kind;
        return register_in_memory_source("", text, memory_kind);
    }
    return register_source_file(source_name, source_name, text, kind);
}

SourceId source_id_for_name(const std::string& source_name) {
    auto found = source_path_index().find(source_name);
    if (found != source_path_index().end()) return found->second;
    found = source_display_index().find(source_name);
    if (found == source_display_index().end()) return SourceId{};
    return found->second;
}

const SourceFile* find_source_file(SourceId id) {
    if (!source_id_in_bounds(id)) return nullptr;
    return &source_files()[static_cast<std::size_t>(id.value)];
}

const std::string* find_source_text(SourceId id) {
    const SourceFile* file = find_source_file(id);
    if (file == nullptr) return nullptr;
    return &file->text;
}

const std::string* find_source_text(const std::string& source_name) {
    return find_source_text(source_id_for_name(source_name));
}

std::size_t source_eof_offset(SourceId id) {
    const SourceFile* file = find_source_file(id);
    if (file == nullptr) return 0;
    return file->eof_offset;
}

SourceLocation source_location_for_offset(SourceId id, std::size_t byte_offset) {
    SourceLocation loc;
    loc.source_id = id;
    const SourceFile* file = find_source_file(id);
    if (file == nullptr) return loc;

    byte_offset = std::min(byte_offset, file->eof_offset);
    std::size_t line_index = line_index_for_offset(*file, byte_offset);
    loc.source_name = file->display_name;
    loc.line = static_cast<int>(line_index + 1);
    loc.column = static_cast<int>(byte_offset - file->line_starts[line_index] + 1);
    loc.byte_start = byte_offset;
    loc.byte_end = byte_offset;
    loc.has_byte_range = true;
    return loc;
}

SourceLocation source_location_for_span(SourceId id, std::size_t byte_start, std::size_t byte_end) {
    SourceLocation loc = source_location_for_offset(id, byte_start);
    const SourceFile* file = find_source_file(id);
    if (file == nullptr) return loc;
    loc.byte_start = std::min(byte_start, file->eof_offset);
    loc.byte_end = std::min(std::max(byte_end, byte_start), file->eof_offset);
    loc.has_byte_range = true;
    return loc;
}

SourceLocation source_end_location(const std::string& source_name) {
    SourceId id = source_id_for_name(source_name);
    return source_location_for_offset(id, source_eof_offset(id));
}

std::string render_source_snippet(const SourceLocation& loc) {
    if (!has_source_name(loc)) return "";
    SourceLocation normalized = loc;
    const SourceFile* file = source_file_for_location(normalized);
    if (file == nullptr) return "";
    if (!has_byte_span(normalized) || normalized.byte_start > file->eof_offset) return "";

    std::size_t span_start = normalized.byte_start;
    std::size_t span_end = std::max(normalized.byte_end, normalized.byte_start);
    span_end = std::min(span_end, file->eof_offset);
    std::size_t line_start = line_start_for_offset(*file, span_start);
    std::size_t line_end = line_end_for_start(*file, line_start);
    std::string line_text = file->text.substr(line_start, line_end - line_start);

    std::size_t marker_start = std::min(span_start, line_end) - line_start;
    std::size_t marker_end = std::min(std::max(span_end, span_start + 1), line_end);
    std::size_t marker_len = marker_end > span_start ? marker_end - span_start : 1;

    std::string line_number = std::to_string(normalized.line);
    std::ostringstream out;
    out << "  --> " << normalized.source_name << ":" << normalized.line << ":" << normalized.column << "\n"
        << "   |\n"
        << " " << line_number << " | " << line_text << "\n"
        << "   | ";
    for (std::size_t i = 0; i < marker_start; ++i) {
        out << (line_text[i] == '\t' ? '\t' : ' ');
    }
    for (std::size_t i = 0; i < marker_len; ++i) out << '^';
    return out.str();
}

} // namespace ari
