#include <codegen/Codegen.hpp>
#include <cstring>

namespace phantom {
  namespace codegen {
    const char* Gen::gen() {
      output = utils::init();
      utils::append(&output, ".section .text\n");

      for (ir::Function& fn : program.funcs) {
        generate_function(fn);
        utils::append(&output, "\n");
      }

      return output.content;
    }

    void Gen::generate_function(ir::Function& fn) {
      const char* name = fn.name.c_str();

      utils::appendf(&output, "# begin function @%s\n", name);
      utils::appendf(&output, ".globl %s\n", name);
      utils::append(&output, ".p2align 4\n");
      utils::appendf(&output, ".type %s, @function\n", name);
      utils::appendf(&output, "%s:\n", name, name);

      utils::append(&output, "  pushq   %rbp\n");
      utils::append(&output, "  movq    %rsp, %rbp\n");

      const char* regs[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
      const size_t regs_size = 6;

      size_t params_size = fn.params.size();
      size_t tmp = std::min(regs_size, params_size);
      for (size_t i = 0; i < tmp; ++i) {
        ir::Register param = fn.params.at(i);
        size_t size = (param.type.bitwidth == 0) ? 1 : (param.type.bitwidth / 8);

        const char suff = size_suffix(size);
        const char* reg = subreg_name(regs[i], size);

        utils::appendf(&output, "  mov%c    %%%s, -%zu(%%rbp)\n", suff, reg, offset + size);
        offset += size;

        local_vars[param.id] = Variable{ .type = param.type, .offset = offset };
      }

      for (size_t i = tmp; i < params_size; ++i) {
        ir::Register param = fn.params.at(i);
        size_t size = (param.type.bitwidth == 0) ? 1 : (param.type.bitwidth / 8);

        const char suff = size_suffix(size);
        const char* reg = size_areg(size);

        utils::appendf(&output, "  mov%c    %zu(%%rbp), %%%s\n", suff, ((i - tmp) + 2) * 8, reg);
        utils::appendf(&output, "  mov%c    %%%s, -%zu(%%rbp)\n", suff, reg, offset + size);
        offset += size;

        local_vars[param.id] = Variable{ .type = param.type, .offset = offset };
      }

      for (ir::Instruction& inst : fn.body)
        generate_instruction(inst);

      if (fn.terminated)
        generate_terminator(fn.terminator);
      else
        generate_default_terminator(fn.return_type);

      utils::appendf(&output, "# end function @%s\n", name);
    }
    void Gen::generate_instruction(ir::Instruction& inst) {
      switch (inst.index()) {
        case 0: // Alloca
        {
          ir::Alloca& alloca = std::get<0>(inst);
          offset += (alloca.type.bitwidth == 0) ? 1 : (alloca.type.bitwidth / 8);
          local_vars[alloca.reg.id] = Variable{ .type = alloca.type, .offset = offset };
          break;
        }
        case 1: // Store
        {
          ir::Store& store = std::get<1>(inst);
          Variable dst = local_vars[store.dst.id];

          uint dst_size = (dst.type.bitwidth == 0) ? 1 : (dst.type.bitwidth / 8);
          const char dst_suff = size_suffix(dst_size);

          switch (store.src.index()) {
            case 0: // Register
            {
              Variable src = local_vars[std::get<0>(store.src).id];

              // the intermediate register that we will move to and from
              const char* reg = size_areg(dst_size);
              uint reg_size = dst_size;

              uint src_size = (src.type.bitwidth == 0) ? 1 : (src.type.bitwidth / 8);

              utils::Str mov = utils::init("mov");
              if (reg_size > src_size)
                utils::appendf(&mov, "s%c%c", size_suffix(src_size), size_suffix(reg_size));
              else
                utils::appendf(&mov, "%c", size_suffix(reg_size));

              utils::appendf(&output, "  %-7s -%zu(%%rbp), %%%s\n", mov.content, src.offset, reg);
              utils::appendf(&output, "  mov%c    %%%s, -%zu(%%rbp)\n", dst_suff, reg, dst.offset);
              break;
            }
            case 1: // Constant
            {
              ir::Constant constant = std::get<1>(store.src);
              switch (constant.value.index()) {
                case 0: // uint64_t
                {
                  uint64_t value = std::get<0>(constant.value);
                  utils::appendf(&output, "  mov%c    $%lu, -%zu(%%rbp)\n", dst_suff, value, dst.offset);
                  break;
                }
                case 1: // double
                {
                  // TODO:
                }
              }
              break;
            }
          }
          break;
        }

        case 2: // BinOp
        {
          // TODO:
        }
        case 3: // UnOp
        {
          // TODO:
        }
      }
    }

    void Gen::generate_terminator(ir::Terminator& term) {
      switch (term.index()) {
        case 0: // Return
        {
          ir::Return& ret = std::get<0>(term);

          switch (ret.value.index()) {
            case 0: // Register
            {
              Variable value = local_vars[std::get<0>(ret.value).id];
              uint value_size = (value.type.bitwidth == 1) ? 1 : (value.type.bitwidth / 8);

              const char suff = size_suffix(value_size);
              const char* reg = size_areg(value_size);

              utils::appendf(&output, "  mov%c    -%zu(%%rbp), %%%s\n", suff, value.offset, reg);
              break;
            }
            case 1: // Constant
            {
              ir::Constant& constant = std::get<1>(ret.value);
              uint constant_size = (constant.type.bitwidth == 1) ? 1 : (constant.type.bitwidth / 8);

              const char suff = size_suffix(constant_size);
              const char* reg = size_areg(constant_size);

              switch (constant.value.index()) {
                case 0: // uint64_t
                {
                  uint64_t value = std::get<0>(constant.value);
                  utils::appendf(&output, "  mov%c    $%lu, %%%s\n", suff, value, reg);
                  break;
                }
                case 1: // double
                {
                  // TODO:
                }
              }
            }
          }

          utils::appendf(&output, "  movq    %%rbp, %%rsp\n");
          utils::appendf(&output, "  popq    %%rbp\n");
          utils::appendf(&output, "  ret\n");
        }
      }
    }
    void Gen::generate_default_terminator(Type type) {
      switch (type.kind) {
        case Type::Kind::UnsInt:
        case Type::Kind::Void:
        case Type::Kind::Int: // void + unsigned/signed integers
          utils::appendf(&output, "  nop\n");
          break;
        case Type::Kind::FP: // floating points
        {
          uint value_size = (type.bitwidth == 1) ? 1 : (type.bitwidth / 8);

          const char suff = size_suffix(value_size);
          const char* reg = size_areg(value_size);

          utils::appendf(&output, "  mov%c    %%%s, %%xmm0\n", suff, reg);
        }
      }

      utils::appendf(&output, "  movq    %%rbp, %%rsp\n");
      utils::appendf(&output, "  popq    %%rbp\n");
      utils::appendf(&output, "  ret\n");
    }

    char Gen::size_suffix(unsigned int size) {
      // clang-format off
      switch (size) {
         case 1: return 'b'; 
         case 2: return 'w';
         case 4: return 'l'; 
         case 8: return 'q';
         default:
           printf("Undefined size suffix for: %u\n", size);
           exit(1);
      }
      // clang-format on
    }
    char* Gen::size_areg(unsigned int size) {
      // clang-format off
      switch (size) {
         case 1: return (char*)"al"; 
         case 2: return (char*)"ax";
         case 4: return (char*)"eax"; 
         case 8: return (char*)"rax";
         default:
           printf("Undefined size for A register: %u\n", size);
           exit(1);
      }
      // clang-format on
    }
    char* Gen::subreg_name(const char* reg, size_t size) {
      // clang-format off
      if (strcmp(reg, "rax") == 0) {
        switch (size) {
         case 1: return (char*)"al";
         case 2: return (char*)"ax";
         case 4: return (char*)"eax";
         case 8: return (char*)"rax";
        }
      } else if (strcmp(reg, "rbx") == 0) {
        switch (size) {
         case 1: return (char*)"bl";
         case 2: return (char*)"bx";
         case 4: return (char*)"ebx";
         case 8: return (char*)"rbx";
        }
      } else if (strcmp(reg, "rcx") == 0) {
        switch (size) {
         case 1: return (char*)"cl";
         case 2: return (char*)"cx";
         case 4: return (char*)"ecx";
         case 8: return (char*)"rcx";
        }
      } else if (strcmp(reg, "rdx") == 0) {
        switch (size) {
         case 1: return (char*)"dl";
         case 2: return (char*)"dx";
         case 4: return (char*)"edx";
         case 8: return (char*)"rdx";
        }
      } else if (strcmp(reg, "rsp") == 0) {
        switch (size) {
         case 1: return (char*)"spl";
         case 2: return (char*)"sp";
         case 4: return (char*)"esp";
         case 8: return (char*)"rsp";
        }
      } else if (strcmp(reg, "rbp") == 0) {
        switch (size) {
         case 1: return (char*)"bpl";
         case 2: return (char*)"bp";
         case 4: return (char*)"ebp";
         case 8: return (char*)"rbp";
        }
      } else if (strcmp(reg, "rsi") == 0) {
        switch (size) {
         case 1: return (char*)"sil";
         case 2: return (char*)"si";
         case 4: return (char*)"esi";
         case 8: return (char*)"rsi";
        }
      } else if (strcmp(reg, "rdi") == 0) {
        switch (size) {
         case 1: return (char*)"dil";
         case 2: return (char*)"di";
         case 4: return (char*)"edi";
         case 8: return (char*)"rdi";
        }
      } else if (strcmp(reg, "r8") == 0) {
        switch (size) {
         case 1: return (char*)"r8b";
         case 2: return (char*)"r8w";
         case 4: return (char*)"r8d";
         case 8: return (char*)"r8";
        }
      } else if (strcmp(reg, "r9") == 0) {
        switch (size) {
         case 1: return (char*)"r9b";
         case 2: return (char*)"r9w";
         case 4: return (char*)"r9d";
         case 8: return (char*)"r9";
        }
      } else if (strcmp(reg, "r10") == 0) {
        switch (size) {
         case 1: return (char*)"r10b";
         case 2: return (char*)"r10w";
         case 4: return (char*)"r10d";
         case 8: return (char*)"r10";
        }
      } else if (strcmp(reg, "r11") == 0) {
        switch (size) {
         case 1: return (char*)"r11b";
         case 2: return (char*)"r11w";
         case 4: return (char*)"r11d";
         case 8: return (char*)"r11";
        }
      } else if (strcmp(reg, "r12") == 0) {
        switch (size) {
         case 1: return (char*)"r12b";
         case 2: return (char*)"r12w";
         case 4: return (char*)"r12d";
         case 8: return (char*)"r12";
        }
      } else if (strcmp(reg, "r13") == 0) {
        switch (size) {
         case 1: return (char*)"r13b";
         case 2: return (char*)"r13w";
         case 4: return (char*)"r13d";
         case 8: return (char*)"r13";
        }
      } else if (strcmp(reg, "r14") == 0) {
        switch (size) {
         case 1: return (char*)"r14b";
         case 2: return (char*)"r14w";
         case 4: return (char*)"r14d";
         case 8: return (char*)"r14";
        }
      } else if (strcmp(reg, "r15") == 0) {
        switch (size) {
         case 1: return (char*)"r15b";
         case 2: return (char*)"r15w";
         case 4: return (char*)"r15d";
         case 8: return (char*)"r15";
        }
      }

      printf("Unrecognized register or register size: %s, %zu\n", reg, size);
      exit(1);
      return nullptr;
      // clang-format on
    }
  } // namespace codegen
} // namespace phantom
