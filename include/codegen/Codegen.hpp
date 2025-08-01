#pragma once

#include "data/Variable.hpp"
#include "irgen/Program.hpp"
#include <map>
#include <string_view>
#include <unordered_map>
#include <utils/str.hpp>

namespace phantom {
  namespace codegen {
    class Gen {
  public:
      Gen(ir::Program& program)
          : program(program) {}

      const char* gen();

  private:
      ir::Program& program;
      utils::Str output;

      std::unordered_map<uint, Variable> local_vars;
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

      // to track stack size
      size_t offset = 0;

  private:
      void generate_function(ir::Function& fn);
      void generate_instruction(ir::Instruction& inst);

      void generate_terminator(ir::Terminator& term);
      void generate_default_terminator(Type type);

      // void mov_to(const char* dst, Expr& expr);

      // void add(ExprRef left_ref, ExprRef right_ref);
      // void sub();
      // void mul();
      // void div();
      // void asgn(ExprRef left_ref, ExprRef right_ref);
      // void neg();

      char size_suffix(unsigned int size);
      char* size_areg(unsigned int size);
      char* subreg_name(const char* reg, size_t size);
    };
  } // namespace codegen
} // namespace phantom
