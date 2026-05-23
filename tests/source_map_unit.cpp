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

void test_span_helper_invariants() {
    ari::SourceMap map;
    ari::SourceId first = add_file(map, "span-a.ari", "abcd");
    ari::SourceId second = add_file(map, "span-b.ari", "wxyz");
    ari::Span left = map.span(first, 1, 3);
    ari::Span right = map.span(first, 2, 4);
    ari::Span disjoint = map.span(first, 3, 4);
    ari::Span other_source = map.span(second, 1, 2);

    expect_true(ari::span_contains(left, static_cast<std::size_t>(1)), "span contains start");
    expect_false(ari::span_contains(left, static_cast<std::size_t>(3)), "span excludes end");
    expect_true(ari::span_contains(map.span(first, 0, 4), left), "outer span contains inner span");
    expect_false(ari::span_contains(left, other_source), "span containment rejects different source");
    expect_true(ari::span_intersects(left, right), "overlapping spans intersect");
    expect_false(ari::span_intersects(left, disjoint), "half-open adjacent spans do not intersect");

    ari::Span merged = ari::merge_spans(left, right);
    expect_eq(merged.source_id.value, first.value, "merged span source id");
    expect_eq(merged.start, static_cast<std::size_t>(1), "merged span start");
    expect_eq(merged.end, static_cast<std::size_t>(4), "merged span end");
    expect_false(ari::span_has_source(ari::merge_spans(left, other_source)),
                 "merge rejects spans from different sources");
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

void test_source_registration_identity() {
    ari::SourceMap map;
    ari::SourceId first = map.add_source("same.ari", "old-display.ari", "a\n", ari::SourceKind::File);
    ari::SourceId generated = map.add_in_memory("generated.ari", "bb", ari::SourceKind::Generated);
    ari::SourceId second = map.add_source("same.ari", "new-display.ari", "ccc\n", ari::SourceKind::File);

    expect_eq(first.value, 0, "first registered source id");
    expect_eq(generated.value, 1, "generated source id follows file source");
    expect_eq(second.value, first.value, "duplicate path keeps source id");
    expect_eq(map.eof_offset(first), static_cast<std::size_t>(4), "duplicate path updates source text");
    expect_eq(map.id_for_name("same.ari").value, first.value, "canonical path resolves to stable id");
    expect_eq(map.id_for_name("old-display.ari").value,
              ari::kInvalidSourceIdValue,
              "old display name removed after replacement");
    expect_eq(map.id_for_name("new-display.ari").value, first.value, "new display name resolves to stable id");

    const ari::SourceFile* generated_file = map.get(generated);
    expect_true(generated_file != nullptr, "generated source registered");
    if (generated_file != nullptr) {
        expect_eq(ari::source_kind_text(generated_file->kind), std::string("generated"), "generated source kind");
        expect_eq(generated_file->display_name, std::string("generated.ari"), "generated display name");
    }
}

void test_missing_source_snippet_fallback() {
    ari::SourceMap map;
    ari::SourceSnippet snippet = map.snippet(ari::Span{ari::SourceId{99}, 0, 1});
    expect_false(snippet.valid, "missing source snippet invalid");
    expect_eq(snippet.lines.size(), static_cast<std::size_t>(0), "missing source snippet has no lines");
    expect_eq(ari::render_source_snippet(snippet), std::string(""), "missing source snippet renders empty");
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

void test_snippet_context_lines() {
    ari::SourceMap map;
    ari::SourceId id = add_file(map, "snippet-context.ari", "one\ntwo\nthree\n");
    ari::SourceSnippet snippet = map.snippet(map.span(id, 4, 7), 1);
    expect_true(snippet.valid, "context snippet valid");
    expect_eq(snippet.context_lines, static_cast<std::size_t>(1), "context snippet records context");
    expect_eq(snippet.lines.size(), static_cast<std::size_t>(3), "context snippet line count");
    if (snippet.lines.size() == 3) {
        expect_eq(snippet.lines[0].line_number, static_cast<std::size_t>(1), "context first line number");
        expect_false(snippet.lines[0].has_marker, "context first line unmarked");
        expect_eq(snippet.lines[1].line_number, static_cast<std::size_t>(2), "context marked line number");
        expect_true(snippet.lines[1].has_marker, "context middle line marked");
        expect_eq(snippet.lines[1].marker_start, static_cast<std::size_t>(0), "context marker start");
        expect_eq(snippet.lines[1].marker_len, static_cast<std::size_t>(3), "context marker len");
        expect_eq(snippet.lines[2].line_number, static_cast<std::size_t>(3), "context trailing line number");
        expect_false(snippet.lines[2].has_marker, "context trailing line unmarked");
    }
}

void test_snippet_long_line_truncation() {
    ari::SourceMap map;
    std::string text(150, 'a');
    text[130] = 'b';
    ari::SourceId id = add_file(map, "snippet-long.ari", text);
    ari::SourceSnippet snippet = map.snippet(map.span(id, 130, 131));
    expect_true(snippet.valid, "long snippet valid");
    expect_true(snippet.truncated, "long snippet truncated");
    expect_eq(snippet.lines.size(), static_cast<std::size_t>(1), "long snippet line count");
    if (!snippet.lines.empty()) {
        expect_true(snippet.lines[0].text.size() <= static_cast<std::size_t>(120), "long snippet capped");
        expect_true(snippet.lines[0].truncated_start, "long snippet truncates start");
        expect_true(snippet.lines[0].has_marker, "long snippet marker present");
        expect_eq(snippet.lines[0].marker_len, static_cast<std::size_t>(1), "long snippet marker len");
    }
    std::string rendered = ari::render_source_snippet(snippet);
    expect_true(rendered.find("...") != std::string::npos, "long snippet rendered ellipsis");
}

void test_invalid_line_column_lookup() {
    ari::SourceMap map;
    ari::SourceId id = add_file(map, "invalid-line-column.ari", "abc\n");
    expect_no_offset(map.byte_offset(id, 0, 1), "zero line rejected");
    expect_no_offset(map.byte_offset(id, 1, 0), "zero column rejected");
    expect_no_offset(map.byte_offset(id, 3, 1), "past-end line rejected");
    expect_no_offset(map.byte_offset(ari::SourceId{99}, 1, 1), "unknown source rejected");
}

void test_compile_error_source_location_bridge() {
    ari::SourceId id = ari::register_in_memory_source(
        "compile-error-source.ari",
        "let x = false;\n",
        ari::SourceKind::Generated);
    ari::SourceLocation loc = ari::source_location_for_span(id, 8, 13);
    ari::CompileError error(loc, "type mismatch: expected i64, got bool");
    expect_true(error.has_location(), "compile error has source location");
    expect_eq(error.message(), std::string("type mismatch: expected i64, got bool"), "compile error message");
    expect_eq(error.location().source_id.value, id.value, "compile error source id");
    expect_eq(error.location().line, 1, "compile error line");
    expect_eq(error.location().column, 9, "compile error column");
    expect_eq(error.labels().size(), static_cast<std::size_t>(1), "compile error primary label count");
    if (!error.labels().empty()) {
        expect_true(error.labels()[0].primary, "compile error primary label");
        expect_eq(error.labels()[0].span.start, static_cast<std::size_t>(8), "compile error label start");
        expect_eq(error.labels()[0].span.end, static_cast<std::size_t>(13), "compile error label end");
    }
    std::string rendered(error.what());
    expect_true(rendered.find("compile-error-source.ari:1:9") != std::string::npos,
                "compile error rendered location");
    expect_true(rendered.find("byte span: [8, 13)") != std::string::npos,
                "compile error rendered byte span");
    expect_true(rendered.find("^^^^^") != std::string::npos, "compile error rendered snippet");
}

void test_compile_error_legacy_location_bridge() {
    ari::SourceId id = ari::register_in_memory_source(
        "compile-error-legacy.ari",
        "abc\n",
        ari::SourceKind::Generated);
    ari::SourceLocation loc = ari::source_location_for_span(id, 1, 3);
    ari::CompileError error(ari::where(loc) + ": legacy diagnostic");
    expect_true(error.has_location(), "legacy compile error has source location");
    expect_eq(error.message(), std::string("legacy diagnostic"), "legacy compile error message");
    expect_eq(error.location().source_id.value, id.value, "legacy compile error source id");
    expect_eq(error.location().line, 1, "legacy compile error line");
    expect_eq(error.location().column, 2, "legacy compile error column");
    expect_eq(error.labels().size(), static_cast<std::size_t>(1), "legacy compile error label count");
    if (!error.labels().empty()) {
        expect_eq(error.labels()[0].span.start, static_cast<std::size_t>(1), "legacy label start");
        expect_eq(error.labels()[0].span.end, static_cast<std::size_t>(3), "legacy label end");
    }
    std::string rendered(error.what());
    expect_true(rendered.find("compile-error-legacy.ari:1:2") != std::string::npos,
                "legacy compile error rendered location");
    expect_true(rendered.find("byte span: [1, 3)") != std::string::npos,
                "legacy compile error rendered byte span");
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
    test_span_helper_invariants();
    test_multi_file_source_map();
    test_source_registration_identity();
    test_missing_source_snippet_fallback();
    test_snippet_single_line();
    test_snippet_multi_line();
    test_snippet_context_lines();
    test_snippet_long_line_truncation();
    test_invalid_line_column_lookup();
    test_compile_error_source_location_bridge();
    test_compile_error_legacy_location_bridge();

    if (failures != 0) return EXIT_FAILURE;
    std::cout << "source-map unit ok\n";
    return EXIT_SUCCESS;
}
