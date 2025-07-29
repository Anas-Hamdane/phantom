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
      const std::map<std::string_view, unsigned int> regs_table{
        { "rax", 8 },
        { "eax", 4 },
        { "ax", 2 },
        { "al", 1 },

        { "rbx", 8 },
        { "ebx", 4 },
        { "bx", 2 },
        { "bl", 1 },

        { "rcx", 8 },
        { "ecx", 4 },
        { "cx", 2 },
        { "cl", 1 },

        { "rdx", 8 },
        { "edx", 4 },
        { "dx", 2 },
        { "dl", 1 },

        { "rsp", 8 },
        { "esp", 4 },
        { "sp", 2 },
        { "spl", 1 },

        { "rsi", 8 },
        { "esi", 4 },
        { "si", 2 },
        { "sil", 1 },

        { "rdi", 8 },
        { "edi", 4 },
        { "di", 2 },
        { "dil", 1 },

        { "rbp", 8 },
        { "ebp", 4 },
        { "bp", 2 },
        { "bpl", 1 },

        { "r8", 8 },
        { "r8d", 4 },
        { "r8w", 2 },
        { "r8b", 1 },

        { "r9", 8 },
        { "r9d", 4 },
        { "r9w", 2 },
        { "r9b", 1 },

        { "r10", 8 },
        { "r10d", 4 },
        { "r10w", 2 },
        { "r10b", 1 },

        { "r11", 8 },
        { "r11d", 4 },
        { "r11w", 2 },
        { "r11b", 1 },

        { "r12", 8 },
        { "r12d", 4 },
        { "r12w", 2 },
        { "r12b", 1 },

        { "r13", 8 },
        { "r13d", 4 },
        { "r13w", 2 },
        { "r13b", 1 },

        { "r14", 8 },
        { "r14d", 4 },
        { "r14w", 2 },
        { "r14b", 1 },

        { "r15", 8 },
        { "r15d", 4 },
        { "r15w", 2 },
        { "r15b", 1 }
      };

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
      void store_value(int64_t value, Expr& expr);
      // void mov_to(const char* dst, Expr& expr);

      void add(ExprRef left_ref, ExprRef right_ref);
      // void sub();
      // void mul();
      // void div();
      void asgn(ExprRef left_ref, ExprRef right_ref);
      // void neg();

      char size_suffix(unsigned int size);
      char* size_areg(unsigned int size);

      void check_identifier(const char* name);
    };
  } // namespace codegen
} // namespace phantom
