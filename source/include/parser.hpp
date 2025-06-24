#ifndef PHANTOM_PARSER_HPP
#define PHANTOM_PARSER_HPP

#include "statement.hpp"
#include "token.hpp"
#include <vector>

namespace phantom {
  class Parser {
    std::vector<Token> tokens;
    std::string log;
    size_t index;

    Token peek(size_t offset = 0) const;

    Token consume(size_t offset = 1);

    bool match(TokenType token, size_t offset = 0) const;

    void report_error(const std::string& error);

    std::unique_ptr<Expression> parse_primary();

    std::unique_ptr<Expression> parse_expression(int min_prec = 0);

    std::unique_ptr<Statement> parse_keyword();

    std::unique_ptr<Statement> parse_data_types();

    Parameter parse_param();

    std::unique_ptr<Statement> parse_function();

    std::unique_ptr<Statement> parse_statement();

    constexpr static const int precedence(TokenType type) {
      switch (type) {
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

public:
    Parser(std::vector<Token> tokens);
    std::vector<std::unique_ptr<Statement>> parse(TokenType limit = TokenType::EndOfFile);

    std::string get_log();
  };
} // namespace phantom

#endif // !PHANTOM_PARSER_HPP
