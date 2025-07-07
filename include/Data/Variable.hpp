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
    llvm::Type* ptr_to_type;

    // variable name
    std::string name;
    std::string raw_type; // in parsing stage

    // flag
    bool global;

    Variable(std::string name, llvm::Value* value = nullptr,
             llvm::Type* type = nullptr, llvm::Type* ptr_to_type = nullptr, bool global = false)
      : name(name), value(value), type(type), ptr_to_type(ptr_to_type), global(global) {}

    Variable(std::string name, std::string raw_type) : name(name), raw_type(raw_type) {}

    Variable() = default;
  };
} // namespace phantom

#endif // !PHANTOM_VARIABLE_HPP
