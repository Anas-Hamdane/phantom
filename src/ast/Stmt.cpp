#include <ast/Stmt.hpp>

namespace phantom {
  AstElm RetStmt::represent() const {
    return AstElm{"RetStmt: \n", {expr->represent()}};
  }
  AstElm ExprStmt::represent() const {
    return AstElm{"ExprStmt: \n", {expr->represent()}};
  }
  AstElm FnDecStmt::represent() const {
    std::vector<AstElm> elms;

    if (type)
      elms.push_back(type->represent());

    for (auto& p : params)
      elms.push_back(p->represent());

    return AstElm{"FnDecStmt: " + name + "\n", elms};
  }
  AstElm FnDefStmt::represent() const {
    std::vector<AstElm> elms;

    if (declaration)
      elms.push_back(declaration->represent());

    for (auto& b : body)
      elms.push_back(b->represent());

    return AstElm{"FnDefStmt: \n", elms};
  }
} // namespace phantom
