#pragma once

#include "ast/Expr.hpp"
#include "ast/Stmt.hpp"

/*
 * NOTE:
 * First Expr/Stmt *MUST* be Invalid to use it in the parser in case of an "Invalid" node
 * or an "Optional" value.
 *
 * TODO:
 * `dump()` should free the inner elements of each `Expr`/`Stmt`
 */

namespace phantom {
  class ExprArea {
public:
    size_t count = 0;
    void add(Expr& expr);
    Expr get(ExprRef expr_ref) const;
    void init();
    void dump();

private:
    std::vector<Expr> exprs;
  };

  class StmtArea {
public:
    size_t count = 0;
    void add(Stmt& stmt);
    Stmt get(StmtRef stmt_ref) const;
    void init();
    void dump();

private:
    std::vector<Stmt> stmts;
  };
} // namespace phantom
