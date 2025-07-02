#include <utility>

// #include "../../include/lexer/token.hpp"
#include <Lexer/Token.hpp>

namespace phantom {
  Token::Token(TokenType type, std::string  form, Location location)
      : type(type),
        form(std::move(form)),
        location(location) {}

  Token::Token(TokenType type, Location location)
      : type(type),
        location(location) {}

  Token::Token(Location location)
    : location(location) {}

  Token& Token::set_type(const TokenType& type) {
    this->type = type;
    return *this;
  }

  Token& Token::set_form(const std::string& form) {
    this->form = form;
    return *this;
  }
} // namespace phantom
