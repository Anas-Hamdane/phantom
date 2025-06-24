#ifndef PHANTOM_STATEMENTS_HPP
#define PHANTOM_STATEMENTS_HPP

#include "expression.hpp"
#include "parameter.hpp"
#include <memory>
#include <vector>

namespace phantom {
  class Statement {
public:
    virtual ~Statement();
  };

  class ReturnStatement : public Statement {
    std::unique_ptr<Expression> expression;

public:
    ReturnStatement(std::unique_ptr<Expression> expression);
  };

  class VariableStatement : public Statement {
    std::string name;
    std::string type;
    std::unique_ptr<Expression> initializer;

public:
    VariableStatement(std::string name, std::string type, std::unique_ptr<Expression> initializer);
  };

  // Function Declaration Statement
  class FnDecStatement : public Statement {
    std::string name;
    std::string type;
    std::vector<Parameter> params;

public:
    FnDecStatement(std::string name, std::string type,
        std::vector<Parameter> params);
  };

  // Function Definition Statement
  class FnDefStatement : public Statement {
    std::unique_ptr<FnDecStatement> declaration;
    std::vector<std::unique_ptr<Statement>> body;

public:
    FnDefStatement(std::unique_ptr<FnDecStatement> declaration,
        std::vector<std::unique_ptr<Statement>> body);
  };
} // namespace phantom

#endif // !PHANTOM_STATEMENTS_HPP
