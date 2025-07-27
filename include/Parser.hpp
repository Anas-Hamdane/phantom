#pragma once

#include "Areas.hpp"
#include "Logger.hpp"
#include "Token.hpp"

namespace phantom {
  class Parser {
public:
    Parser(const std::vector<Token>& tokens, const Logger& logger, ExprArea& expr_rea, StmtArea& stmt_area)
        : tokens(tokens), logger(logger), expr_area(expr_rea), stmt_area(stmt_area), index(0) {}

    std::vector<StmtRef> parse();

private:
    const std::vector<Token>& tokens;
    const Logger& logger;
    ExprArea& expr_area;
    StmtArea& stmt_area;
    size_t index = 0;

    Token consume();
    Token peek(off_t offset = 0) const;
    bool match(Token::Kind token, off_t offset = 0) const;

    std::string expect(Token::Kind kind);
    void todo(const std::string& msg);

    StmtRef parse_function();
    StmtRef parse_return();
    StmtRef parse_expmt();
    StmtRef parse_stmt();

    ExprRef parse_expr(const int min_prec = 0);
    ExprRef parse_prim();

    ExprRef parse_type();
    ExprRef parse_literal();
  };
} // namespace phantom
