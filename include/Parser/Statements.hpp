#ifndef PHANTOM_STATEMENTS_HPP
#define PHANTOM_STATEMENTS_HPP

#include <Parser/Expressions.hpp>
#include <Data/Variable.hpp>
#include <global.hpp>

namespace phantom {
  class Parameter {
public:
    std::string name;
    std::string type;

    Parameter(std::string name, std::string type);
  };

  class Statement {
public:
    virtual ~Statement();
    virtual llvm::Value* accept(Visitor* visitor) = 0;
  };

  class ReturnStt : public Statement {
public:
    std::unique_ptr<Expression> expr;

    explicit ReturnStt(std::unique_ptr<Expression> expr);
    llvm::Value* accept(Visitor* visitor) override;
  };

  class ExprStt : public Statement {
public:
    std::unique_ptr<Expression> expr;

    explicit ExprStt(std::unique_ptr<Expression> expr);
    llvm::Value* accept(Visitor* visitor) override;
  };

  class VarDecStt : public Statement {
public:
    Variable variable;
    std::unique_ptr<Expression> initializer;

    VarDecStt(Variable variable, std::unique_ptr<Expression> initializer);
    llvm::Value* accept(Visitor* visitor) override;
    llvm::Value* global_var_dec();
    llvm::Value* local_var_dec();
  };

  class FnDecStt : public Statement {
public:
    std::string name;
    std::string type;
    std::vector<Parameter> params;

    FnDecStt(std::string name, std::string type,
             std::vector<Parameter> params);
    llvm::Function* accept(Visitor* visitor) override;
  };

  class FnDefStt : public Statement {
public:
    std::unique_ptr<FnDecStt> declaration;
    std::vector<std::unique_ptr<Statement>> body;

    FnDefStt(std::unique_ptr<FnDecStt> declaration,
             std::vector<std::unique_ptr<Statement>> body);
    llvm::Function* accept(Visitor* visitor) override;
  };

} // namespace phantom

#endif // !PHANTOM_STATEMENTS_HPP
