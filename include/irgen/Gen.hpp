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

      std::unordered_map<std::string, Register> scope_vars;
      std::unordered_map<std::string, Function> funcs_table;

      uint nrid = 0; // next register id
      Function* current_function = nullptr;

      void define_function(std::unique_ptr<ast::FnDef>& ast_fn);
      void declare_function(std::unique_ptr<ast::FnDecl>& ast_fn);
      void generate_stmt(std::unique_ptr<ast::Stmt>& stmt);
      void generate_return(std::unique_ptr<ast::Return>& ast_rt);

      Value generate_expr(std::unique_ptr<ast::Expr>& expr);
      void create_store(Register dst, Value src);
      Register allocate_register(Type type);
    };
  } // namespace ir
} // namespace phantom
