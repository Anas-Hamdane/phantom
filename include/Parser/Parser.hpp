#ifndef PHANTOM_PARSER_HPP
#define PHANTOM_PARSER_HPP

// #include "statement.hpp"
// #include "../lexer/token.hpp"
#include <Lexer/Token.hpp>
#include <Parser/Statements.hpp>

#include <vector>

namespace phantom {
  class Parser {
    std::vector<Token> tokens;
    size_t index;

    Token peek(off_t offset = 0) const;

    Token consume(off_t offset = 1);

    bool match(TokenType token, off_t offset = 0) const;

    // function call expression parsing
    std::unique_ptr<Expression> parse_function_call_expression(std::string name);

    // Expression parsing
    std::unique_ptr<Expression> parse_primary();
    std::unique_ptr<Expression> parse_expression(int min_prec = 0);

    // return statement parsing
    std::unique_ptr<Statement> parse_return();

    // function parsing
    Parameter parse_param();
    std::unique_ptr<Statement> parse_function();

    // variable declaration parsing
    // starting with "let"
    std::unique_ptr<Statement> parse_variable_declaration();

    // keywords parsing
    std::unique_ptr<Statement> parse_keyword();

    // statements starting with an identifier
    std::unique_ptr<Statement> parse_identifier();

    // statements parsing
    std::unique_ptr<Statement> parse_statement();

    const static int precedence(const TokenType type) {
      switch (type) {
        case TokenType::EQUAL:
          return 5;
        case TokenType::PLUS:
        case TokenType::MINUS:
          return 10;
        case TokenType::STAR:
        case TokenType::SLASH:
          return 20;
        default:
          return 0;
      }
    }

    const static bool right_associative(const TokenType type) {
      return type == TokenType::EQUAL;
    }

public:
    explicit Parser(std::vector<Token> tokens);

    // general parsing
    std::vector<std::unique_ptr<Statement>> parse(TokenType limit = TokenType::EndOfFile);
  };
} // namespace phantom

#endif // !PHANTOM_PARSER_HPP
