#pragma once

#include "ast/Expr.hpp"
#include "ast/Stmt.hpp"

namespace phantom {
  class ExprArea {
public:
    void add(Expr& expr);
    void dump();

private:
    std::vector<Expr> exprs;
  };

  class StmtArea {
public:
    void add(Stmt& stmt);
    void dump();

private:
    std::vector<Expr> stmts;
  };
} // namespace phantom
