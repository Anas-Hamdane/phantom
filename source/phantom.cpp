#include <fstream>
#include <iostream>
#include <sstream>

#include "include/lexer.hpp"
#include "include/parser.hpp"

namespace phantom {
  std::string toke_type(phantom::TokenType type) {
    switch (type) {
      case phantom::DATA_TYPE:
        return "TYPE";
      case phantom::KEYWORD:
        return "KEYWORD";
      case phantom::IDENTIFIER:
        return "IDENTIFIER";
      case phantom::COLON:
        return "COLON";
      case phantom::SEMI_COLON:
        return "SEMI_COLON";
      case phantom::OPEN_PARENTHESIS:
        return "OPEN_PARENTHESIS";
      case phantom::CLOSE_PARENTHESIS:
        return "CLOSE_PARENTHESIS";
      case phantom::OPEN_CURLY_BRACE:
        return "OPEN_CURLY_BRACE";
      case phantom::CLOSE_CURLY_BRACE:
        return "CLOSE_CURLY_BRACE";
      case phantom::EQUAL:
        return "EQUAL";
      case phantom::PLUS:
        return "PLUS";
      case phantom::MINUS:
        return "MINUS";
      case phantom::STAR:
        return "STAR";
      case phantom::SLASH:
        return "SLASH";
      case phantom::INTEGER_LITERAL:
        return "INTEGER_LITERAL";
      case phantom::EndOfFile:
        return "EndOfFile";
      case phantom::INVALID:
        return "INVALID";
    }

    return "IDK";
  }
} // namespace phantom

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Not enough arguments\n";
    return 1;
  }

  std::string content;
  {
    std::ifstream file(argv[1]);
    std::stringstream ct;
    ct << file.rdbuf();

    content = ct.str();
  }

  phantom::Lexer lexer(content);
  auto tokens = lexer.tokenize();

  for (phantom::Token token : tokens) {
    std::cout << toke_type(token.type) << ": `" << token.form << "`";
    if (token.type == phantom::TokenType::INTEGER_LITERAL)
      std::cout << ", value: " << token.value;

    std::cout << '\n';
  }

  phantom::Parser parser(tokens);
  auto stts = parser.parse();

  std::cout << "statements size: " << stts.size() << '\n';
  std::cout << "Lexer log: " << lexer.get_log() << '\n';
  std::cout << "Parser log: " << parser.get_log() << '\n';
  return 0;
}
