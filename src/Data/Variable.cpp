#include <Data/Variable.hpp>

namespace phantom {
  Variable::Variable() = default;
  Variable::Variable(std::string name, std::string type,
                     llvm::Value* value, llvm::Type* llvm_type,
                     bool global, bool pointer)
      : name(name), type(type), alloca_inst(value), llvm_type(llvm_type), global(global), pointer(pointer) {}

  bool Variable::is_pointer() const { return pointer; }
  bool Variable::is_global() const { return global; }

  std::string Variable::get_name() const { return name; }
  std::string Variable::get_type() const { return type; }

  llvm::Value* Variable::get_alloca() const { return alloca_inst; }
  llvm::Type* Variable::get_llvm_type() const { return llvm_type; }

  void Variable::set_pointer(bool pointer) { this->pointer = pointer; }
  void Variable::set_global(bool global) { this->global = global; }

  void Variable::set_name(std::string name) { this->name = name; }
  void Variable::set_type(std::string type) { this->type = type; }

  void Variable::set_alloca(llvm::Value* value) { this->alloca_inst = value; }
  void Variable::set_llvm_type(llvm::Type* llvm_type) { this->llvm_type = llvm_type; }
} // namespace phantom
