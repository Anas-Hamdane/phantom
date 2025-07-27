#include <Areas.hpp>

namespace phantom {
  void ExprArea::add(Expr& expr) {
    exprs.emplace_back(expr);
    count++;
  }
  Expr ExprArea::get(ExprRef expr_ref) const {
    return exprs[expr_ref];
  }
  void ExprArea::init() {
    exprs.emplace_back(Expr{ .kind = ExprKind::Invalid, .data = { .invalid = {} } });
    count++;
  }
  void ExprArea::dump() {
    exprs.clear();
    count = 0;
  }

  void StmtArea::add(Stmt& stmt) {
    stmts.emplace_back(stmt);
    count++;
  }
  Stmt StmtArea::get(StmtRef stmt_ref) const{
    return stmts[stmt_ref];
  }
  void StmtArea::init() {
    stmts.emplace_back(Stmt{ .kind = StmtKind::Invalid, .data = { .invalid = {} } });
    count++;
  }
  void StmtArea::dump() {
    stmts.clear();
    count = 0;
  }
} // namespace phantom
