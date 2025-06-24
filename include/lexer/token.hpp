#ifndef PHANTOM_TOKEN_HPP
#define PHANTOM_TOKEN_HPP

#include "location.hpp"
#include <array>
#include <string>
#include <string_view>

namespace phantom {
  enum TokenType {
    // types
    DATA_TYPE, // int, float, double, char

    // keywords
    KEYWORD, // return

    // identifiers
    IDENTIFIER, // var_name

    // Punctuation
    COLON,             // :
    SEMI_COLON,        // ;
    COMMA,             // ,
    OPEN_PARENTHESIS,  // (
    CLOSE_PARENTHESIS, // )
    OPEN_CURLY_BRACE,  // {
    CLOSE_CURLY_BRACE, // }
    EQUAL,             // =
    PLUS,              // +
    MINUS,             // -
    STAR,              // *
    SLASH,             // /

    // numeric literals
    INTEGER_LITERAL,
    // FLOAT_LITERAL,

    // enf of file flag
    EndOfFile,

    // Invalid Tokens
    INVALID,
  };

  constexpr const std::array<std::string_view, 4> keywords{
      "fn",
      "if",
      "return",
      "while",
  };

  constexpr const std::array<std::string_view, 4> types{
      "char",
      "double",
      "float",
      "int",
  };

  constexpr const std::array<char, 12> punctuation{
      '(',
      ')',
      '*',
      '+',
      ',',
      '-',
      '/',
      ':',
      ';',
      '=',
      '{',
      '}',
  };

  struct Token {
    TokenType type;
    std::string form;
    long value{};

    Location location;

    Token(TokenType token, std::string  form, long value, Location location);
    Token(TokenType token, std::string  form, Location location);
    Token(TokenType type, Location location);
    explicit Token(Location location);

    Token& set_type(const TokenType& type);
    Token& set_form(const std::string& value);
    Token& set_value(long value);

    constexpr static TokenType punctuation_type(char character) {
      switch (character) {
        case '(':
          return TokenType::OPEN_PARENTHESIS;
        case ')':
          return TokenType::CLOSE_PARENTHESIS;
        case '*':
          return TokenType::STAR;
        case '+':
          return TokenType::PLUS;
        case ',':
          return TokenType::COMMA;
        case '-':
          return TokenType::MINUS;
        case '/':
          return TokenType::SLASH;
        case ':':
          return TokenType::COLON;
        case ';':
          return TokenType::SEMI_COLON;
        case '=':
          return TokenType::EQUAL;
        case '{':
          return TokenType::OPEN_CURLY_BRACE;
        case '}':
          return TokenType::CLOSE_CURLY_BRACE;
        default: return TokenType::INVALID;
      }
    }
  };
} // namespace phantom

#endif // !PHANTOM_TOKEN_HPP
