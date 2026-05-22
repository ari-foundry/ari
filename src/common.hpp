#pragma once

#include <cstddef>
#include <exception>
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

struct SourceLocation {
    SourceId source_id;
    int line = 1;
    int column = 1;
    std::size_t byte_start = 0;
    std::size_t byte_end = 0;
    bool has_byte_range = false;
    std::string source_name;
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
SourceLocation source_location_for_span(SourceId id, std::size_t byte_start, std::size_t byte_end);
SourceLocation source_end_location(const std::string& source_name);
std::string render_source_snippet(const SourceLocation& loc);

} // namespace ari
