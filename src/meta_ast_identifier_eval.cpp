#include "meta_ast_identifier_eval.hpp"

#include "common.hpp"

#include <cctype>
#include <string>
#include <utility>
#include <vector>

namespace ari {
namespace {

[[noreturn]] void fail_eval(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

bool is_open_delimiter(TokenKind kind) {
    return kind == TokenKind::LParen || kind == TokenKind::LBrace || kind == TokenKind::LBracket;
}

TokenKind matching_close(TokenKind kind) {
    if (kind == TokenKind::LParen) return TokenKind::RParen;
    if (kind == TokenKind::LBrace) return TokenKind::RBrace;
    if (kind == TokenKind::LBracket) return TokenKind::RBracket;
    return TokenKind::End;
}

bool is_close_delimiter(TokenKind kind) {
    return kind == TokenKind::RParen || kind == TokenKind::RBrace || kind == TokenKind::RBracket;
}

std::size_t find_matching_delimiter(const std::vector<Token>& tokens, std::size_t open_index) {
    std::vector<TokenKind> closing_stack{matching_close(tokens[open_index].kind)};
    for (std::size_t i = open_index + 1; i < tokens.size(); ++i) {
        TokenKind kind = tokens[i].kind;
        if (is_open_delimiter(kind)) {
            closing_stack.push_back(matching_close(kind));
            continue;
        }
        if (!is_close_delimiter(kind)) continue;
        if (closing_stack.empty() || kind != closing_stack.back()) {
            fail_eval(tokens[i].loc, "mismatched delimiter in meta_ident! identifier constructor");
        }
        closing_stack.pop_back();
        if (closing_stack.empty()) return i;
    }
    fail_eval(tokens[open_index].loc, "unterminated meta_ident! identifier constructor");
}

bool is_identifier_start(char c) {
    unsigned char value = static_cast<unsigned char>(c);
    return std::isalpha(value) || c == '_';
}

bool is_identifier_continue(char c) {
    unsigned char value = static_cast<unsigned char>(c);
    return std::isalnum(value) || c == '_';
}

bool is_keyword(const std::string& text) {
    static const char* keywords[] = {
        "as", "break", "const", "continue", "drop", "dyn", "else", "enum",
        "extern", "false", "fn", "for", "if", "impl", "in", "init", "let",
        "match", "meta", "mod", "mut", "next", "null", "own", "ptr", "pub",
        "ref", "return", "struct", "trait", "true", "use", "var", "while"
    };
    for (const char* keyword : keywords) {
        if (text == keyword) return true;
    }
    return false;
}

bool is_valid_identifier(const std::string& text) {
    if (text.empty() || !is_identifier_start(text.front()) || is_keyword(text)) return false;
    for (char c : text) {
        if (!is_identifier_continue(c)) return false;
    }
    return true;
}

bool is_dynamic_identifier_constructor(const std::vector<Token>& tokens, std::size_t index) {
    return index + 2 < tokens.size() &&
           tokens[index].kind == TokenKind::Identifier &&
           tokens[index].text == "meta_ident" &&
           tokens[index + 1].kind == TokenKind::Bang &&
           tokens[index + 2].kind == TokenKind::LParen;
}

bool matches_input_name_method(const std::vector<Token>& tokens,
                               std::size_t begin,
                               std::size_t end,
                               const std::string& input_name,
                               const std::string& method_name) {
    return end - begin == 5 &&
           tokens[begin].kind == TokenKind::Identifier &&
           tokens[begin].text == input_name &&
           tokens[begin + 1].kind == TokenKind::Dot &&
           tokens[begin + 2].kind == TokenKind::Identifier &&
           tokens[begin + 2].text == method_name &&
           tokens[begin + 3].kind == TokenKind::LParen &&
           tokens[begin + 4].kind == TokenKind::RParen;
}

bool matches_free_input_call(const std::vector<Token>& tokens,
                             std::size_t begin,
                             std::size_t end,
                             const std::string& input_name,
                             const std::string& function_name) {
    return end - begin == 4 &&
           tokens[begin].kind == TokenKind::Identifier &&
           tokens[begin].text == function_name &&
           tokens[begin + 1].kind == TokenKind::LParen &&
           tokens[begin + 2].kind == TokenKind::Identifier &&
           tokens[begin + 2].text == input_name &&
           tokens[begin + 3].kind == TokenKind::RParen;
}

std::string unsupported_part_message(const std::string& input_name) {
    return "meta_ident! parts must be string or identifier literals, " + input_name +
           ".name(), decl_name(" + input_name + "), " + input_name +
           ".kind(), or decl_kind(" + input_name + ")";
}

std::string evaluate_identifier_part(const std::vector<Token>& tokens,
                                     std::size_t begin,
                                     std::size_t end,
                                     const std::string& input_name,
                                     const MetaAstDeclInput& input) {
    if (begin == end) {
        fail_eval(tokens[begin - 1].loc, "meta_ident! does not allow empty identifier parts");
    }
    if (end - begin == 1) {
        const Token& token = tokens[begin];
        if (token.kind == TokenKind::String || token.kind == TokenKind::Identifier) {
            return token.text;
        }
    }
    if (matches_input_name_method(tokens, begin, end, input_name, "name") ||
        matches_free_input_call(tokens, begin, end, input_name, "decl_name")) {
        return input.name;
    }
    if (matches_input_name_method(tokens, begin, end, input_name, "kind") ||
        matches_free_input_call(tokens, begin, end, input_name, "decl_kind")) {
        return input.kind;
    }
    fail_eval(tokens[begin].loc, unsupported_part_message(input_name));
}

std::vector<std::pair<std::size_t, std::size_t>> split_identifier_args(const std::vector<Token>& tokens,
                                                                       std::size_t begin,
                                                                       std::size_t end) {
    std::vector<std::pair<std::size_t, std::size_t>> args;
    std::size_t arg_begin = begin;
    std::vector<TokenKind> closing_stack;
    for (std::size_t i = begin; i < end; ++i) {
        TokenKind kind = tokens[i].kind;
        if (is_open_delimiter(kind)) {
            closing_stack.push_back(matching_close(kind));
            continue;
        }
        if (is_close_delimiter(kind)) {
            if (closing_stack.empty() || kind != closing_stack.back()) {
                fail_eval(tokens[i].loc, "mismatched delimiter in meta_ident! identifier part");
            }
            closing_stack.pop_back();
            continue;
        }
        if (kind == TokenKind::Comma && closing_stack.empty()) {
            args.push_back({arg_begin, i});
            arg_begin = i + 1;
        }
    }
    if (!closing_stack.empty()) {
        fail_eval(tokens[begin].loc, "unterminated delimiter in meta_ident! identifier part");
    }
    args.push_back({arg_begin, end});
    return args;
}

Token evaluate_dynamic_identifier(const std::vector<Token>& tokens,
                                  std::size_t constructor_index,
                                  std::size_t close_index,
                                  const std::string& input_name,
                                  const MetaAstDeclInput& input) {
    std::size_t args_begin = constructor_index + 3;
    if (args_begin == close_index) {
        fail_eval(tokens[constructor_index].loc, "meta_ident! requires at least one identifier part");
    }

    std::string name;
    for (const auto& arg : split_identifier_args(tokens, args_begin, close_index)) {
        name += evaluate_identifier_part(tokens, arg.first, arg.second, input_name, input);
    }
    if (!is_valid_identifier(name)) {
        fail_eval(tokens[constructor_index].loc, "meta_ident! generated invalid identifier '" + name + "'");
    }
    return Token{TokenKind::Identifier, name, 0, tokens[constructor_index].loc, 0.0, ""};
}

} // namespace

std::vector<Token> substitute_meta_decl_tokens(const std::vector<Token>& constructor_tokens,
                                               const std::string& input_name,
                                               const MetaAstDeclInput& input) {
    std::vector<Token> expanded;
    for (std::size_t i = 0; i < constructor_tokens.size(); ++i) {
        const Token& token = constructor_tokens[i];
        if (is_dynamic_identifier_constructor(constructor_tokens, i)) {
            std::size_t close_index = find_matching_delimiter(constructor_tokens, i + 2);
            expanded.push_back(evaluate_dynamic_identifier(constructor_tokens, i, close_index, input_name, input));
            i = close_index;
            continue;
        }
        if (token.kind == TokenKind::Identifier && token.text == "meta_ident" &&
            i + 1 < constructor_tokens.size() &&
            constructor_tokens[i + 1].kind == TokenKind::Bang) {
            fail_eval(token.loc, "meta_ident! identifier constructor requires (...)");
        }
        if (token.kind == TokenKind::Identifier && token.text == input_name) {
            expanded.insert(expanded.end(), input.tokens.begin(), input.tokens.end());
            continue;
        }
        expanded.push_back(token);
    }
    return expanded;
}

} // namespace ari
