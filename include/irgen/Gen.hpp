#pragma once

#include "Program.hpp"
#include "ast/Stmt.hpp"
#include <unordered_map>

namespace phantom {
  namespace ir {
    class Gen {
  public:
      Gen(std::vector<std::unique_ptr<ast::Stmt>>& ast)
          : ast(ast) {}

      Program gen();

  private:
      std::vector<std::unique_ptr<ast::Stmt>>& ast;
      Program program; // output

      std::unordered_map<std::string, VirtReg> scope_vars;
      std::unordered_map<std::string, Function> funcs_table;

      // last used BinOp physical register, 'A' -> "rax", 'C' -> "rcx"
      char lubpr = 'C';
      // last used BinOp floating point physical register, '0' -> "xmm0", '1' -> "xmm1"
      char lubfppr = '1';

      uint nrid = 0; // next register id
      Function* current_function = nullptr;

      void define_function(std::unique_ptr<ast::FnDef>& ast_fn);
      void declare_function(std::unique_ptr<ast::FnDecl>& ast_fn);
      void generate_stmt(std::unique_ptr<ast::Stmt>& stmt);
      void generate_return(std::unique_ptr<ast::Return>& ast_rt);

      void create_store(std::variant<VirtReg, PhysReg> dst, Value src);

      Value generate_expr(std::unique_ptr<ast::Expr>& expr);
      VirtReg allocate_vritual_register(Type& type);
      PhysReg allocate_physical_register(Type& type);

      Type value_type(Value& value);
      Type resolve_type(phantom::Type& type);

      char* subreg_name(const std::string& reg, size_t size);
    };
  } // namespace ir
} // namespace phantom
