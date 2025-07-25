#ifndef PHANTOM_PARSER_HPP
#define PHANTOM_PARSER_HPP

#include "Logger.hpp"
#include "ast/Stmt.hpp"

namespace phantom {
  class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens, const Logger& logger)
        : tokens(tokens), logger(logger), index(0) {}

    std::vector<std::unique_ptr<Stmt>> parse(Token::Kind limit = Token::Kind::EndOfFile);

private:
    const std::vector<Token>& tokens;
    const Logger& logger;
    size_t index = 0;

    Token consume();
    Token peek(off_t offset = 0) const;
    bool match(Token::Kind token, off_t offset = 0) const;

    // function call expression parsing
    std::unique_ptr<Expr> parse_function_call_expression(const std::string& name);

    // Expression parsing
    std::unique_ptr<Expr> parse_primary();
    std::unique_ptr<Expr> parse_expression(int min_prec = 0);

    // return statement parsing
    std::unique_ptr<Stmt> parse_return();

    // function parsing
    std::unique_ptr<VarDecExpr> parse_param();
    std::unique_ptr<Stmt> parse_function();

    // variable declaration parsing
    // starting with "let"
    std::unique_ptr<Stmt> parse_variable_declaration();

    // keywords parsing
    std::unique_ptr<Stmt> parse_keyword();

    // statements starting with an identifier
    std::unique_ptr<Stmt> parse_expr_stt();

    // statements parsing
    std::unique_ptr<Stmt> parse_statement();

    static int precedence(const Token::Kind type);

    static bool right_associative(const Token::Kind type);
  };
} // namespace phantom

#endif // !PHANTOM_PARSER_HPP
