#ifndef PHANTOM_TOKEN_HPP
#define PHANTOM_TOKEN_HPP

// #include "location.hpp"
#include <Lexer/Location.hpp>

#include <array>
#include <string_view>

namespace phantom {
  enum TokenType {
    // types
    DATA_TYPE, // int, float, ...

    // keywords
    KEYWORD, // return, let, ...

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
    GREATER_THAN,      // >
    LESS_THAN,         // <
    AND,               // &
    OR,                // |

    // numeric literals
    INTEGER_LITERAL,
    STRING_LITERAL,
    FLOAT_LITERAL,
    CHAR_LITERAL,

    // enf of file flag
    EndOfFile,

    // Invalid Tokens
    INVALID,
  };

  constexpr const std::array<std::string_view, 8> keywords{
      "false",
      "fn",
      "if",
      "let",
      "return",
      "true",
      "while",
      "write",
  };

  constexpr const std::array<std::string_view, 10> types{
      "bool",   // 1-bit integer
      "char",   // 8-bit integer
      "double", // 64-bit float
      "float",  // 32-bit float

      // "half",   // 16-bit float, removed currently
      // "huge",   // 128-bit integer, removed currently
      // because there's no way to store that value in c++

      "int",  // 32-bit integer
      "long", // 64-bit integer
      "ptr",
      "quad",  // 128-bit float
      "short", // 16-bit integer
  };

  constexpr const std::array<char, 16> punctuation{
      '&',
      '(',
      ')',
      '*',
      '+',
      ',',
      '-',
      '/',
      ':',
      ';',
      '<',
      '=',
      '>',
      '{',
      '|',
      '}',
  };

  struct Token {
    TokenType type;
    std::string form;
    Location location;

    Token(TokenType token, std::string form, Location location);
    Token(TokenType type, Location location);
    explicit Token(Location location);

    Token& set_type(const TokenType& type);
    Token& set_form(const std::string& value);

    constexpr static TokenType punctuation_type(char character) {
      switch (character) {
        case '&':
          return TokenType::AND;
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
        case '<':
          return TokenType::LESS_THAN;
        case '=':
          return TokenType::EQUAL;
        case '>':
          return TokenType::GREATER_THAN;
        case '{':
          return TokenType::OPEN_CURLY_BRACE;
        case '|':
          return TokenType::OR;
        case '}':
          return TokenType::CLOSE_CURLY_BRACE;
        default:
          return TokenType::INVALID;
      }
    }
  };
} // namespace phantom

#endif // !PHANTOM_TOKEN_HPP
