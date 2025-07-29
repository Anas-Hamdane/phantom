#pragma once

#include "Logger.hpp"
#include "Token.hpp"
#include "ast/Stmt.hpp"

namespace phantom {
  class Parser {
public:
    Parser(const std::vector<Token>& tokens, const Logger& logger)
        : tokens(tokens), logger(logger), index(0) {}

    std::vector<Stmt> parse();

private:
    const std::vector<Token>& tokens;
    const Logger& logger;
    size_t index = 0;

    Token consume();
    Token peek(off_t offset = 0) const;
    bool match(Token::Kind token, off_t offset = 0) const;

    std::string expect(Token::Kind kind);
    void todo(const std::string& msg);

    std::unique_ptr<Stmt> parse_function();
    std::unique_ptr<Stmt> parse_return();
    std::unique_ptr<Stmt> parse_expmt();
    std::unique_ptr<Stmt> parse_stmt();

    std::unique_ptr<Expr> parse_expr(const int min_prec = 0);
    std::unique_ptr<Expr> parse_prim();

    std::unique_ptr<Expr> parse_type();
    std::unique_ptr<Expr> parse_literal();

    Type resolve_type(std::string str);
  };
} // namespace phantom
