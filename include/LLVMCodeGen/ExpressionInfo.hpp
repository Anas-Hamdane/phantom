#ifndef PHANTOM_EXPRESSION_INFO_HPP
#define PHANTOM_EXPRESSION_INFO_HPP

#include <Data/Variable.hpp>

namespace phantom {
  struct ExpressionInfo {
    llvm::Value* value;
    llvm::Type* type;

    Variable* variable;

    ExpressionInfo(llvm::Value* value = nullptr, llvm::Type* type = nullptr, Variable* variable = nullptr)
      : value(value), type(type), variable(variable) {}
  };
}

#endif // !PHANTOM_EXPRESSION_INFO_HPP
