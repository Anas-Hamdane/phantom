#include <utility>

#include "../../include/parser/statement.hpp"

namespace phantom {
  Statement::~Statement() = default;

  ReturnStatement::ReturnStatement(std::unique_ptr<Expression> expression)
      : expression(std::move(expression)) {}

  VariableStatement::VariableStatement(std::string name, std::string type,
                                       std::unique_ptr<Expression> initializer)
      : name(std::move(name)), type(std::move(type)), initializer(std::move(initializer)) {}

  FnDecStatement::FnDecStatement(std::string name, std::string type,
                                 std::vector<Parameter> params)
      : name(std::move(name)), type(std::move(type)), params(std::move(params)) {}

  FnDefStatement::FnDefStatement(std::unique_ptr<FnDecStatement> declaration,
                 std::vector<std::unique_ptr<Statement>> body)
    : declaration(std::move(declaration)), body(std::move(body)) {}

} // namespace phantom
