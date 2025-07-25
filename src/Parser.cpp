#include <Parser.hpp>

namespace phantom {
  std::vector<std::unique_ptr<Stmt>> Parser::parse() {

  }

  Token Parser::consume() {
    if (index >= tokens.size())
      return tokens.back();

    Token token = tokens[index];

    index++;
    return token;
  }
  Token Parser::peek(off_t offset) const {
    if (index + offset >= tokens.size())
      return tokens.back();

    return tokens[index + offset];
  }
  bool Parser::match(Token::Kind kind, off_t offset) const {
    if ((index + offset) >= tokens.size())
      return (kind == tokens.back().kind);

    return (kind == tokens[index + offset].kind);
  }
} // namespace phantom
