#pragma once

#include "Instruction.hpp"
#include "ast/Stmt.hpp"
#include <map>

namespace phantom {
  namespace irgen {
    class Gen {
  public:
      Gen(vec::Vec<Stmt>& ast)
          : ast(ast) {}

      vec::Vec<Instruction> gen();

  private:
      vec::Vec<Stmt>& ast;
      std::map<const char*, Register> variables;

      Instruction generate_stmt(Stmt& stmt);
      Instruction declare_function(phantom::FnDecl fn);
    };
  } // namespace irgen
} // namespace phantom
