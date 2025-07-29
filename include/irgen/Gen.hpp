#pragma once

#include <Areas.hpp>
#include <vector>
#include <map>
#include "Instruction.hpp"

namespace phantom {
  namespace irgen {
    class Gen {
  public:
      Gen(std::vector<StmtRef>& ast, StmtArea& stmt_area, ExprArea& expr_area)
          : ast(ast), stmt_area(stmt_area), expr_area(expr_area) {}

      std::vector<Instruction> gen();
  private:
      std::vector<StmtRef>& ast;
      StmtArea& stmt_area;
      ExprArea& expr_area;

      std::vector<Instruction> output;

      std::map<const char*, Register> variables;

      void declare_function(phantom::FnDecl fn);
    };
  } // namespace irgen
} // namespace phantom
