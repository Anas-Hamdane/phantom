#ifndef PHANTOM_TOKEN_HPP
#define PHANTOM_TOKEN_HPP

#include <array>
#include <string>
#include <string_view>

#include "info.hpp"

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
    AMPERSAND,         // &
    BAR,               // |

    // literals
    INTEGER_LITERAL,
    STRING_LITERAL,
    FLOAT_LITERAL,
    CHAR_LITERAL,

    // enf of file flag
    ENDOFFILE,

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

    explicit Token(const TokenType& type, const std::string& form, const Location& location)
        : type(type), form(form), location(location) {}

    explicit Token(const TokenType& type, const Location& location)
        : type(type), location(location) {}

    constexpr static TokenType punctuation_type(char character) {
      switch (character) {
        case '&':
          return TokenType::AMPERSAND;
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
          return TokenType::BAR;
        case '}':
          return TokenType::CLOSE_CURLY_BRACE;
        default:
          return TokenType::INVALID;
      }
    }

    static std::string get_token_type(const TokenType& type) {
      switch (type) {
        case TokenType::STAR:
          return "STAR";
        case TokenType::SLASH:
          return "SLASH";
        case TokenType::BAR:
          return "BAR";
        case TokenType::AMPERSAND:
          return "AMPERSAND";
        case TokenType::DATA_TYPE:
          return "DATA_TYPE";
        case TokenType::CHAR_LITERAL:
          return "CHAR_LITERAL";
        case TokenType::FLOAT_LITERAL:
          return "FLOAT_LITERAL";
        case TokenType::INTEGER_LITERAL:
          return "INTEGER_LITERAL";
        case TokenType::STRING_LITERAL:
          return "STRING_LITERAL";
        case TokenType::CLOSE_PARENTHESIS:
          return "CLOSE_PARENTHESIS";
        case TokenType::CLOSE_CURLY_BRACE:
          return "CLOSE_CURLY_BRACE";
        case TokenType::OPEN_CURLY_BRACE:
          return "OPEN_CURLY_BRACE";
        case TokenType::COLON:
          return "COLON";
        case TokenType::SEMI_COLON:
          return "SEMI_COLON";
        case TokenType::COMMA:
          return "COMMA";
        case TokenType::ENDOFFILE:
          return "EndOfFile";
        case TokenType::EQUAL:
          return "EQUAL";
        case TokenType::GREATER_THAN:
          return "GREATER_THAN";
        case TokenType::LESS_THAN:
          return "LESS_THAN";
        case TokenType::IDENTIFIER:
          return "IDENTIFIER";
        case TokenType::KEYWORD:
          return "KEYWORD";
        case TokenType::MINUS:
          return "MINUS";
        case TokenType::PLUS:
          return "PLUS";
        case TokenType::OPEN_PARENTHESIS:
          return "OPEN_PARENTHESIS";
        case TokenType::INVALID:
          return "INVALID";
        default:
          return "UNKNOWN";
      }
    }
  };
} // namespace phantom

#endif // !PHANTOM_TOKEN_HPP
