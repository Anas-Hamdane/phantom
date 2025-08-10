#pragma once

#include "data/Variable.hpp"
#include "irgen/Program.hpp"
#include <unordered_map>
#include <array>
#include <utils/str.hpp>

namespace phantom {
  namespace codegen {
    struct Directive {
      std::variant<long, float, double, std::string> data;
      enum class Kind {
        Long,
        Float,
        Double,
        Asciz
      } kind;
    };

    struct DataLabel {
      std::string name;
      std::vector<Directive> dirs;
    };

    class Gen {
  public:
      Gen(ir::Program& program)
          : program(program) {}

      const char* gen();

  private:
      ir::Program& program;
      utils::Str output;

      std::unordered_map<uint, Variable> scope_vars;
      std::unordered_map<float, DataLabel> floats_data;
      std::unordered_map<double, DataLabel> doubles_data;

      // NOTE: the first three registers are used as a mirror of the IR three
      // physical registers, the fourth one is use in case we need a temporary
      // register that we should use only in one instruction generation so it won'try
      // get overrided from other instructions.
      std::array<const char*, 4> integer_registers = { "rax", "rcx", "rdx", "rsi" };
      std::array<const char*, 4> float_registers = { "xmm0", "xmm1", "xmm2", "xmm3" };
      const size_t TR_INDEX = 3; // the temporary register index

      size_t constants_size = 0;
      // to track stack size
      size_t offset = 0;

  private:
      void generate_function(ir::Function& fn);
      void generate_instruction(ir::Instruction& inst);

      void generate_terminator(ir::Terminator& term, ir::Type& return_type);
      void generate_default_terminator(ir::Type& type);

      void generate_data();
      DataLabel constant_label(std::variant<double, std::string> value, Directive::Kind kind);

      void store_constant_in_memory(ir::Constant& constant, ir::VirtReg& memory);
      void store_register_in_memory(ir::PhysReg& reg, ir::VirtReg& memory);
      void store_memory_in_memory(ir::VirtReg& src, ir::VirtReg& dst);
      void store_constant_in_register(ir::Constant& constant, ir::PhysReg& reg);
      void store_register_in_register(ir::PhysReg& src, ir::PhysReg& dst);
      void store_memory_in_register(ir::VirtReg& memory, ir::PhysReg& reg);

      void add_constant_to_register(ir::Constant& constant, ir::PhysReg& reg);
      void add_register_to_register(ir::PhysReg& src, ir::PhysReg& dst);
      void add_memory_to_register(ir::VirtReg& memory, ir::PhysReg& reg);

      void sub_constant_from_register(ir::Constant& constant, ir::PhysReg& reg);
      void sub_register_from_register(ir::PhysReg& src, ir::PhysReg& dst);
      void sub_memory_from_register(ir::VirtReg& memory, ir::PhysReg& reg);

      void imul_constant_with_memory(ir::Constant& constant, ir::VirtReg& memory, ir::PhysReg& dst);
      void imul_constant_with_register(ir::Constant& constant, ir::PhysReg& reg, ir::PhysReg& dst);
      void imul_register_with_register(ir::PhysReg& src, ir::PhysReg& dst);
      void imul_memory_with_register(ir::VirtReg& memory, ir::PhysReg& reg);

      void mul_constant_with_register(ir::Constant& constant, ir::PhysReg& reg);
      void mul_register_with_register(ir::PhysReg& src, ir::PhysReg& dst);
      void mul_memory_with_register(ir::VirtReg& memory, ir::PhysReg& reg);

      char type_suffix(ir::Type& type);
      char integer_suffix(unsigned int size);
      char floating_point_suffix(unsigned int size);

      char* type_default_register(ir::Type& type);

      const char* physical_register_name(ir::PhysReg& pr);

      void generate_float_sign_mask_label();
      void generate_double_sign_mask_label();

      // remember to free the returned value
      char* generate_integer_move(ir::Type& src, ir::Type& dst);

      // the type of the other side rather than the xmm register
      // remember to free the returned value
      char* generate_floating_point_move(ir::Type& type);

      bool is_integer(ir::Type& type);
      bool is_float(ir::Type& type);
    };
  } // namespace codegen
} // namespace phantom
