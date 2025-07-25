#pragma once

#include "Expr.hpp"

namespace phantom {
  class Stmt {
public:
    virtual ~Stmt();
    virtual AstElm represent() = 0;
  };

  class RetStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;

    explicit RetStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}

    AstElm represent() = 0;
  };

  class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;

    explicit ExprStmt(std::unique_ptr<Expr> expr);

    AstElm represent() = 0;
  };

  class FnDecStmt : public Stmt {
public:
    std::string name;
    std::unique_ptr<DataTypeExpr> type;
    std::vector<std::unique_ptr<VarDecExpr>> params;

    FnDecStmt(const std::string& name, std::unique_ptr<DataTypeExpr> type, std::vector<std::unique_ptr<VarDecExpr>> params)
        : name(name), type(std::move(type)), params(std::move(params)) {}

    AstElm represent() = 0;
  };

  class FnDefStmt : public Stmt {
public:
    std::unique_ptr<FnDecStmt> declaration;
    std::vector<std::unique_ptr<Stmt>> body;

    FnDefStmt(std::unique_ptr<FnDecStmt> declaration, std::vector<std::unique_ptr<Stmt>> body)
        : declaration(std::move(declaration)), body(std::move(body)) {}

    AstElm represent() = 0;
  };

} // namespace phantom
