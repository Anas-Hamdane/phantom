#pragma once

#include "Token.hpp"
#include <string>

namespace phantom {
  class Logger;
  class Lexer {
public:
    explicit Lexer(const std::string& source, const Logger& logger)
        : source(source), logger(logger), line_number(1), column_number(1), index(0) {}

    std::vector<Token> lex();

private:
    const std::string source;
    const Logger& logger;
    size_t line_number;
    size_t column_number;
    size_t index;

    // clang-format off
    static constexpr std::pair<std::string_view, Token::Kind> Puncts[] = {
        {"?",   Token::Kind::Qst},
        {"{",   Token::Kind::OpenCurly},
        {"}",   Token::Kind::CloseCurly},
        {"(",   Token::Kind::OpenParent},
        {")",   Token::Kind::CloseParent},
        {"[",   Token::Kind::OpenBracket},
        {"]",   Token::Kind::CloseBracket},
        {";",   Token::Kind::SemiColon},
        {":",   Token::Kind::Colon},
        {",",   Token::Kind::Comma},

        {"->",  Token::Kind::RightArrow},
        {"--",  Token::Kind::Dec},
        {"-=",  Token::Kind::MinusEq},
        {"-",   Token::Kind::Minus},

        {"++",  Token::Kind::Inc},
        {"+=",  Token::Kind::PlusEq},
        {"+",   Token::Kind::Plus},

        {"*=",  Token::Kind::MulEq},
        {"*",   Token::Kind::Mul},

        {"%=",  Token::Kind::ModEq},
        {"%",   Token::Kind::Mod},

        {"/=",  Token::Kind::DivEq},
        {"/",   Token::Kind::Div},

        {"|=",  Token::Kind::OrEq},
        {"|",   Token::Kind::Or},

        {"&=",  Token::Kind::AndEq},
        {"&",   Token::Kind::And},

        {"==",  Token::Kind::EqEq},
        {"=",   Token::Kind::Eq},

        {"!=",  Token::Kind::NotEq},
        {"!",   Token::Kind::Not},

        {"<<=", Token::Kind::ShlEq},
        {"<<",  Token::Kind::Shl},
        {"<=",  Token::Kind::LessEq},
        {"<",   Token::Kind::Less},

        {">>=", Token::Kind::ShrEq},
        {">>",  Token::Kind::Shr},
        {">=",  Token::Kind::GreaterEq},
        {">",   Token::Kind::Greater},
    };

    static constexpr std::pair<std::string_view, Token::Kind> Keywords[] = {
        {"let",      Token::Kind::Let},
        {"return",   Token::Kind::Return},
        {"fn",       Token::Kind::Fn},
        {"if",       Token::Kind::If},
        {"else",     Token::Kind::Else},
        {"while",    Token::Kind::While},
        {"for",      Token::Kind::For},
    };

    static constexpr std::string_view PrimDataTys[] = {
        "void",

        // integers
        "i1",
        "i8",
        "i16",
        "i32",
        "i64",

        // unsigned integers
        // "u1",
        // "u8",
        // "u16",
        // "u32",
        // "u64",

        // floating points
        "f16",
        "f32",
        "f64"
    };
    // clang-format on

    char consume();
    char peek(const off_t offset = 0) const;
    bool match(const char character, const off_t offset = 0) const;

    bool skip_prefix(const std::string& pattern);
    void skip_until(const std::string& prefix);

    bool identifier_start(const char c);
    bool identifier_valid(const char c);
  };
} // namespace phantom
