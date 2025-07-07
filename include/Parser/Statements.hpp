#ifndef PHANTOM_STATEMENTS_HPP
#define PHANTOM_STATEMENTS_HPP

#include <Parser/Expressions.hpp>
#include <Data/Variable.hpp>
#include <global.hpp>

namespace phantom {
  class Statement {
public:
    virtual ~Statement();
    virtual ExpressionInfo accept(Visitor* visitor) = 0;
  };

  class ReturnStt : public Statement {
public:
    std::unique_ptr<Expression> expr;

    explicit ReturnStt(std::unique_ptr<Expression> expr);
    ExpressionInfo accept(Visitor* visitor) override;
  };

  class ExprStt : public Statement {
public:
    std::unique_ptr<Expression> expr;

    explicit ExprStt(std::unique_ptr<Expression> expr);
    ExpressionInfo accept(Visitor* visitor) override;
  };

  class VarDecStt : public Statement {
public:
    Variable variable;
    std::unique_ptr<Expression> initializer;

    VarDecStt(Variable variable, std::unique_ptr<Expression> initializer);
    ExpressionInfo accept(Visitor* visitor) override;
    ExpressionInfo global_var_dec();
    ExpressionInfo local_var_dec();
  };

  class FnDecStt : public Statement {
public:
    std::string name;
    std::string type;
    std::vector<Variable> params;

    FnDecStt(std::string name, std::string type,
             std::vector<Variable> params);
    ExpressionInfo accept(Visitor* visitor) override;
  };

  class FnDefStt : public Statement {
public:
    std::unique_ptr<FnDecStt> declaration;
    std::vector<std::unique_ptr<Statement>> body;

    FnDefStt(std::unique_ptr<FnDecStt> declaration,
             std::vector<std::unique_ptr<Statement>> body);
    ExpressionInfo accept(Visitor* visitor) override;
  };

} // namespace phantom

#endif // !PHANTOM_STATEMENTS_HPP
