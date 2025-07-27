#pragma once

#include "data/Function.hpp"
#include <utils/StrUtils.hpp>
#include <Areas.hpp>
#include <map>

namespace phantom {
  namespace codegen {
    class Codegen {
  public:
      Codegen(std::vector<StmtRef>& ast, ExprArea& expr_area, StmtArea& stmt_area)
          : ast(ast), expr_area(expr_area), stmt_area(stmt_area), output(str::init()) {}

      void codegen();

  private:
      std::vector<StmtRef>& ast;
      ExprArea& expr_area;
      StmtArea& stmt_area;

      str::Str output;

      std::map<std::string, Variable> vars_table;
      std::map<std::string, Function> fns_table;

  private:
      void generate_function(Stmt& stmt);
    };
  } // namespace codegen
} // namespace phantom
