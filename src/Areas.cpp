#include <Areas.hpp>

namespace phantom {
  void ExprArea::add(Expr& expr) {
    this->exprs.emplace_back(expr);
  }
  void ExprArea::dump() {
    this->exprs.clear();
  }

  void StmtArea::add(Stmt& stmt) {
    this->stmts.emplace_back(stmt);
  }
  void StmtArea::dump() {
    this->stmts.clear();
  }
} // namespace phantom
