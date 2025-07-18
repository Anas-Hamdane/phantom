#ifndef PHANTOM_VARIABLE_HPP
#define PHANTOM_VARIABLE_HPP

#include <string>

namespace llvm {
  class Value;
  class Type;
} // namespace llvm

namespace phantom {
  namespace llvm_codegen {
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

      Variable(const std::string& name, bool global = false, llvm::Value* value = nullptr,
               llvm::Type* type = nullptr, Variable* ptr_to_type = nullptr)
          : name(name), global(global), value(value), type(type), ptr_to_variable(ptr_to_type) {}

      Variable() = default;
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_VARIABLE_HPP
