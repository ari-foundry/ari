#include "lexer.hpp"

#include "literal.hpp"

#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace ari {

class Lexer {
public:
    explicit Lexer(std::string source, std::string source_name = {})
        : source_(std::move(source)), source_name_(std::move(source_name)) {
        source_id_ = register_source_text(source_name_, source_);
        if (const SourceFile* file = find_source_file(source_id_)) {
            source_name_ = file->display_name;
        }
    }

    std::vector<Token> lex() {
        std::vector<Token> tokens;
        for (;;) {
            skip_space_and_comments();
            SourceLocation loc = current_location();
            char c = peek();
            if (c == '\0') {
                loc.byte_end = loc.byte_start;
                tokens.push_back(Token{TokenKind::End, "", 0, loc, 0.0, ""});
                return tokens;
            }
            if (is_alpha(c) || c == '_') {
                tokens.push_back(identifier(loc));
                continue;
            }
            if (is_digit(c)) {
                tokens.push_back(number(loc));
                continue;
            }
            advance();
            switch (c) {
                case '(': tokens.push_back(simple(TokenKind::LParen, "(", loc)); break;
                case ')': tokens.push_back(simple(TokenKind::RParen, ")", loc)); break;
                case '{': tokens.push_back(simple(TokenKind::LBrace, "{", loc)); break;
                case '}': tokens.push_back(simple(TokenKind::RBrace, "}", loc)); break;
                case '[': tokens.push_back(simple(TokenKind::LBracket, "[", loc)); break;
                case ']': tokens.push_back(simple(TokenKind::RBracket, "]", loc)); break;
                case ',': tokens.push_back(simple(TokenKind::Comma, ",", loc)); break;
                case '.':
                    if (match('.')) {
                        if (match('=')) tokens.push_back(simple(TokenKind::DotDotEqual, "..=", loc));
                        else if (match('.')) tokens.push_back(simple(TokenKind::Ellipsis, "...", loc));
                        else tokens.push_back(simple(TokenKind::DotDot, "..", loc));
                    } else {
                        tokens.push_back(simple(TokenKind::Dot, ".", loc));
                    }
                    break;
                case ':':
                    if (match(':')) tokens.push_back(simple(TokenKind::ColonColon, "::", loc));
                    else tokens.push_back(simple(TokenKind::Colon, ":", loc));
                    break;
                case ';': tokens.push_back(simple(TokenKind::Semicolon, ";", loc)); break;
                case '@': tokens.push_back(simple(TokenKind::At, "@", loc)); break;
                case '?':
                    if (match('?')) tokens.push_back(simple(TokenKind::QuestionQuestion, "??", loc));
                    else tokens.push_back(simple(TokenKind::Question, "?", loc));
                    break;
                case '+':
                    if (match('=')) tokens.push_back(simple(TokenKind::PlusEqual, "+=", loc));
                    else tokens.push_back(simple(TokenKind::Plus, "+", loc));
                    break;
                case '*':
                    if (match('=')) tokens.push_back(simple(TokenKind::StarEqual, "*=", loc));
                    else tokens.push_back(simple(TokenKind::Star, "*", loc));
                    break;
                case '%':
                    if (match('=')) tokens.push_back(simple(TokenKind::PercentEqual, "%=", loc));
                    else tokens.push_back(simple(TokenKind::Percent, "%", loc));
                    break;
                case '&':
                    if (match('&')) tokens.push_back(simple(TokenKind::AmpAmp, "&&", loc));
                    else if (match('=')) tokens.push_back(simple(TokenKind::AmpEqual, "&=", loc));
                    else tokens.push_back(simple(TokenKind::Amp, "&", loc));
                    break;
                case '|':
                    if (match('|')) tokens.push_back(simple(TokenKind::PipePipe, "||", loc));
                    else if (match('=')) tokens.push_back(simple(TokenKind::PipeEqual, "|=", loc));
                    else tokens.push_back(simple(TokenKind::Pipe, "|", loc));
                    break;
                case '^':
                    if (match('=')) tokens.push_back(simple(TokenKind::CaretEqual, "^=", loc));
                    else tokens.push_back(simple(TokenKind::Caret, "^", loc));
                    break;
                case '/':
                    if (match('=')) tokens.push_back(simple(TokenKind::SlashEqual, "/=", loc));
                    else tokens.push_back(simple(TokenKind::Slash, "/", loc));
                    break;
                case '"': tokens.push_back(string(loc)); break;
                case '\'': tokens.push_back(byte_char(loc)); break;
                case '-':
                    if (match('>')) tokens.push_back(simple(TokenKind::Arrow, "->", loc));
                    else if (match('=')) tokens.push_back(simple(TokenKind::MinusEqual, "-=", loc));
                    else tokens.push_back(simple(TokenKind::Minus, "-", loc));
                    break;
                case '=':
                    if (match('>')) tokens.push_back(simple(TokenKind::FatArrow, "=>", loc));
                    else if (match('=')) tokens.push_back(simple(TokenKind::EqEq, "==", loc));
                    else tokens.push_back(simple(TokenKind::Equal, "=", loc));
                    break;
                case '!':
                    if (match('=')) tokens.push_back(simple(TokenKind::BangEq, "!=", loc));
                    else tokens.push_back(simple(TokenKind::Bang, "!", loc));
                    break;
                case '~': tokens.push_back(simple(TokenKind::Tilde, "~", loc)); break;
                case '<':
                    if (match('<')) {
                        if (match('=')) tokens.push_back(simple(TokenKind::LessLessEqual, "<<=", loc));
                        else tokens.push_back(simple(TokenKind::LessLess, "<<", loc));
                    }
                    else if (match('=')) tokens.push_back(simple(TokenKind::LessEq, "<=", loc));
                    else tokens.push_back(simple(TokenKind::Less, "<", loc));
                    break;
                case '>':
                    if (match('>')) {
                        if (match('=')) tokens.push_back(simple(TokenKind::GreaterGreaterEqual, ">>=", loc));
                        else tokens.push_back(simple(TokenKind::GreaterGreater, ">>", loc));
                    }
                    else if (match('=')) tokens.push_back(simple(TokenKind::GreaterEq, ">=", loc));
                    else tokens.push_back(simple(TokenKind::Greater, ">", loc));
                    break;
                default:
                    fail(loc, unexpected_character_message(c));
            }
        }
    }

private:
    std::string source_;
    std::string source_name_;
    SourceId source_id_;
    std::size_t index_ = 0;
    int line_ = 1;
    int column_ = 1;

    static bool is_alpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    static bool is_digit(char c) {
        return c >= '0' && c <= '9';
    }

    static std::string byte_hex(unsigned char byte) {
        std::ostringstream out;
        out << "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(byte);
        return out.str();
    }

    static std::string unexpected_character_message(char c) {
        unsigned char byte = static_cast<unsigned char>(c);
        if (byte >= 0x20 && byte <= 0x7e) {
            return std::string("unexpected character '") + c + "'";
        }
        return "unexpected byte " + byte_hex(byte);
    }

    static bool is_unexpected_input_message(const std::string& message) {
        return message.rfind("unexpected character", 0) == 0 ||
               message.rfind("unexpected byte", 0) == 0;
    }

    static bool is_unterminated_string_message(const std::string& message) {
        return message == "unterminated string literal";
    }

    static bool is_unterminated_block_comment_message(const std::string& message) {
        return message == "unterminated block comment";
    }

    static bool is_unterminated_unicode_escape_message(const std::string& message) {
        return message == "unterminated Unicode escape";
    }

    static bool is_suffix_char(char c) {
        return is_alpha(c) || is_digit(c) || c == '_';
    }

    static bool is_integer_suffix(const std::string& suffix) {
        return suffix == "i8" || suffix == "i16" || suffix == "i32" || suffix == "i64" ||
               suffix == "u8" || suffix == "u16" || suffix == "u32" || suffix == "u64";
    }

    static bool is_float_suffix(const std::string& suffix) {
        return suffix == "f32" || suffix == "f64" || suffix == "f128";
    }

    static bool is_octal_digit(char c) {
        return c >= '0' && c <= '7';
    }

    char peek(int offset = 0) const {
        std::size_t at = index_ + static_cast<std::size_t>(offset);
        if (at >= source_.size()) return '\0';
        return source_[at];
    }

    SourceLocation current_location() const {
        SourceLocation loc;
        loc.source_id = source_id_;
        loc.line = line_;
        loc.column = column_;
        set_location_span(loc, source_span(source_id_, index_, index_));
        loc.source_name = source_name_;
        return loc;
    }

    void finish_location(SourceLocation& loc, std::size_t byte_end) const {
        set_location_span(loc, default_source_map().span(loc.source_id, loc.byte_start, byte_end));
    }

    char advance() {
        char c = peek();
        if (c == '\0') return c;
        ++index_;
        if (c == '\n') {
            ++line_;
            column_ = 1;
        } else {
            ++column_;
        }
        return c;
    }

    bool match(char expected) {
        if (peek() != expected) return false;
        advance();
        return true;
    }

    void skip_space_and_comments() {
        for (;;) {
            while (peek() == ' ' || peek() == '\t' || peek() == '\r' || peek() == '\n') {
                advance();
            }
            if (peek() == '/' && peek(1) == '/') {
                while (peek() != '\n' && peek() != '\0') advance();
                continue;
            }
            if (peek() == '/' && peek(1) == '*') {
                skip_block_comment();
                continue;
            }
            break;
        }
    }

    void skip_block_comment() {
        SourceLocation loc = current_location();
        advance();
        advance();
        int depth = 1;
        while (depth > 0) {
            if (peek() == '\0') fail(loc, "unterminated block comment");
            if (peek() == '/' && peek(1) == '*') {
                advance();
                advance();
                ++depth;
            } else if (peek() == '*' && peek(1) == '/') {
                advance();
                advance();
                --depth;
            } else {
                advance();
            }
        }
    }

    static Token simple(TokenKind kind, std::string text, SourceLocation loc) {
        set_location_span(loc, source_span(loc.source_id, loc.byte_start, loc.byte_start + text.size()));
        return Token{kind, std::move(text), 0, loc, 0.0, ""};
    }

    Token identifier(SourceLocation loc) {
        std::string text;
        while (is_alpha(peek()) || is_digit(peek()) || peek() == '_') {
            text.push_back(advance());
        }
        static const std::map<std::string, TokenKind> keywords = {
            {"fn", TokenKind::KwFn},
            {"const", TokenKind::KwConst},
            {"as", TokenKind::KwAs},
            {"meta", TokenKind::KwMeta},
            {"struct", TokenKind::KwStruct},
            {"extern", TokenKind::KwExtern},
            {"enum", TokenKind::KwEnum},
            {"trait", TokenKind::KwTrait},
            {"dyn", TokenKind::KwDyn},
            {"match", TokenKind::KwMatch},
            {"mod", TokenKind::KwMod},
            {"pub", TokenKind::KwPub},
            {"use", TokenKind::KwUse},
            {"impl", TokenKind::KwImpl},
            {"for", TokenKind::KwFor},
            {"in", TokenKind::KwIn},
            {"let", TokenKind::KwLet},
            {"var", TokenKind::KwVar},
            {"own", TokenKind::KwOwn},
            {"ref", TokenKind::KwRef},
            {"mut", TokenKind::KwMut},
            {"ptr", TokenKind::KwPtr},
            {"return", TokenKind::KwReturn},
            {"if", TokenKind::KwIf},
            {"else", TokenKind::KwElse},
            {"while", TokenKind::KwWhile},
            {"init", TokenKind::KwInit},
            {"next", TokenKind::KwNext},
            {"continue", TokenKind::KwContinue},
            {"break", TokenKind::KwBreak},
            {"drop", TokenKind::KwDrop},
            {"forget", TokenKind::KwForget},
            {"null", TokenKind::KwNull},
            {"true", TokenKind::KwTrue},
            {"false", TokenKind::KwFalse}
        };
        auto found = keywords.find(text);
        finish_location(loc, index_);
        if (found != keywords.end()) return Token{found->second, text, 0, loc, 0.0, ""};
        return Token{TokenKind::Identifier, text, 0, loc, 0.0, ""};
    }

    Token number(SourceLocation loc) {
        if (peek() == '0' && (peek(1) == 'x' || peek(1) == 'X' ||
                              peek(1) == 'o' || peek(1) == 'O' ||
                              peek(1) == 'b' || peek(1) == 'B')) {
            return based_integer(loc);
        }

        std::string text;
        bool is_float = false;
        while (is_digit(peek())) text.push_back(advance());
        if (peek() == '.' && is_digit(peek(1))) {
            is_float = true;
            text.push_back(advance());
            while (is_digit(peek())) text.push_back(advance());
        }
        if ((peek() == 'e' || peek() == 'E') &&
            (is_digit(peek(1)) || ((peek(1) == '+' || peek(1) == '-') && is_digit(peek(2))))) {
            is_float = true;
            text.push_back(advance());
            if (peek() == '+' || peek() == '-') text.push_back(advance());
            while (is_digit(peek())) text.push_back(advance());
        }
        std::string suffix;
        if (is_alpha(peek())) {
            while (is_suffix_char(peek())) suffix.push_back(advance());
            if (is_float) {
                if (!is_float_suffix(suffix)) fail(loc, "float literal suffix must be f32, f64, or f128");
            } else if (is_float_suffix(suffix)) {
                is_float = true;
            } else if (!is_integer_suffix(suffix)) {
                fail(loc, "unsupported numeric literal suffix '" + suffix + "'");
            }
        }
        if (is_float) {
            try {
                finish_location(loc, index_);
                Token token{TokenKind::Float, text, 0, loc, 0.0, ""};
                token.literal_suffix = std::move(suffix);
                token.float_value = std::stod(text);
                return token;
            } catch (const std::out_of_range&) {
                fail(loc, "float literal is too large");
            } catch (const std::invalid_argument&) {
                fail(loc, "invalid float literal");
            }
        }
        try {
            finish_location(loc, index_);
            Token token{TokenKind::Integer, text, parse_integer_digits(loc, text, 10, "decimal"), loc, 0.0, ""};
            token.literal_suffix = std::move(suffix);
            return token;
        } catch (const CompileError&) {
            throw;
        }
    }

    Token based_integer(SourceLocation loc) {
        std::string text;
        text.push_back(advance());
        char marker = advance();
        text.push_back(marker);

        int base = 10;
        std::string kind;
        if (marker == 'x' || marker == 'X') {
            base = 16;
            kind = "hexadecimal";
        } else if (marker == 'o' || marker == 'O') {
            base = 8;
            kind = "octal";
        } else {
            base = 2;
            kind = "binary";
        }

        std::string digits;
        while (is_digit_for_base(peek(), base)) {
            char c = advance();
            text.push_back(c);
            digits.push_back(c);
        }

        if (digits.empty()) fail(loc, kind + " literal requires at least one digit after " + text);
        if ((base == 2 || base == 8) && is_digit(peek())) {
            char invalid = peek();
            fail(current_location(), "invalid digit '" + std::string(1, invalid) + "' in " + kind + " literal");
        }
        if (peek() == '.') fail(current_location(), "non-decimal float literals are not supported");

        std::string suffix;
        if (is_alpha(peek())) {
            while (is_suffix_char(peek())) suffix.push_back(advance());
            if (is_float_suffix(suffix)) fail(loc, "non-decimal float literal suffixes are not supported");
            if (!is_integer_suffix(suffix)) fail(loc, "unsupported numeric literal suffix '" + suffix + "'");
        }

        finish_location(loc, index_);
        Token token{TokenKind::Integer, text, parse_integer_digits(loc, digits, base, kind), loc, 0.0, ""};
        token.literal_suffix = std::move(suffix);
        return token;
    }

    Token string(SourceLocation loc) {
        std::string text;
        for (;;) {
            char c = advance();
            if (c == '\0' || c == '\n') fail(loc, "unterminated string literal");
            if (c == '"') {
                finish_location(loc, index_);
                return Token{TokenKind::String, text, 0, loc, 0.0, ""};
            }
            if (c == '\\') {
                append_string_escape(text, loc);
            } else {
                text.push_back(c);
            }
        }
    }

    Token byte_char(SourceLocation loc) {
        std::uint64_t value = byte_char_value(loc);
        if (!match('\'')) {
            fail(loc, "byte character literal must contain exactly one byte");
        }
        finish_location(loc, index_);
        Token token{TokenKind::Integer, "", value, loc, 0.0, ""};
        token.literal_suffix = "char";
        return token;
    }

    std::uint64_t byte_char_value(SourceLocation literal_loc) {
        SourceLocation loc = current_location();
        char c = advance();
        if (c == '\0' || c == '\n') fail(literal_loc, "unterminated byte character literal");
        if (c == '\'') fail(literal_loc, "empty byte character literal");
        if (c == '\\') return byte_char_escape(literal_loc);

        unsigned char byte = static_cast<unsigned char>(c);
        if (byte > 0x7f) {
            fail(loc, "byte character literal must be ASCII; use a string literal for Unicode text");
        }
        return byte;
    }

    std::uint64_t byte_char_escape(SourceLocation literal_loc) {
        SourceLocation escape_loc = current_location();
        char escaped = advance();
        switch (escaped) {
            case 'a': return '\a';
            case 'b': return '\b';
            case 'e': return 0x1b;
            case 'f': return '\f';
            case 'n': return '\n';
            case 'r': return '\r';
            case 't': return '\t';
            case 'v': return '\v';
            case '"': return '"';
            case '\'': return '\'';
            case '?': return '?';
            case '\\': return '\\';
            case 'x': return read_byte_escape_value(escape_loc, "\\x");
            case 'u': return read_ascii_unicode_escape_value(escape_loc, 4);
            case 'U': return read_ascii_unicode_escape_value(escape_loc, 8);
            case '\0':
            case '\n':
                fail(literal_loc, "unterminated byte character literal");
            default:
                if (is_octal_digit(escaped)) {
                    return read_octal_escape_value(escape_loc, escaped);
                }
                fail(escape_loc, "unsupported byte character escape");
        }
    }

    void append_string_escape(std::string& text, SourceLocation string_loc) {
        SourceLocation escape_loc = current_location();
        char escaped = advance();
        switch (escaped) {
            case 'a': text.push_back('\a'); break;
            case 'b': text.push_back('\b'); break;
            case 'e': text.push_back('\x1b'); break;
            case 'f': text.push_back('\f'); break;
            case 'n': text.push_back('\n'); break;
            case 'r': text.push_back('\r'); break;
            case 't': text.push_back('\t'); break;
            case 'v': text.push_back('\v'); break;
            case '"': text.push_back('"'); break;
            case '\'': text.push_back('\''); break;
            case '?': text.push_back('?'); break;
            case '\\': text.push_back('\\'); break;
            case 'x': append_hex_byte_escape(text, escape_loc); break;
            case 'u': append_unicode_escape(text, escape_loc, 4); break;
            case 'U': append_unicode_escape(text, escape_loc, 8); break;
            case '\r':
                if (match('\n')) break;
                fail(escape_loc, "unsupported string escape");
            case '\n':
                break;
            case '\0':
                fail(string_loc, "unterminated string literal");
            default:
                if (is_octal_digit(escaped)) {
                    append_octal_escape(text, escape_loc, escaped);
                    break;
                }
                fail(escape_loc, "unsupported string escape");
        }
    }

    std::string read_digits_for_escape(SourceLocation loc, int base, const std::string& kind, bool require_digit) {
        std::string digits;
        while (is_digit_for_base(peek(), base)) digits.push_back(advance());
        if (require_digit && digits.empty()) fail(loc, kind + " escape requires at least one digit");
        return digits;
    }

    std::uint64_t read_byte_escape_value(SourceLocation loc, const std::string& kind) {
        std::string digits = read_digits_for_escape(loc, 16, kind, true);
        std::uint64_t value = parse_integer_digits(loc, digits, 16, "byte escape");
        if (value > 0xff) fail(loc, kind + " escape value must fit in one byte");
        return value;
    }

    std::uint64_t read_octal_escape_value(SourceLocation loc, char first) {
        std::string digits;
        digits.push_back(first);
        while (is_octal_digit(peek())) digits.push_back(advance());
        std::uint64_t value = parse_integer_digits(loc, digits, 8, "octal escape");
        if (value > 0xff) fail(loc, "octal escape value must fit in one byte");
        return value;
    }

    std::uint64_t read_ascii_unicode_escape_value(SourceLocation loc, int fixed_digits) {
        std::string digits;
        if (fixed_digits == 4 && match('{')) {
            while (peek() != '}') {
                if (peek() == '\0' || peek() == '\n') fail(loc, "unterminated Unicode escape");
                if (!is_digit_for_base(peek(), 16)) fail(current_location(), "invalid digit in Unicode escape");
                digits.push_back(advance());
            }
            advance();
            if (digits.empty()) fail(loc, "\\u{} escape requires at least one digit");
        } else {
            for (int i = 0; i < fixed_digits; ++i) {
                if (!is_digit_for_base(peek(), 16)) {
                    fail(loc, fixed_digits == 4 ? "\\u escape requires exactly 4 hex digits" : "\\U escape requires exactly 8 hex digits");
                }
                digits.push_back(advance());
            }
        }

        std::uint64_t value = parse_integer_digits(loc, digits, 16, "Unicode escape");
        if (value > 0x7f) fail(loc, "Unicode escape in byte character literal must fit in ASCII");
        return value;
    }

    void append_hex_byte_escape(std::string& text, SourceLocation loc) {
        text.push_back(static_cast<char>(read_byte_escape_value(loc, "\\x")));
    }

    void append_octal_escape(std::string& text, SourceLocation loc, char first) {
        text.push_back(static_cast<char>(read_octal_escape_value(loc, first)));
    }

    void append_unicode_escape(std::string& text, SourceLocation loc, int fixed_digits) {
        std::string digits;
        if (fixed_digits == 4 && match('{')) {
            while (peek() != '}') {
                if (peek() == '\0' || peek() == '\n') fail(loc, "unterminated Unicode escape");
                if (!is_digit_for_base(peek(), 16)) fail(current_location(), "invalid digit in Unicode escape");
                digits.push_back(advance());
            }
            advance();
            if (digits.empty()) fail(loc, "\\u{} escape requires at least one digit");
        } else {
            for (int i = 0; i < fixed_digits; ++i) {
                if (!is_digit_for_base(peek(), 16)) {
                    fail(loc, fixed_digits == 4 ? "\\u escape requires exactly 4 hex digits" : "\\U escape requires exactly 8 hex digits");
                }
                digits.push_back(advance());
            }
        }

        std::uint64_t value = parse_integer_digits(loc, digits, 16, "Unicode escape");
        append_utf8(text, loc, static_cast<std::uint32_t>(value));
    }

    [[noreturn]] void fail(SourceLocation loc, const std::string& message) const {
        loc.has_byte_range = true;
        if (loc.byte_end == loc.byte_start) {
            if (loc.byte_start < index_) {
                loc.byte_end = index_;
            } else if (loc.byte_start < source_.size()) {
                loc.byte_end = loc.byte_start + 1;
            }
        }
        set_location_span(loc, default_source_map().span(loc.source_id, loc.byte_start, loc.byte_end));
        CompileError error(std::move(loc), message);
        if (is_unexpected_input_message(message)) {
            error.add_note(DiagnosticNote{
                std::nullopt,
                "the lexer can only tokenize characters that are part of Ari syntax",
                DiagnosticNoteKind::Note});
            error.add_note(DiagnosticNote{
                std::nullopt,
                "remove this input or put text data inside a string literal",
                DiagnosticNoteKind::Help});
        } else if (is_unterminated_string_message(message)) {
            error.add_note(DiagnosticNote{
                std::nullopt,
                "string literals must close before a newline or end of file",
                DiagnosticNoteKind::Note});
            error.add_note(DiagnosticNote{
                std::nullopt,
                "add a closing \" before the line ends",
                DiagnosticNoteKind::Help});
        } else if (is_unterminated_block_comment_message(message)) {
            error.add_note(DiagnosticNote{
                std::nullopt,
                "block comments must close before end of file",
                DiagnosticNoteKind::Note});
            error.add_note(DiagnosticNote{
                std::nullopt,
                "add */ to close the block comment",
                DiagnosticNoteKind::Help});
        } else if (is_unterminated_unicode_escape_message(message)) {
            error.add_note(DiagnosticNote{
                std::nullopt,
                "Unicode escapes must close with } before the string literal continues",
                DiagnosticNoteKind::Note});
            error.add_note(DiagnosticNote{
                std::nullopt,
                "add } after the hexadecimal code point digits",
                DiagnosticNoteKind::Help});
        }
        throw error;
    }
};

std::vector<Token> lex_source(std::string source) {
    Lexer lexer(std::move(source));
    return lexer.lex();
}

std::vector<Token> lex_source(std::string source, std::string source_name) {
    Lexer lexer(std::move(source), std::move(source_name));
    return lexer.lex();
}

} // namespace ari
