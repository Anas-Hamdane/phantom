#pragma once

#include "data/Function.hpp"
#include <Areas.hpp>
#include <map>
#include <utils/str.hpp>

namespace phantom {
  namespace codegen {
    class Codegen {
  public:
      Codegen(std::vector<StmtRef>& ast, ExprArea& expr_area, StmtArea& stmt_area)
          : ast(ast), expr_area(expr_area), stmt_area(stmt_area), output(str::init()) {}

      const char* codegen();

  private:
      std::vector<StmtRef>& ast;
      ExprArea& expr_area;
      StmtArea& stmt_area;

      str::Str output;

      std::map<std::string, Variable> vars_table;
      std::map<std::string, Function> fn_table;

      Function* current_function;

      // to track stack size
      size_t offset = 0;

  private:
      void generate_stmt(Stmt& stmt);
      void generate_function(FnDef& fn);
      void generate_return(Return& ret);
      void generate_expression(Expmt& expmt);

      void generate_vardecl(VarDecl& vardecl);
      void generate_binop(BinOp& binop);

      void declare_function(FnDecl& decl);
      void declare_variable(VarDecl& decl);

      void load_to_reg(const char* reg, Expr& expr);
      // void mov_to(const char* dst, Expr& expr);

      // void add();
      // void sub();
      // void mul();
      // void div();
      void asgn(ExprRef left_ref, ExprRef right_ref);
      // void neg();

      char resolve_suffix(VarType type);
      char* resolve_reg(VarType type);
    };
  } // namespace codegen
} // namespace phantom
