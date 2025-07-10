#include <LLVMCodeGen/Visitor.hpp>
#include <Parser/Statements.hpp>

namespace phantom {
  Statement::~Statement() = default;

  ReturnStt::ReturnStt(std::unique_ptr<Expression> expr)
      : expr(std::move(expr)) {}
  ExpressionInfo ReturnStt::accept(Visitor* visitor) { return visitor->visit(this); }

  ExprStt::ExprStt(std::unique_ptr<Expression> expr) : expr(std::move(expr)) {}
  ExpressionInfo ExprStt::accept(Visitor* visitor) { return visitor->visit(this); }

  VarDecStt::VarDecStt(Variable variable, std::unique_ptr<Expression> initializer)
    : variable(variable), initializer(std::move(initializer)) {}
  ExpressionInfo VarDecStt::accept(Visitor* visitor) { return visitor->visit(this); }

  FnDefStt::FnDefStt(std::unique_ptr<FnDecStt> declaration,
                     std::vector<std::unique_ptr<Statement>> body)
      : declaration(std::move(declaration)), body(std::move(body)) {}
  ExpressionInfo FnDefStt::accept(Visitor* visitor) { return visitor->visit(this); }

  FnDecStt::FnDecStt(std::string name, std::string type,
                     std::vector<Variable> params)
      : name(name), type(type), params(std::move(params)) {}
  ExpressionInfo FnDecStt::accept(Visitor* visitor) { return visitor->visit(this); }
} // namespace phantom
