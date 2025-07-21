#include <ast/Statement.hpp>

namespace phantom {
  Statement::~Statement() = default;

  ExprInfo ReturnStt::accept(Visitor* visitor) { return visitor->visit(this); }

  ExprStt::ExprStt(std::unique_ptr<Expression> expr) : expr(std::move(expr)) {}
  ExprInfo ExprStt::accept(Visitor* visitor) { return visitor->visit(this); }

  VarDecStt::VarDecStt(Variable variable, std::unique_ptr<Expression> initializer)
    : variable(variable), initializer(std::move(initializer)) {}
  ExprInfo VarDecStt::accept(Visitor* visitor) { return visitor->visit(this); }

  FnDefStt::FnDefStt(std::unique_ptr<FnDecStt> declaration,
                     std::vector<std::unique_ptr<Statement>> body)
      : declaration(std::move(declaration)), body(std::move(body)) {}
  ExprInfo FnDefStt::accept(Visitor* visitor) { return visitor->visit(this); }

  FnDecStt::FnDecStt(std::string name, std::string type,
                     std::vector<std::unique_ptr<VarDecStt>> params)
      : name(name), type(type), params(std::move(params)) {}
  ExprInfo FnDecStt::accept(Visitor* visitor) { return visitor->visit(this); }
} // namespace phantom
