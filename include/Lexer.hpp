#ifndef PHANTOM_LEXER_HPP
#define PHANTOM_LEXER_HPP

#include "info.hpp"
#include <string>
#include "Token.hpp"

namespace phantom {
  class Logger;
  class Lexer {
public:
    explicit Lexer(const std::string& source, const Logger& logger)
        : source(source), logger(logger), line_number(1), column_number(1), index(0) {}

    std::vector<Token> lex();

public:
    enum class NumKind {
      Decimal = 10,
      Hex = 16,
      Octal = 8,
      Binary = 2,
      Mongolien,
    };

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

    static constexpr std::pair<std::string_view, Token::Kind> PrimDataTys[] = {
        {"bool",      Token::Kind::Bool},
        {"char",      Token::Kind::Char},
        {"short",      Token::Kind::Short},
        {"int",      Token::Kind::Int},
        {"long",      Token::Kind::Long},
        {"half",   Token::Kind::Half},
        {"float",   Token::Kind::Float},
        {"double",   Token::Kind::Double},
    };
    // clang-format on

    char consume();
    char peek(const off_t offset = 0) const;
    bool match(const char character, const off_t offset = 0) const;

    bool skip_prefix(const std::string& pattern);
    void skip_until(const std::string& prefix);

    bool identifier_start(const char c);
    bool identifier_valid(const char c);

    bool starts_with(const std::string& str, const std::string& cmp);
    NumKind numkind(const std::string& str);

    Token::Kind invalid_kind(const std::string& msg, Location location);

    Token::Kind scan_dec(const std::string& str);
    Token::Kind scan_hex(const std::string& str);
    Token::Kind scan_oct(const std::string& str);
    Token::Kind scan_bin(const std::string& str);
  };
} // namespace phantom

#endif // !PHANTOM_LEXER_HPP
