#include "token_dump.hpp"

#include <sstream>

namespace ari {

static const char* token_kind_name(TokenKind kind) {
    switch (kind) {
        case TokenKind::End: return "End";
        case TokenKind::Identifier: return "Identifier";
        case TokenKind::Integer: return "Integer";
        case TokenKind::Float: return "Float";
        case TokenKind::String: return "String";
        case TokenKind::LParen: return "LParen";
        case TokenKind::RParen: return "RParen";
        case TokenKind::LBrace: return "LBrace";
        case TokenKind::RBrace: return "RBrace";
        case TokenKind::LBracket: return "LBracket";
        case TokenKind::RBracket: return "RBracket";
        case TokenKind::Comma: return "Comma";
        case TokenKind::Colon: return "Colon";
        case TokenKind::ColonColon: return "ColonColon";
        case TokenKind::Dot: return "Dot";
        case TokenKind::DotDot: return "DotDot";
        case TokenKind::DotDotEqual: return "DotDotEqual";
        case TokenKind::Ellipsis: return "Ellipsis";
        case TokenKind::Semicolon: return "Semicolon";
        case TokenKind::At: return "At";
        case TokenKind::Question: return "Question";
        case TokenKind::QuestionQuestion: return "QuestionQuestion";
        case TokenKind::Plus: return "Plus";
        case TokenKind::PlusEqual: return "PlusEqual";
        case TokenKind::Minus: return "Minus";
        case TokenKind::MinusEqual: return "MinusEqual";
        case TokenKind::Star: return "Star";
        case TokenKind::StarEqual: return "StarEqual";
        case TokenKind::Slash: return "Slash";
        case TokenKind::SlashEqual: return "SlashEqual";
        case TokenKind::Percent: return "Percent";
        case TokenKind::PercentEqual: return "PercentEqual";
        case TokenKind::Amp: return "Amp";
        case TokenKind::AmpEqual: return "AmpEqual";
        case TokenKind::AmpAmp: return "AmpAmp";
        case TokenKind::Pipe: return "Pipe";
        case TokenKind::PipeEqual: return "PipeEqual";
        case TokenKind::PipePipe: return "PipePipe";
        case TokenKind::Caret: return "Caret";
        case TokenKind::CaretEqual: return "CaretEqual";
        case TokenKind::Equal: return "Equal";
        case TokenKind::EqEq: return "EqEq";
        case TokenKind::Bang: return "Bang";
        case TokenKind::BangEq: return "BangEq";
        case TokenKind::Tilde: return "Tilde";
        case TokenKind::Less: return "Less";
        case TokenKind::LessEq: return "LessEq";
        case TokenKind::LessLess: return "LessLess";
        case TokenKind::LessLessEqual: return "LessLessEqual";
        case TokenKind::Greater: return "Greater";
        case TokenKind::GreaterEq: return "GreaterEq";
        case TokenKind::GreaterGreater: return "GreaterGreater";
        case TokenKind::GreaterGreaterEqual: return "GreaterGreaterEqual";
        case TokenKind::Arrow: return "Arrow";
        case TokenKind::FatArrow: return "FatArrow";
        case TokenKind::KwAs: return "KwAs";
        case TokenKind::KwConst: return "KwConst";
        case TokenKind::KwFn: return "KwFn";
        case TokenKind::KwMeta: return "KwMeta";
        case TokenKind::KwStruct: return "KwStruct";
        case TokenKind::KwExtern: return "KwExtern";
        case TokenKind::KwEnum: return "KwEnum";
        case TokenKind::KwTrait: return "KwTrait";
        case TokenKind::KwDyn: return "KwDyn";
        case TokenKind::KwMatch: return "KwMatch";
        case TokenKind::KwMod: return "KwMod";
        case TokenKind::KwPub: return "KwPub";
        case TokenKind::KwUse: return "KwUse";
        case TokenKind::KwImpl: return "KwImpl";
        case TokenKind::KwFor: return "KwFor";
        case TokenKind::KwIn: return "KwIn";
        case TokenKind::KwLet: return "KwLet";
        case TokenKind::KwVar: return "KwVar";
        case TokenKind::KwOwn: return "KwOwn";
        case TokenKind::KwRef: return "KwRef";
        case TokenKind::KwMut: return "KwMut";
        case TokenKind::KwPtr: return "KwPtr";
        case TokenKind::KwReturn: return "KwReturn";
        case TokenKind::KwIf: return "KwIf";
        case TokenKind::KwElse: return "KwElse";
        case TokenKind::KwWhile: return "KwWhile";
        case TokenKind::KwInit: return "KwInit";
        case TokenKind::KwNext: return "KwNext";
        case TokenKind::KwContinue: return "KwContinue";
        case TokenKind::KwBreak: return "KwBreak";
        case TokenKind::KwDrop: return "KwDrop";
        case TokenKind::KwForget: return "KwForget";
        case TokenKind::KwNull: return "KwNull";
        case TokenKind::KwTrue: return "KwTrue";
        case TokenKind::KwFalse: return "KwFalse";
    }
    return "Unknown";
}

static std::string escape_text(const std::string& text) {
    std::string escaped;
    for (unsigned char c : text) {
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (c < 0x20 || c == 0x7f) {
                    const char* digits = "0123456789abcdef";
                    escaped += "\\x";
                    escaped.push_back(digits[(c >> 4) & 0xf]);
                    escaped.push_back(digits[c & 0xf]);
                } else {
                    escaped.push_back(static_cast<char>(c));
                }
                break;
        }
    }
    return escaped;
}

std::string dump_tokens(const std::vector<Token>& tokens, const std::string& source_name) {
    std::ostringstream out;
    for (const Token& token : tokens) {
        out << "token " << token_kind_name(token.kind)
            << " \"" << escape_text(token.text) << "\"";
        if (token.kind == TokenKind::Integer) {
            out << " value=" << token.int_value;
        } else if (token.kind == TokenKind::Float) {
            out << " value=" << token.float_value;
        }
        if (!token.literal_suffix.empty()) {
            out << " suffix=\"" << escape_text(token.literal_suffix) << "\"";
        }
        const std::string& token_source = token.loc.source_name.empty() ? source_name : token.loc.source_name;
        out << " @ " << token_source << ":" << token.loc.line << ":" << token.loc.column
            << " source_id=" << source_id_text(token.loc.source_id)
            << " bytes=" << token.loc.byte_start << ".." << token.loc.byte_end << "\n";
    }
    return out.str();
}

} // namespace ari
