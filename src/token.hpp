#pragma once

#include "common.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace ari {

enum class TokenKind {
    End,
    Identifier,
    Integer,
    Float,
    String,
    LParen,
    RParen,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
    Comma,
    Colon,
    ColonColon,
    Dot,
    DotDot,
    DotDotEqual,
    Ellipsis,
    Semicolon,
    At,
    Question,
    QuestionQuestion,
    Plus,
    PlusEqual,
    Minus,
    MinusEqual,
    Star,
    StarEqual,
    Slash,
    SlashEqual,
    Percent,
    PercentEqual,
    Amp,
    AmpEqual,
    AmpAmp,
    Pipe,
    PipeEqual,
    PipePipe,
    Caret,
    CaretEqual,
    Equal,
    EqEq,
    Bang,
    BangEq,
    Tilde,
    Less,
    LessEq,
    LessLess,
    LessLessEqual,
    Greater,
    GreaterEq,
    GreaterGreater,
    GreaterGreaterEqual,
    Arrow,
    FatArrow,
    KwAs,
    KwConst,
    KwFn,
    KwMeta,
    KwStruct,
    KwExtern,
    KwEnum,
    KwTrait,
    KwDyn,
    KwMatch,
    KwMod,
    KwPub,
    KwUse,
    KwImpl,
    KwFor,
    KwIn,
    KwLet,
    KwVar,
    KwOwn,
    KwRef,
    KwMut,
    KwPtr,
    KwReturn,
    KwIf,
    KwElse,
    KwWhile,
    KwInit,
    KwNext,
    KwContinue,
    KwBreak,
    KwDrop,
    KwForget,
    KwNull,
    KwTrue,
    KwFalse
};

struct Token {
    TokenKind kind = TokenKind::End;
    std::string text;
    std::uint64_t int_value = 0;
    Span span;
    SourceLocation loc;
    double float_value = 0.0;
    std::string literal_suffix;

    Token() = default;

    Token(TokenKind kind,
          std::string text,
          std::uint64_t int_value,
          SourceLocation loc,
          double float_value,
          std::string literal_suffix)
        : kind(kind),
          text(std::move(text)),
          int_value(int_value),
          span(span_from_location(loc)),
          loc(std::move(loc)),
          float_value(float_value),
          literal_suffix(std::move(literal_suffix)) {}

    Token(TokenKind kind,
          std::string text,
          std::uint64_t int_value,
          Span span,
          double float_value,
          std::string literal_suffix)
        : kind(kind),
          text(std::move(text)),
          int_value(int_value),
          span(span),
          loc(source_location_for_span(span)),
          float_value(float_value),
          literal_suffix(std::move(literal_suffix)) {
        if (!has_byte_span(loc) && span_has_source(span)) {
            set_location_span(loc, span);
        }
    }
};

} // namespace ari
