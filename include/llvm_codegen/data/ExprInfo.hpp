#ifndef PHANTOM_EXPRESSION_INFO_HPP
#define PHANTOM_EXPRESSION_INFO_HPP

#include "Variable.hpp"

namespace phantom {
  namespace llvm_codegen {
    struct ExprInfo {
      llvm::Value* value;
      llvm::Type* type;

      Variable* variable;

      ExprInfo(llvm::Value* value = nullptr, llvm::Type* type = nullptr, Variable* variable = nullptr)
          : value(value), type(type), variable(variable) {}
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_EXPRESSION_INFO_HPP
