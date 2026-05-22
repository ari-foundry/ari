#include "common.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

int failures = 0;

void fail(const std::string& message) {
    std::cerr << "source-map unit failure: " << message << "\n";
    ++failures;
}

template <typename T, typename U>
void expect_eq(const T& actual, const U& expected, const std::string& label) {
    if (!(actual == expected)) {
        std::cerr << "source-map unit failure: " << label
                  << " expected=" << expected
                  << " actual=" << actual << "\n";
        ++failures;
    }
}

void expect_true(bool value, const std::string& label) {
    if (!value) fail(label);
}

void expect_false(bool value, const std::string& label) {
    if (value) fail(label);
}

void expect_offset(const std::optional<std::size_t>& actual,
                   std::size_t expected,
                   const std::string& label) {
    if (!actual) {
        fail(label + " missing byte offset");
        return;
    }
    expect_eq(*actual, expected, label);
}

void expect_no_offset(const std::optional<std::size_t>& actual, const std::string& label) {
    if (actual) fail(label + " unexpectedly had byte offset");
}

ari::SourceId add_file(ari::SourceMap& map, const std::string& name, const std::string& text) {
    return map.add_source(name, name, text, ari::SourceKind::File);
}

void test_empty_file() {
    ari::SourceMap map;
    ari::SourceId id = add_file(map, "empty.ari", "");
    const ari::SourceFile* file = map.get(id);
    expect_true(file != nullptr, "empty file registered");
    if (file == nullptr) return;
    expect_eq(id.value, 0, "empty file source id");
    expect_eq(file->line_starts.size(), static_cast<std::size_t>(1), "empty file line table size");
    expect_eq(file->line_starts[0], static_cast<std::size_t>(0), "empty file first line start");
    expect_eq(file->eof_offset, static_cast<std::size_t>(0), "empty file eof");

    ari::LineColumn loc = map.location(id, 0);
    expect_eq(loc.line, 1, "empty file line");
    expect_eq(loc.column, 1, "empty file column");
    expect_offset(map.byte_offset(id, 1, 1), 0, "empty file line/column to byte");

    ari::SourceSnippet snippet = map.snippet(map.span(id, 0, 0));
    expect_true(snippet.valid, "empty file snippet valid");
    expect_eq(snippet.lines.size(), static_cast<std::size_t>(1), "empty file snippet lines");
    if (!snippet.lines.empty()) {
        expect_eq(snippet.lines[0].line_number, static_cast<std::size_t>(1), "empty file snippet line number");
        expect_eq(snippet.lines[0].text, std::string(""), "empty file snippet text");
        expect_eq(snippet.lines[0].marker_start, static_cast<std::size_t>(0), "empty file marker start");
        expect_eq(snippet.lines[0].marker_len, static_cast<std::size_t>(1), "empty file marker len");
    }
}

void test_one_line_file() {
    ari::SourceMap map;
    ari::SourceId id = add_file(map, "one.ari", "abc");
    expect_eq(map.location(id, 0).line, 1, "one-line start line");
    expect_eq(map.location(id, 0).column, 1, "one-line start column");
    expect_eq(map.location(id, 2).column, 3, "one-line middle column");
    expect_eq(map.location(id, 3).column, 4, "one-line eof column");
    expect_offset(map.byte_offset(id, 1, 4), 3, "one-line eof byte");
    expect_no_offset(map.byte_offset(id, 1, 5), "one-line out-of-range column");
}

void test_multi_line_file() {
    ari::SourceMap map;
    ari::SourceId id = add_file(map, "multi.ari", "aa\nbbb\nc");
    const ari::SourceFile* file = map.get(id);
    expect_true(file != nullptr, "multi-line file registered");
    if (file == nullptr) return;
    expect_eq(file->line_starts.size(), static_cast<std::size_t>(3), "multi-line line count");
    expect_eq(file->line_starts[0], static_cast<std::size_t>(0), "multi-line line 1 start");
    expect_eq(file->line_starts[1], static_cast<std::size_t>(3), "multi-line line 2 start");
    expect_eq(file->line_starts[2], static_cast<std::size_t>(7), "multi-line line 3 start");
    expect_eq(map.location(id, 3).line, 2, "multi-line second line");
    expect_eq(map.location(id, 3).column, 1, "multi-line second line column");
    expect_eq(map.location(id, 7).line, 3, "multi-line third line");
    expect_offset(map.byte_offset(id, 3, 1), 7, "multi-line line/column to byte");
}

void test_eof_span() {
    ari::SourceMap map;
    ari::SourceId id = add_file(map, "eof.ari", "abc\n");
    ari::SourceLocation loc = map.location_for_span(map.span(id, 4, 4));
    expect_eq(loc.line, 2, "eof span line after final newline");
    expect_eq(loc.column, 1, "eof span column after final newline");
    expect_eq(loc.byte_start, static_cast<std::size_t>(4), "eof span byte start");
    expect_eq(loc.byte_end, static_cast<std::size_t>(4), "eof span byte end");

    ari::SourceSnippet snippet = map.snippet(map.span(id, 4, 4));
    expect_true(snippet.valid, "eof snippet valid");
    expect_eq(snippet.lines.size(), static_cast<std::size_t>(1), "eof snippet line count");
    if (!snippet.lines.empty()) {
        expect_eq(snippet.lines[0].line_number, static_cast<std::size_t>(2), "eof snippet line number");
        expect_eq(snippet.lines[0].marker_len, static_cast<std::size_t>(1), "eof marker len");
    }
}

void test_crlf_file() {
    ari::SourceMap map;
    ari::SourceId id = add_file(map, "crlf.ari", "a\r\nbb\r\nc");
    const ari::SourceFile* file = map.get(id);
    expect_true(file != nullptr, "crlf file registered");
    if (file == nullptr) return;
    expect_eq(file->line_starts.size(), static_cast<std::size_t>(3), "crlf line count");
    expect_eq(file->line_starts[1], static_cast<std::size_t>(3), "crlf second line start");
    expect_eq(file->line_starts[2], static_cast<std::size_t>(7), "crlf third line start");
    expect_eq(map.location(id, 1).column, 2, "crlf column before carriage return");
    expect_eq(map.location(id, 2).column, 2, "crlf newline byte clamps to logical line end");
    expect_eq(map.location(id, 3).line, 2, "crlf second line lookup");
    expect_eq(map.location(id, 3).column, 1, "crlf second line column");
    expect_offset(map.byte_offset(id, 1, 2), 1, "crlf line end byte");
    expect_no_offset(map.byte_offset(id, 1, 3), "crlf hidden carriage return column");
}

void test_utf8_byte_columns_and_multibyte_text() {
    ari::SourceMap map;
    std::string text = std::string("a") + "\xED\x95\x9C" + "b\n";
    ari::SourceId id = add_file(map, "utf8.ari", text);
    expect_eq(map.location(id, 1).column, 2, "utf8 scalar first byte column");
    expect_eq(map.location(id, 4).column, 5, "utf8 scalar advances by byte length");
    expect_eq(map.location(id, 5).column, 6, "utf8 line end byte column");
    expect_offset(map.byte_offset(id, 1, 5), 4, "utf8 byte column reverse lookup");

    ari::SourceSnippet snippet = map.snippet(map.span(id, 1, 4));
    expect_true(snippet.valid, "utf8 snippet valid");
    expect_eq(snippet.lines.size(), static_cast<std::size_t>(1), "utf8 snippet line count");
    if (!snippet.lines.empty()) {
        expect_eq(snippet.lines[0].marker_start, static_cast<std::size_t>(1), "utf8 marker start");
        expect_eq(snippet.lines[0].marker_len, static_cast<std::size_t>(3), "utf8 marker byte len");
    }
}

void test_invalid_span() {
    ari::SourceMap map;
    ari::SourceId id = add_file(map, "invalid.ari", "abcd");
    expect_false(map.valid_span(ari::Span{id, 3, 1}), "reversed span is invalid");
    expect_false(map.valid_span(ari::Span{id, 0, 99}), "out-of-range span is invalid");
    expect_false(ari::span_has_source(ari::invalid_span()), "invalid span has no source");

    ari::Span clamped = map.span(id, 2, 99);
    expect_true(map.valid_span(clamped), "SourceMap span helper clamps to EOF");
    expect_eq(clamped.start, static_cast<std::size_t>(2), "clamped span start");
    expect_eq(clamped.end, static_cast<std::size_t>(4), "clamped span end");
}

void test_multi_file_source_map() {
    ari::SourceMap map;
    ari::SourceId first = add_file(map, "first.ari", "a\n");
    ari::SourceId second = add_file(map, "second.ari", "bb\n");
    expect_eq(first.value, 0, "first source id");
    expect_eq(second.value, 1, "second source id");
    expect_eq(map.get(first)->display_name, std::string("first.ari"), "first display name");
    expect_eq(map.get(second)->display_name, std::string("second.ari"), "second display name");
    expect_eq(map.location(first, 1).column, 2, "first source column");
    expect_eq(map.location(second, 1).column, 2, "second source column");
    expect_eq(map.span(first, 0, 1).source_id.value, first.value, "first span source id");
    expect_eq(map.span(second, 0, 1).source_id.value, second.value, "second span source id");
}

void test_snippet_single_line() {
    ari::SourceMap map;
    ari::SourceId id = add_file(map, "snippet-one.ari", "let value = 42;\n");
    ari::SourceSnippet snippet = map.snippet(map.span(id, 4, 9));
    expect_true(snippet.valid, "single-line snippet valid");
    expect_eq(snippet.lines.size(), static_cast<std::size_t>(1), "single-line snippet line count");
    if (!snippet.lines.empty()) {
        expect_eq(snippet.lines[0].text, std::string("let value = 42;"), "single-line snippet text");
        expect_eq(snippet.lines[0].marker_start, static_cast<std::size_t>(4), "single-line marker start");
        expect_eq(snippet.lines[0].marker_len, static_cast<std::size_t>(5), "single-line marker len");
    }
    std::string rendered = ari::render_source_snippet(snippet);
    expect_true(rendered.find("^^^^^") != std::string::npos, "single-line rendered marker");
}

void test_snippet_multi_line() {
    ari::SourceMap map;
    ari::SourceId id = add_file(map, "snippet-multi.ari", "aa\nbbbb\ncc\n");
    ari::SourceSnippet snippet = map.snippet(map.span(id, 1, 9));
    expect_true(snippet.valid, "multi-line snippet valid");
    expect_eq(snippet.lines.size(), static_cast<std::size_t>(3), "multi-line snippet line count");
    if (snippet.lines.size() == 3) {
        expect_eq(snippet.lines[0].line_number, static_cast<std::size_t>(1), "multi-line first line number");
        expect_eq(snippet.lines[0].marker_start, static_cast<std::size_t>(1), "multi-line first marker start");
        expect_eq(snippet.lines[0].marker_len, static_cast<std::size_t>(1), "multi-line first marker len");
        expect_eq(snippet.lines[1].marker_start, static_cast<std::size_t>(0), "multi-line second marker start");
        expect_eq(snippet.lines[1].marker_len, static_cast<std::size_t>(4), "multi-line second marker len");
        expect_eq(snippet.lines[2].marker_start, static_cast<std::size_t>(0), "multi-line third marker start");
        expect_eq(snippet.lines[2].marker_len, static_cast<std::size_t>(1), "multi-line third marker len");
    }
}

} // namespace

int main() {
    test_empty_file();
    test_one_line_file();
    test_multi_line_file();
    test_eof_span();
    test_crlf_file();
    test_utf8_byte_columns_and_multibyte_text();
    test_invalid_span();
    test_multi_file_source_map();
    test_snippet_single_line();
    test_snippet_multi_line();

    if (failures != 0) return EXIT_FAILURE;
    std::cout << "source-map unit ok\n";
    return EXIT_SUCCESS;
}
