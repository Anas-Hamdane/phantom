#ifndef PHANTOM_STATEMENT_HPP
#define PHANTOM_STATEMENT_HPP

#include "Expression.hpp"

namespace phantom {
  class Statement {
public:
    virtual ~Statement();
    virtual ExprInfo accept(Visitor* visitor) = 0;
  };

  class ReturnStt : public Statement {
public:
    std::unique_ptr<Expression> expr;

    explicit ReturnStt(std::unique_ptr<Expression> expr) : expr(std::move(expr)) {}
    ExprInfo accept(Visitor* visitor) override;
  };

  class ExprStt : public Statement {
public:
    std::unique_ptr<Expression> expr;

    explicit ExprStt(std::unique_ptr<Expression> expr);
    ExprInfo accept(Visitor* visitor) override;
  };

  class VarDecStt : public Statement {
public:
    Variable variable;
    std::unique_ptr<Expression> initializer;

    VarDecStt(Variable variable, std::unique_ptr<Expression> initializer);
    ExprInfo accept(Visitor* visitor) override;
    ExprInfo global_var_dec();
    ExprInfo local_var_dec();
  };

  class ArrDecStt : public Statement {
public:
    Variable variable;
    std::unique_ptr<Expression> initializer;

    ArrDecStt(Variable variable, std::unique_ptr<Expression> initializer);
    ExprInfo accept(Visitor* visitor) override;
    ExprInfo global_var_dec();
    ExprInfo local_var_dec();
  };

  class FnDecStt : public Statement {
public:
    std::string name;
    std::string type;
    std::vector<std::unique_ptr<VarDecStt>> params;

    FnDecStt(std::string name, std::string type,
             std::vector<std::unique_ptr<VarDecStt>> params);
    ExprInfo accept(Visitor* visitor) override;
  };

  class FnDefStt : public Statement {
public:
    std::unique_ptr<FnDecStt> declaration;
    std::vector<std::unique_ptr<Statement>> body;

    FnDefStt(std::unique_ptr<FnDecStt> declaration,
             std::vector<std::unique_ptr<Statement>> body);
    ExprInfo accept(Visitor* visitor) override;
  };

} // namespace phantom

#endif // !PHANTOM_STATEMENT_HPP
