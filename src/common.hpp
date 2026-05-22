#pragma once

#include <cstddef>
#include <exception>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace ari {

inline constexpr int kInvalidSourceIdValue = -1;

struct SourceId {
    int value = kInvalidSourceIdValue;
};

enum class SourceKind {
    Unknown,
    File,
    Generated,
    Builtin
};

struct SourceFile {
    SourceId id;
    SourceKind kind = SourceKind::Unknown;
    std::string path;
    std::string display_name;
    std::string text;
    std::vector<std::size_t> line_starts;
    std::size_t eof_offset = 0;
};

struct Span {
    SourceId source_id;
    std::size_t start = 0;
    std::size_t end = 0;
};

using SourceSpan = Span;

struct LineColumn {
    int line = 1;
    int column = 1;
    std::size_t byte_offset = 0;
};

struct SourceSnippet {
    Span span;
    LineColumn start;
    std::string source_name;
    std::string line_text;
    std::size_t line_number = 1;
    std::size_t marker_start = 0;
    std::size_t marker_len = 0;
    bool valid = false;
};

struct SourceLocation {
    Span span;
    SourceId source_id;
    int line = 1;
    int column = 1;
    std::size_t byte_start = 0;
    std::size_t byte_end = 0;
    bool has_byte_range = false;
    std::string source_name;
};

struct DiagnosticLabel {
    Span span;
    std::string message;
    bool primary = false;
};

class SourceMap {
public:
    SourceId add_file(const std::string& path, const std::string& text);
    SourceId add_source(const std::string& path,
                        const std::string& display_name,
                        const std::string& text,
                        SourceKind kind = SourceKind::File);
    SourceId add_in_memory(const std::string& display_name,
                           const std::string& text,
                           SourceKind kind = SourceKind::Generated);

    bool valid(SourceId id) const;
    const SourceFile* get(SourceId id) const;
    const std::string* text(SourceId id) const;
    const std::string* text(const std::string& source_name) const;
    SourceId id_for_name(const std::string& source_name) const;
    std::size_t eof_offset(SourceId id) const;

    Span span(SourceId id, std::size_t byte_start, std::size_t byte_end) const;
    bool valid_span(Span span) const;
    LineColumn location(SourceId id, std::size_t byte_offset) const;
    std::optional<std::size_t> byte_offset(SourceId id, int line, int column) const;
    std::optional<std::size_t> byte_offset(SourceId id, LineColumn location) const;
    SourceLocation location_for_offset(SourceId id, std::size_t byte_offset) const;
    SourceLocation location_for_span(Span span) const;
    SourceLocation location_for_span(SourceId id, std::size_t byte_start, std::size_t byte_end) const;
    SourceLocation end_location(const std::string& source_name) const;
    SourceSnippet snippet(Span span) const;

private:
    std::vector<SourceFile> sources_;
    std::map<std::string, SourceId> path_index_;
    std::map<std::string, SourceId> display_index_;
    std::size_t in_memory_count_ = 0;
};

struct CompileError : std::exception {
    explicit CompileError(std::string message);
    CompileError(SourceLocation loc, std::string message);

    const char* what() const noexcept override;
    const std::string& message() const;
    bool has_location() const;
    const SourceLocation& location() const;

private:
    std::string rendered_;
    std::string message_;
    SourceLocation loc_;
    bool has_location_ = false;
};

SourceMap& default_source_map();

Span invalid_span();
Span source_span(SourceId id, std::size_t byte_start, std::size_t byte_end);
Span span_from_location(const SourceLocation& loc);
void set_location_span(SourceLocation& loc, Span span);
SourceLocation source_location_for_span(Span span);
bool span_has_source(Span span);
bool span_has_valid_order(Span span);
bool span_is_empty(Span span);
std::size_t span_length(Span span);
bool span_contains(Span span, std::size_t byte_offset);
bool span_contains(Span outer, Span inner);
bool span_intersects(Span lhs, Span rhs);
Span merge_spans(Span lhs, Span rhs);

std::string where(const SourceLocation& loc);
bool valid_source_id(SourceId id);
std::string source_id_text(SourceId id);
std::string source_kind_text(SourceKind kind);
bool has_source_name(const SourceLocation& loc);
bool has_byte_span(const SourceLocation& loc);
SourceId register_source_file(const std::string& path,
                              const std::string& display_name,
                              const std::string& text,
                              SourceKind kind = SourceKind::File);
SourceId register_in_memory_source(const std::string& display_name,
                                   const std::string& text,
                                   SourceKind kind = SourceKind::Generated);
SourceId register_source_text(const std::string& source_name,
                              const std::string& text,
                              SourceKind kind = SourceKind::File);
SourceId source_id_for_name(const std::string& source_name);
const SourceFile* find_source_file(SourceId id);
const std::string* find_source_text(SourceId id);
const std::string* find_source_text(const std::string& source_name);
std::size_t source_eof_offset(SourceId id);
SourceLocation source_location_for_offset(SourceId id, std::size_t byte_offset);
std::optional<std::size_t> source_byte_offset(SourceId id, int line, int column);
std::optional<std::size_t> source_byte_offset(SourceId id, LineColumn location);
SourceLocation source_location_for_span(SourceId id, std::size_t byte_start, std::size_t byte_end);
SourceLocation source_end_location(const std::string& source_name);
std::string render_source_snippet(const SourceSnippet& snippet);
std::string render_source_snippet(Span span);
std::string render_source_snippet(const SourceLocation& loc);

} // namespace ari
