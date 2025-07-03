#ifndef PHANTOM_VARIABLE_HPP
#define PHANTOM_VARIABLE_HPP

#include <llvm/IR/Value.h>

namespace phantom {
  class Variable {
    llvm::Value* alloca_inst = nullptr;
    llvm::Type* llvm_type = nullptr;

    std::string name;
    std::string type;

    bool pointer = false;
    bool global = false;

public:
    Variable();
    Variable(std::string name, std::string type,
             llvm::Value* value = nullptr, llvm::Type* llvm_type = nullptr,
             bool global = false, bool pointer = false);

    bool is_pointer() const;
    bool is_global() const;

    std::string get_name() const;
    std::string get_type() const;

    llvm::Value* get_alloca() const;
    llvm::Type* get_llvm_type() const;

    void set_pointer(bool pointer);
    void set_global(bool global);

    void set_name(std::string name);
    void set_type(std::string type);

    void set_alloca(llvm::Value* value);
    void set_llvm_type(llvm::Type* llvm_type);
  };
} // namespace phantom

#endif // !PHANTOM_VARIABLE_HPP
