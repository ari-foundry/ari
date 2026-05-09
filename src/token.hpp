#pragma once

#include "common.hpp"

#include <cstdint>
#include <string>

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
    KwNull,
    KwTrue,
    KwFalse
};

struct Token {
    TokenKind kind = TokenKind::End;
    std::string text;
    std::uint64_t int_value = 0;
    SourceLocation loc;
    double float_value = 0.0;
    std::string literal_suffix;
};

} // namespace ari
