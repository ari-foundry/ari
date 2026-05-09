#include "cfg_eval.hpp"

#include "common.hpp"

#include <set>
#include <string>
#include <vector>

namespace ari {
namespace {

[[noreturn]] void cfg_fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

bool is_active_target(const std::string& name) {
#if defined(_WIN32)
    if (name == "windows") return true;
#elif defined(__APPLE__)
    if (name == "macos" || name == "unix") return true;
#elif defined(__linux__)
    if (name == "linux" || name == "unix") return true;
#endif

#if defined(__x86_64__) || defined(_M_X64)
    if (name == "x86_64") return true;
#elif defined(__aarch64__) || defined(_M_ARM64)
    if (name == "aarch64") return true;
#elif defined(__i386__) || defined(_M_IX86)
    if (name == "x86") return true;
#endif

    return false;
}

class CfgParser {
public:
    CfgParser(const Attribute& attr, const std::set<std::string>& features)
        : attr_(attr), tokens_(attr.args), features_(features) {}

    bool parse() {
        if (tokens_.empty()) cfg_fail(attr_.loc, "attribute '@cfg' expects arguments");
        bool value = parse_expr();
        if (!at_end()) cfg_fail(peek().loc, "unexpected token in @cfg expression");
        return value;
    }

private:
    const Attribute& attr_;
    const std::vector<Token>& tokens_;
    const std::set<std::string>& features_;
    std::size_t pos_ = 0;

    bool at_end() const {
        return pos_ >= tokens_.size();
    }

    const Token& peek() const {
        if (at_end()) return tokens_.back();
        return tokens_[pos_];
    }

    bool match(TokenKind kind) {
        if (at_end() || tokens_[pos_].kind != kind) return false;
        ++pos_;
        return true;
    }

    Token expect(TokenKind kind, const std::string& message) {
        if (at_end() || tokens_[pos_].kind != kind) {
            SourceLocation loc = at_end() ? attr_.loc : tokens_[pos_].loc;
            cfg_fail(loc, message);
        }
        return tokens_[pos_++];
    }

    bool parse_expr() {
        if (match(TokenKind::KwTrue)) return true;
        if (match(TokenKind::KwFalse)) return false;

        Token name = expect(TokenKind::Identifier, "expected @cfg predicate");
        if (!match(TokenKind::LParen)) {
            cfg_fail(name.loc, "unknown cfg predicate '" + name.text + "'");
        }

        if (name.text == "all") return parse_all(name.loc);
        if (name.text == "any") return parse_any(name.loc);
        if (name.text == "not") return parse_not(name.loc);
        if (name.text == "target") return parse_target(name.loc);
        if (name.text == "feature") return parse_feature(name.loc);
        cfg_fail(name.loc, "unknown cfg predicate '" + name.text + "'");
    }

    bool parse_all(SourceLocation loc) {
        (void)loc;
        if (match(TokenKind::RParen)) return true;
        bool result = true;
        while (true) {
            result = parse_expr() && result;
            if (match(TokenKind::RParen)) return result;
            expect(TokenKind::Comma, "expected , or ) in all(...) cfg predicate");
            if (match(TokenKind::RParen)) return result;
        }
    }

    bool parse_any(SourceLocation loc) {
        (void)loc;
        if (match(TokenKind::RParen)) return false;
        bool result = false;
        while (true) {
            result = parse_expr() || result;
            if (match(TokenKind::RParen)) return result;
            expect(TokenKind::Comma, "expected , or ) in any(...) cfg predicate");
            if (match(TokenKind::RParen)) return result;
        }
    }

    bool parse_not(SourceLocation loc) {
        if (match(TokenKind::RParen)) cfg_fail(loc, "cfg not predicate expects one argument");
        bool value = !parse_expr();
        expect(TokenKind::RParen, "cfg not predicate expects exactly one argument");
        return value;
    }

    std::string parse_name_argument(SourceLocation loc, const std::string& predicate) {
        if (at_end() || tokens_[pos_].kind == TokenKind::RParen) {
            cfg_fail(loc, "cfg " + predicate + " predicate expects one string or identifier argument");
        }
        Token arg = tokens_[pos_++];
        if (arg.kind != TokenKind::String && arg.kind != TokenKind::Identifier) {
            cfg_fail(arg.loc, "cfg " + predicate + " predicate expects a string or identifier argument");
        }
        expect(TokenKind::RParen, "cfg " + predicate + " predicate expects exactly one argument");
        return arg.text;
    }

    bool parse_target(SourceLocation loc) {
        std::string name = parse_name_argument(loc, "target");
        return is_active_target(name);
    }

    bool parse_feature(SourceLocation loc) {
        std::string name = parse_name_argument(loc, "feature");
        return features_.count(name) != 0;
    }
};

} // namespace

bool cfg_attribute_enabled(const Attribute& attr, const std::set<std::string>& features) {
    if (attr.name != "cfg") return true;
    return CfgParser(attr, features).parse();
}

} // namespace ari
