#pragma once

#include "Program.hpp"
#include "ast/Stmt.hpp"
#include <map>

namespace phantom {
  namespace ir {
    class Gen {
  public:
      Gen(utils::Vec<Stmt>& ast)
          : ast(ast) {}

      Program gen();

  private:
      utils::Vec<Stmt>& ast;
      Program program; // output

      std::map<const char*, Register> local_vars;
      std::map<const char*, Function> funcs_table;

      uint nrid = 0; // next register id
      uint nbid = 0; // next block id

      Function* current_function = nullptr;
      BasicBlock* current_block = nullptr;

      void generate_stmt(Stmt& stmt);
      void generate_function(phantom::FnDef& ast_fn);
      void generate_return(phantom::Return& ast_rt);
      void declare_function(phantom::FnDecl& ast_fn);

      Value generate_expr(Expr& expr);
      Register allocate(Type type);

      Type proper_int_type(uint64_t value);
      Type proper_float_type(double value);
      Type binop_type(Value& lhs, Value& rhs);
      Type value_type(Value& operand);
      BinOp::Op binop_op(Token::Kind op);
      UnOp::Op unop_op(Token::Kind op);
    };
  } // namespace ir
} // namespace phantom
