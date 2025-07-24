#ifndef PHANTOM_LEXER_HPP
#define PHANTOM_LEXER_HPP

#include "info.hpp"
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace phantom {
  class Logger;

  struct Token {
    enum class Kind {
      // data types
      Bool,
      Char,
      Short,
      Int,
      Huge,
      Float,
      Double,
      Quad,

      // keywords
      Return,
      Let,
      Fn,
      If,
      Else,
      While,
      For,

      // identifiers
      Identifier,

      // ponctuation
      Colon,
      SemiColon,
      Comma,
      OpenCurly,
      CloseCurly,
      OpenParent,
      CloseParent,
      OpenBracket,
      CloseBracket,
      Not,
      Mul,
      Div,
      Mod,
      And,
      Or,
      Plus,
      Minus,
      Inc,
      Dec,
      Less,
      LessEq,
      Greater,
      GreaterEq,
      Eq,
      EqEq,
      NotEq,
      Shl,
      ShlEq,
      Shr,
      ShrEq,
      ModEq,
      OrEq,
      AndEq,
      PlusEq,
      MinusEq,
      MulEq,
      DivEq,
      Qst,

      // literals
      IntLit,
      FloatLit,
      CharLit,
      StrLit,

      // enf of file flag
      EndOfFile,

      // Invalid Tokens
      Mongolien,
    };
    Kind kind;
    std::string form;
    Location location;

    Token(const Kind& kind, const Location& location)
        : kind(kind), location(location) {}

    Token(const Kind& kind, const std::string& form, const Location& location)
        : kind(kind), form(form), location(location) {}
  };

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
      Binary = 2
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
        {"return",   Token::Kind::Return},
        {"fn",       Token::Kind::Fn},
        {"if",       Token::Kind::If},
        {"else",     Token::Kind::Else},
        {"while",    Token::Kind::While},
        {"for",      Token::Kind::For},
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

    uint64_t parse_hex(size_t start, const std::string& str, size_t end);
    uint64_t parse_dec(size_t start, const std::string& str, size_t end);
    uint64_t parse_oct(size_t start, const std::string& str, size_t end);
    uint64_t parse_bin(size_t start, const std::string& str, size_t end);

    uint64_t parse_int(const std::string& str);
    long double parse_float(const std::string& str);

    // constexpr static int recognize_punctuation(const char character);
    // constexpr static int recognize_type(const std::string& buffer);
    // constexpr static int recognize_keyword(const std::string& buffer);
    //
    // bool new_line(char character);
    // bool whitespace(char character);
    // bool alphabet(char character);
    // bool digit(char character);
    //
    // bool alpha_digit(char character);
    // bool float_suffix(char character);
    // bool double_suffixe(char character);
    // Token invalid_token(std::string form);
    // Token handle_numerics();
    // Token handle_words();
    // bool escaped(size_t pos);
    // Token eat_character();
    // Token eat_string();
  };
} // namespace phantom

#endif // !PHANTOM_LEXER_HPP
