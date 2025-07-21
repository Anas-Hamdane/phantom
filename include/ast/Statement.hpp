#ifndef PHANTOM_STATEMENT_HPP
#define PHANTOM_STATEMENT_HPP

#include "Expression.hpp"

namespace phantom {
  class Statement {
public:
    virtual ~Statement();
    virtual Value gen(Visitor* visitor) = 0;
  };

  class ReturnStt : public Statement {
public:
    std::unique_ptr<Expr> expr;

    explicit ReturnStt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
    Value gen(Visitor* visitor) override;
  };

  class ExprStt : public Statement {
public:
    std::unique_ptr<Expr> expr;

    explicit ExprStt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
    Value gen(Visitor* visitor) override;
  };

  class FnDecStt : public Statement {
public:
    std::string name;
    std::string type;
    std::vector<std::unique_ptr<VarDecExpr>> params;

    FnDecStt(const std::string& name, const std::string& type,
             std::vector<std::unique_ptr<VarDecExpr>> params)
      : name(name), type(type), params(std::move(params)) {}
    Value gen(Visitor* visitor) override;
  };

  class FnDefStt : public Statement {
public:
    std::unique_ptr<FnDecStt> declaration;
    std::vector<std::unique_ptr<Statement>> body;

    FnDefStt(std::unique_ptr<FnDecStt> declaration,
             std::vector<std::unique_ptr<Statement>> body)
      : declaration(std::move(declaration)), body(std::move(body)) {}
    Value gen(Visitor* visitor) override;
  };

} // namespace phantom

#endif // !PHANTOM_STATEMENT_HPP
