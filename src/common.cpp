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

int display_column_for_offset(const SourceFile& file, std::size_t line_index, std::size_t offset) {
    offset = std::min(offset, file.eof_offset);
    std::size_t line_start = file.line_starts[line_index];
    std::size_t line_end = line_end_for_start(file, line_start);
    std::size_t display_offset = std::min(offset, line_end);
    return static_cast<int>(display_offset - line_start + 1);
}

std::optional<std::size_t> byte_offset_for_line_column(const SourceFile& file, int line, int column) {
    if (line < 1 || column < 1) return std::nullopt;
    std::size_t line_index = static_cast<std::size_t>(line - 1);
    if (line_index >= file.line_starts.size()) return std::nullopt;
    std::size_t line_start = file.line_starts[line_index];
    std::size_t line_end = line_end_for_start(file, line_start);
    std::size_t column_index = static_cast<std::size_t>(column - 1);
    if (column_index > line_end - line_start) return std::nullopt;
    return line_start + column_index;
}

void fill_source_location_from_file(SourceLocation& loc, const SourceFile& file, std::size_t byte_offset) {
    byte_offset = std::min(byte_offset, file.eof_offset);
    std::size_t line_index = line_index_for_offset(file, byte_offset);
    loc.span = Span{file.id, byte_offset, byte_offset};
    loc.source_id = file.id;
    loc.source_name = file.display_name;
    loc.line = static_cast<int>(line_index + 1);
    loc.column = display_column_for_offset(file, line_index, byte_offset);
    loc.byte_start = byte_offset;
    loc.byte_end = byte_offset;
    loc.has_byte_range = true;
}

const SourceFile* source_file_for_location(SourceLocation& loc) {
    if (!default_source_map().valid(loc.source_id) && !loc.source_name.empty()) {
        loc.source_id = default_source_map().id_for_name(loc.source_name);
    }
    const SourceFile* file = default_source_map().get(loc.source_id);
    if (file != nullptr && loc.source_name.empty()) {
        loc.source_name = file->display_name;
    }
    if (file != nullptr && loc.has_byte_range) {
        Span derived = source_span(loc.source_id, loc.byte_start, loc.byte_end);
        if (!span_has_source(loc.span) ||
            loc.span.source_id.value != derived.source_id.value ||
            loc.span.start != derived.start ||
            loc.span.end != derived.end) {
            loc.span = derived;
        }
    }
    return file;
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
    loc.has_byte_range = true;
    loc.source_name = std::move(source_name);
    loc.source_id = source_id_for_name(loc.source_name);
    set_location_span(loc, source_span(loc.source_id, byte_start, byte_end));
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
        Span span = span_from_location(normalized);
        out << "\n  source id: " << source_id_text(normalized.source_id)
            << "\n  byte span: [" << span.start << ", " << span.end << ")";
    }
    std::string snippet = render_source_snippet(normalized);
    if (!snippet.empty()) out << "\n" << snippet;
    return out.str();
}

} // namespace

SourceMap& default_source_map() {
    static SourceMap map;
    return map;
}

Span invalid_span() {
    return Span{};
}

Span source_span(SourceId id, std::size_t byte_start, std::size_t byte_end) {
    return Span{id, byte_start, std::max(byte_end, byte_start)};
}

bool span_has_source(Span span) {
    return span.source_id.value != kInvalidSourceIdValue;
}

bool span_has_valid_order(Span span) {
    return span.start <= span.end;
}

bool span_is_empty(Span span) {
    return span_has_source(span) && span_has_valid_order(span) && span.start == span.end;
}

std::size_t span_length(Span span) {
    if (!span_has_valid_order(span)) return 0;
    return span.end - span.start;
}

Span span_from_location(const SourceLocation& loc) {
    if (span_has_source(loc.span) && span_has_valid_order(loc.span)) return loc.span;
    if (!loc.has_byte_range) return invalid_span();
    return source_span(loc.source_id, loc.byte_start, loc.byte_end);
}

void set_location_span(SourceLocation& loc, Span span) {
    loc.span = span;
    loc.source_id = span.source_id;
    loc.byte_start = span.start;
    loc.byte_end = span.end;
    loc.has_byte_range = span_has_source(span) && span_has_valid_order(span);
}

SourceLocation source_location_for_span(Span span) {
    return default_source_map().location_for_span(span);
}

bool span_contains(Span span, std::size_t byte_offset) {
    if (!span_has_source(span)) return false;
    if (!span_has_valid_order(span)) return false;
    return span.start <= byte_offset && byte_offset < span.end;
}

bool span_contains(Span outer, Span inner) {
    if (!span_has_source(outer) || !span_has_source(inner)) return false;
    if (outer.source_id.value != inner.source_id.value) return false;
    if (!span_has_valid_order(outer) || !span_has_valid_order(inner)) return false;
    return outer.start <= inner.start && inner.end <= outer.end;
}

bool span_intersects(Span lhs, Span rhs) {
    if (!span_has_source(lhs) || !span_has_source(rhs)) return false;
    if (lhs.source_id.value != rhs.source_id.value) return false;
    if (!span_has_valid_order(lhs) || !span_has_valid_order(rhs)) return false;
    return std::max(lhs.start, rhs.start) < std::min(lhs.end, rhs.end);
}

Span merge_spans(Span lhs, Span rhs) {
    if (!span_has_source(lhs)) return rhs;
    if (!span_has_source(rhs)) return lhs;
    if (lhs.source_id.value != rhs.source_id.value) return invalid_span();
    if (!span_has_valid_order(lhs) || !span_has_valid_order(rhs)) return invalid_span();
    return Span{lhs.source_id, std::min(lhs.start, rhs.start), std::max(lhs.end, rhs.end)};
}

SourceId SourceMap::add_file(const std::string& path, const std::string& text) {
    return add_source(path, path, text, SourceKind::File);
}

SourceId SourceMap::add_source(const std::string& path,
                               const std::string& display_name,
                               const std::string& text,
                               SourceKind kind) {
    std::string source_path = fallback_source_path(path, display_name);
    if (source_path.empty()) return SourceId{};
    std::string source_display = fallback_display_name(source_path, display_name);

    auto found = path_index_.find(source_path);
    if (found != path_index_.end()) {
        SourceId id = found->second;
        SourceFile* file = nullptr;
        if (valid(id)) file = &sources_[static_cast<std::size_t>(id.value)];
        if (file != nullptr) {
            auto display_found = display_index_.find(file->display_name);
            if (display_found != display_index_.end() && display_found->second.value == id.value) {
                display_index_.erase(display_found);
            }
            file->kind = kind;
            file->path = source_path;
            file->display_name = source_display;
            reset_source_file_text(*file, text);
            display_index_[file->display_name] = id;
        }
        return id;
    }

    SourceId id{static_cast<int>(sources_.size())};
    SourceFile file;
    file.id = id;
    file.kind = kind;
    file.path = std::move(source_path);
    file.display_name = std::move(source_display);
    reset_source_file_text(file, text);
    sources_.push_back(std::move(file));
    const SourceFile& stored = sources_.back();
    path_index_.emplace(stored.path, id);
    display_index_[stored.display_name] = id;
    return id;
}

SourceId SourceMap::add_in_memory(const std::string& display_name,
                                  const std::string& text,
                                  SourceKind kind) {
    std::string source_path = "<memory:" + std::to_string(in_memory_count_++) + ">";
    std::string source_display = display_name.empty() ? source_path : display_name;
    return add_source(source_path, source_display, text, kind);
}

bool SourceMap::valid(SourceId id) const {
    return id.value >= 0 && static_cast<std::size_t>(id.value) < sources_.size();
}

const SourceFile* SourceMap::get(SourceId id) const {
    if (!valid(id)) return nullptr;
    return &sources_[static_cast<std::size_t>(id.value)];
}

const std::string* SourceMap::text(SourceId id) const {
    const SourceFile* file = get(id);
    if (file == nullptr) return nullptr;
    return &file->text;
}

const std::string* SourceMap::text(const std::string& source_name) const {
    return text(id_for_name(source_name));
}

SourceId SourceMap::id_for_name(const std::string& source_name) const {
    auto found = path_index_.find(source_name);
    if (found != path_index_.end()) return found->second;
    found = display_index_.find(source_name);
    if (found == display_index_.end()) return SourceId{};
    return found->second;
}

std::size_t SourceMap::eof_offset(SourceId id) const {
    const SourceFile* file = get(id);
    if (file == nullptr) return 0;
    return file->eof_offset;
}

Span SourceMap::span(SourceId id, std::size_t byte_start, std::size_t byte_end) const {
    Span result;
    result.source_id = id;
    const SourceFile* file = get(id);
    if (file == nullptr) {
        result.start = byte_start;
        result.end = std::max(byte_end, byte_start);
        return result;
    }
    result.start = std::min(byte_start, file->eof_offset);
    result.end = std::min(std::max(byte_end, byte_start), file->eof_offset);
    return result;
}

bool SourceMap::valid_span(Span source_span) const {
    const SourceFile* file = get(source_span.source_id);
    if (file == nullptr) return false;
    return span_has_valid_order(source_span) && source_span.end <= file->eof_offset;
}

LineColumn SourceMap::location(SourceId id, std::size_t byte_offset) const {
    LineColumn result;
    const SourceFile* file = get(id);
    if (file == nullptr) return result;
    byte_offset = std::min(byte_offset, file->eof_offset);
    std::size_t line_index = line_index_for_offset(*file, byte_offset);
    result.line = static_cast<int>(line_index + 1);
    result.column = display_column_for_offset(*file, line_index, byte_offset);
    result.byte_offset = byte_offset;
    return result;
}

std::optional<std::size_t> SourceMap::byte_offset(SourceId id, int line, int column) const {
    const SourceFile* file = get(id);
    if (file == nullptr) return std::nullopt;
    return byte_offset_for_line_column(*file, line, column);
}

std::optional<std::size_t> SourceMap::byte_offset(SourceId id, LineColumn location) const {
    return byte_offset(id, location.line, location.column);
}

SourceLocation SourceMap::location_for_offset(SourceId id, std::size_t byte_offset) const {
    SourceLocation loc;
    const SourceFile* file = get(id);
    if (file == nullptr) return loc;
    fill_source_location_from_file(loc, *file, byte_offset);
    return loc;
}

SourceLocation SourceMap::location_for_span(Span source_span) const {
    SourceLocation loc = location_for_offset(source_span.source_id, source_span.start);
    const SourceFile* file = get(source_span.source_id);
    if (file == nullptr) return loc;
    Span normalized = span(source_span.source_id, source_span.start, source_span.end);
    set_location_span(loc, normalized);
    loc.has_byte_range = true;
    return loc;
}

SourceLocation SourceMap::location_for_span(SourceId id, std::size_t byte_start, std::size_t byte_end) const {
    return location_for_span(span(id, byte_start, byte_end));
}

SourceLocation SourceMap::end_location(const std::string& source_name) const {
    SourceId id = id_for_name(source_name);
    return location_for_offset(id, eof_offset(id));
}

SourceSnippet SourceMap::snippet(Span source_span) const {
    SourceSnippet result;
    const SourceFile* file = get(source_span.source_id);
    if (file == nullptr) return result;

    Span normalized = span(source_span.source_id, source_span.start, source_span.end);
    std::size_t line_start = line_start_for_offset(*file, normalized.start);
    std::size_t line_end = line_end_for_start(*file, line_start);
    std::size_t span_end = std::min(normalized.end, file->eof_offset);

    result.span = normalized;
    result.start = location(normalized.source_id, normalized.start);
    result.source_name = file->display_name;
    result.line_text = file->text.substr(line_start, line_end - line_start);
    result.line_number = static_cast<std::size_t>(result.start.line);
    result.marker_start = std::min(normalized.start, line_end) - line_start;
    std::size_t marker_end = std::min(std::max(span_end, normalized.start + 1), line_end);
    result.marker_len = marker_end > normalized.start ? marker_end - normalized.start : 1;
    result.valid = true;
    return result;
}

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
        Span span = span_from_location(normalized);
        text += ":bytes=" + std::to_string(span.start) + ".." + std::to_string(span.end);
    }
    return text;
}

bool valid_source_id(SourceId id) {
    return default_source_map().valid(id);
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
    return loc.has_byte_range && span_has_valid_order(span_from_location(loc));
}

SourceId register_source_file(const std::string& path,
                              const std::string& display_name,
                              const std::string& text,
                              SourceKind kind) {
    return default_source_map().add_source(path, display_name, text, kind);
}

SourceId register_in_memory_source(const std::string& display_name,
                                   const std::string& text,
                                   SourceKind kind) {
    return default_source_map().add_in_memory(display_name, text, kind);
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
    return default_source_map().id_for_name(source_name);
}

const SourceFile* find_source_file(SourceId id) {
    return default_source_map().get(id);
}

const std::string* find_source_text(SourceId id) {
    return default_source_map().text(id);
}

const std::string* find_source_text(const std::string& source_name) {
    return default_source_map().text(source_name);
}

std::size_t source_eof_offset(SourceId id) {
    return default_source_map().eof_offset(id);
}

SourceLocation source_location_for_offset(SourceId id, std::size_t byte_offset) {
    return default_source_map().location_for_offset(id, byte_offset);
}

std::optional<std::size_t> source_byte_offset(SourceId id, int line, int column) {
    return default_source_map().byte_offset(id, line, column);
}

std::optional<std::size_t> source_byte_offset(SourceId id, LineColumn location) {
    return default_source_map().byte_offset(id, location);
}

SourceLocation source_location_for_span(SourceId id, std::size_t byte_start, std::size_t byte_end) {
    return default_source_map().location_for_span(id, byte_start, byte_end);
}

SourceLocation source_end_location(const std::string& source_name) {
    return default_source_map().end_location(source_name);
}

std::string render_source_snippet(const SourceSnippet& snippet) {
    if (!snippet.valid) return "";
    std::string line_number = std::to_string(snippet.line_number);
    std::ostringstream out;
    out << "  --> " << snippet.source_name << ":" << snippet.start.line << ":" << snippet.start.column << "\n"
        << "   |\n"
        << " " << line_number << " | " << snippet.line_text << "\n"
        << "   | ";
    for (std::size_t i = 0; i < snippet.marker_start; ++i) {
        out << (snippet.line_text[i] == '\t' ? '\t' : ' ');
    }
    for (std::size_t i = 0; i < snippet.marker_len; ++i) out << '^';
    return out.str();
}

std::string render_source_snippet(Span span) {
    return render_source_snippet(default_source_map().snippet(span));
}

std::string render_source_snippet(const SourceLocation& loc) {
    if (!has_source_name(loc) || !has_byte_span(loc)) return "";
    SourceLocation normalized = loc;
    const SourceFile* file = source_file_for_location(normalized);
    if (file == nullptr) return "";
    return render_source_snippet(Span{
        normalized.source_id,
        normalized.byte_start,
        normalized.byte_end,
    });
}

} // namespace ari
