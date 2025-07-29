#pragma once

#include "Logger.hpp"
#include "Token.hpp"
#include "ast/Stmt.hpp"

namespace phantom {
  class Parser {
public:
    Parser(const std::vector<Token>& tokens, const Logger& logger)
        : tokens(tokens), logger(logger), index(0) {}

    vec::Vec<Stmt> parse();

private:
    const std::vector<Token>& tokens;
    const Logger& logger;
    size_t index = 0;

    Token consume();
    Token peek(off_t offset = 0) const;
    bool match(Token::Kind token, off_t offset = 0) const;

    std::string expect(Token::Kind kind);
    void todo(const std::string& msg);

    Stmt parse_function();
    Stmt parse_return();
    Stmt parse_expmt();
    Stmt parse_stmt();

    Expr parse_expr(const int min_prec = 0);
    Expr parse_prim();

    Expr parse_type();
    Expr parse_literal();

    Type resolve_type(std::string str);
  };
} // namespace phantom
