#ifndef PHANTOM_VARIABLE_HPP
#define PHANTOM_VARIABLE_HPP

#include <llvm/IR/Value.h>

namespace phantom {
  struct Variable {
    // llvm::AllocaInst or llvm::GlobalVariable
    llvm::Value* value;

    // variable type
    llvm::Type* type;

    // pointer-to type (for pointers)
    Variable* ptr_to_variable;

    // variable name
    std::string name;

    // flag
    bool global;

    Variable(std::string name, llvm::Value* value = nullptr,
             llvm::Type* type = nullptr, Variable* ptr_to_type = nullptr, bool global = false)
      : name(name), value(value), type(type), ptr_to_variable(ptr_to_type), global(global) {}

    Variable() = default;
  };
} // namespace phantom

#endif // !PHANTOM_VARIABLE_HPP
