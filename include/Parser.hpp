#ifndef PHANTOM_PARSER_HPP
#define PHANTOM_PARSER_HPP

#include "ast/Statement.hpp"
#include "Logger.hpp"

namespace phantom {
  class Parser {
    const std::vector<Token>& tokens;
    const Logger& logger;
    size_t index;

    Token peek(off_t offset = 0) const;

    Token consume(off_t offset = 1);

    bool match(Token::Kind token, off_t offset = 0) const;

    // function call expression parsing
    std::unique_ptr<Expression> parse_function_call_expression(const std::string& name);

    // Expression parsing
    std::unique_ptr<Expression> parse_primary();
    std::unique_ptr<Expression> parse_expression(int min_prec = 0);

    // return statement parsing
    std::unique_ptr<Statement> parse_return();

    // function parsing
    std::unique_ptr<VarDecStt> parse_param();
    std::unique_ptr<Statement> parse_function();

    // variable declaration parsing
    // starting with "let"
    std::unique_ptr<Statement> parse_variable_declaration();

    // keywords parsing
    std::unique_ptr<Statement> parse_keyword();

    // statements starting with an identifier
    std::unique_ptr<Statement> parse_expr_stt();

    // statements parsing
    std::unique_ptr<Statement> parse_statement();

    static int precedence(const Token::Kind type);

    static bool right_associative(const Token::Kind type);

public:
    explicit Parser(const std::vector<Token>& tokens, const Logger& logger)
      : tokens(tokens), logger(logger), index(0) {}

    // general parsing
    std::vector<std::unique_ptr<Statement>> parse(Token::Kind limit = Token::Kind::EndOfFile);
  };
} // namespace phantom

#endif // !PHANTOM_PARSER_HPP
