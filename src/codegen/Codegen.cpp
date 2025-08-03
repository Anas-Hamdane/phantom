#include "codegen/Codegen.hpp"
#include "common.hpp"
#include <cstring>

namespace phantom {
  namespace codegen {
    const char* Gen::gen() {
      output = utils::init();
      utils::append(&output, ".section .text\n\n");

      for (ir::Function& fn : program.funcs) {
        generate_function(fn);
        utils::append(&output, "\n");
      }

      generate_data();

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
      offset = 0;

      const char* regs[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
      const size_t regs_size = 6;

      size_t params_size = fn.params.size();
      size_t tmp = std::min(regs_size, params_size);
      for (size_t i = 0; i < tmp; ++i) {
        ir::VirtReg param = fn.params.at(i);

        const char suff = integer_suffix(param.type.size);
        const char* reg = subreg_name(regs[i], param.type.size);

        utils::appendf(&output, "  mov%c    %%%s, -%zu(%%rbp)\n", suff, reg, offset + param.type.size);
        offset += param.type.size;

        local_vars[param.id] = Variable{ .type = param.type, .offset = offset };
      }

      for (size_t i = tmp; i < params_size; ++i) {
        ir::VirtReg param = fn.params.at(i);

        const char suff = integer_suffix(param.type.size);
        const char* reg = size_areg(param.type.size);

        utils::appendf(&output, "  mov%c    %zu(%%rbp), %%%s\n", suff, ((i - tmp) + 2) * 8, reg);
        utils::appendf(&output, "  mov%c    %%%s, -%zu(%%rbp)\n", suff, reg, offset + param.type.size);
        offset += param.type.size;

        local_vars[param.id] = Variable{ .type = param.type, .offset = offset };
      }

      for (ir::Instruction& inst : fn.body)
        generate_instruction(inst);

      if (fn.terminated)
        generate_terminator(fn.terminator, fn.return_type);
      else
        generate_default_terminator(fn.return_type);

      utils::appendf(&output, "# end function @%s\n", name);
    }
    void Gen::generate_instruction(ir::Instruction& inst) {
      switch (inst.index()) {
        case 0: // Alloca
        {
          ir::Alloca& alloca = std::get<0>(inst);
          offset += alloca.type.size;
          local_vars[alloca.reg.id] = Variable{ .type = alloca.type, .offset = offset };
          break;
        }
        case 1: // Store
        {
          ir::Store& store = std::get<1>(inst);
          utils::Str dst = utils::init(4);
          ir::Type dst_type;
          char dst_suff;

          switch (store.dst.index()) {
            case 0: // VirtReg
            {
              Variable var = local_vars[std::get<0>(store.dst).id];
              dst_suff = type_suffix(var.type);
              dst_type = var.type;
              utils::appendf(&dst, "-%zu(%%rbp)", var.offset);
              break;
            }
            case 1: // PhysReg
            {
              ir::PhysReg phy = std::get<1>(store.dst);
              dst_suff = type_suffix(phy.type);
              dst_type = phy.type;
              utils::appendf(&dst, "%%%s", phy.name.c_str());
              break;
            }
            default:
              unreachable();
          }

          switch (store.src.index()) {
            case 0: // Constant
            {
              ir::Constant constant = std::get<0>(store.src);

              switch (constant.value.index()) {
                case 0: // int64_t
                {
                  int64_t value = std::get<0>(constant.value);
                  utils::appendf(&output, "  mov%c    $%ld, %s\n", dst_suff, value, dst.content);
                  break;
                }
                case 1: // double
                {
                  double value = std::get<1>(constant.value);
                  if (value == 0) {
                    utils::append(&output, "  pxor    %xmm0, %xmm0\n");
                    utils::appendf(&output, "  movs%c    %%xmm0, %s\n", dst_suff, dst.content);
                    break;
                  }

                  DataLabel label;
                  if (const_fps.find(value) == const_fps.end()) {
                    Directive::Kind kind;

                    if (dst_type.size == 4)
                      kind = Directive::Kind::Float;
                    else if (dst_type.size == 8)
                      kind = Directive::Kind::Double;
                    else
                      unreachable();

                    label.name = ".CFPS" + std::to_string(const_fps.size());
                    label.dirs.push_back(Directive{ .kind = kind, .data = value });
                    const_fps[value] = label;
                  }

                  // already exist
                  else
                    label = const_fps[value];

                  utils::appendf(&output, "  movs%c   %s(%%rip), %%xmm0\n", dst_suff, label.name.c_str());
                  utils::appendf(&output, "  movs%c   %%xmm0, %s\n", dst_suff, dst.content);
                  break;
                }
                default:
                  unreachable();
              };

              break;
            }
            case 1: // VirtReg
            {
              Variable src = local_vars[std::get<1>(store.src).id];

              // the intermediate register that we will move to and from
              uint dst_size = dst_type.size;
              uint src_size = src.type.size;
              utils::Str reg = utils::init(5);

              if (src.type.kind == ir::Type::Kind::Float && dst_type.kind == ir::Type::Kind::Int) {
                utils::append(&reg, type_default_register(dst_type));
                utils::appendf(&output, "  cvtts%c2si -%zu(%%rbp), %%eax\n", fp_suffix(src_size), src.offset);
              }

              else if (src.type.kind == ir::Type::Kind::Int && dst_type.kind == ir::Type::Kind::Float) {
                utils::append(&reg, type_default_register(dst_type));
                utils::appendf(&output, "  cvtsi2s%c -%zu(%%rbp), %%%s\n",
                               fp_suffix(dst_size), src.offset, reg.content);
              }

              else if (src.type.kind == ir::Type::Kind::Float && dst_type.kind == ir::Type::Kind::Float) {
                utils::append(&reg, type_default_register(src.type));

                if (src.type.size != dst_type.size) {
                  const char src_suff = fp_suffix(src_size);
                  const char dst_suff = fp_suffix(dst_size);
                  utils::appendf(&output, "  movs%c   -%zu(%%rbp), %%%s\n", src_suff, src.offset, reg.content);
                  utils::appendf(&output, "  cvts%c2s%c %%%s, %%%s\n", src_suff, dst_suff, reg.content, reg.content);
                }

                else {
                  utils::appendf(&output, "  movs%c   -%zu(%%rbp), %%%s\n", fp_suffix(src_size), src.offset, reg.content);
                }
              }

              else if (src.type.kind == ir::Type::Kind::Int && dst_type.kind == ir::Type::Kind::Int) {
                utils::append(&reg, type_default_register(dst_type));

                if (src.type.size > dst_type.size)
                  utils::appendf(&output, "  movs%c%c  -%zu(%%rbp), %%%s\n",
                                 integer_suffix(src_size), integer_suffix(dst_size), src.offset, reg.content);

                else
                  utils::appendf(&output, "  mov%c    -%zu(%%rbp), %%%s\n", integer_suffix(dst_size), src.offset, reg.content);
              }

              else {
                todo();
              }

              if (dst_type.kind == ir::Type::Kind::Float)
                utils::appendf(&output, "  movs%c   %%%s, %s\n", fp_suffix(dst_size), reg.content, dst.content);
              else if (dst_type.kind == ir::Type::Kind::Int)
                utils::appendf(&output, "  mov%c    %%%s, %s\n", integer_suffix(dst_size), reg.content, dst.content);

              utils::dump(&reg);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg src = std::get<2>(store.src);
              const char* reg = src.name.c_str();

              if (src.type.kind == ir::Type::Kind::Int && dst_type.kind == ir::Type::Kind::Int) {
                uint dst_size = dst_type.size;
                uint src_size = src.type.size;

                if (src.type.size > dst_type.size)
                  utils::appendf(&output, "  movs%c%c  %%%s, %s\n",
                                 integer_suffix(src_size), integer_suffix(dst_size), reg, dst);

                else
                  utils::appendf(&output, "  mov%c    %%%s, %s\n", integer_suffix(dst_size), reg, dst.content);
              } else {
                todo();
              }
              break;
            }
            default:
              unreachable();
          }

          utils::dump(&dst);
          break;
        }
        case 2: // BinOp
        {
          ir::BinOp binop = std::get<2>(inst);
          switch (binop.op) {
            case ir::BinOp::Op::Add: // addition
            {
              uint dst_size = binop.dst.type.size;
              const char* dst = binop.dst.name.c_str();
              const char dst_suff = integer_suffix(dst_size);

              if (binop.lhs.index() == 2) {
                ir::PhysReg lreg = std::get<2>(binop.lhs);

                if (lreg.name == binop.dst.name) {
                  utils::Str src = utils::init(4);
                  switch (binop.rhs.index()) {
                    case 0: // Constant
                    {
                      ir::Constant constant = std::get<0>(binop.rhs);
                      if (constant.value.index() == 0) {
                        int64_t value = std::get<0>(constant.value);
                        utils::appendf(&src, "$%ld", value);
                      }
                      if (constant.value.index() == 1) {
                        double value = std::get<1>(constant.value);
                        utils::appendf(&src, "$%lf", value);
                      }
                      break;
                    }
                    case 1: // VirtReg
                    {
                      Variable var = local_vars[std::get<1>(binop.rhs).id];
                      utils::appendf(&src, "-%zu(%%rbp)", var.offset);
                      break;
                    }
                    case 2: // PhysReg
                    {
                      ir::PhysReg phy = std::get<2>(binop.rhs);
                      const char* reg = phy.name.c_str();
                      utils::appendf(&src, "%%%s", reg);
                      break;
                    }
                  }
                  utils::appendf(&output, "  add%c    %s, %%%s\n", dst_suff, src.content, dst);
                  break;
                }
              }

              if (binop.rhs.index() == 2) {
                ir::PhysReg rreg = std::get<2>(binop.rhs);

                if (rreg.name == binop.dst.name) {
                  utils::Str src = utils::init(4);
                  switch (binop.lhs.index()) {
                    case 0: // Constant
                    {
                      ir::Constant constant = std::get<0>(binop.lhs);
                      if (constant.value.index() == 0) {
                        int64_t value = std::get<0>(constant.value);
                        utils::appendf(&src, "$%ld", value);
                      }
                      if (constant.value.index() == 1) {
                        double value = std::get<1>(constant.value);
                        utils::appendf(&src, "$%lf", value);
                      }
                      break;
                    }
                    case 1: // VirtReg
                    {
                      Variable var = local_vars[std::get<1>(binop.lhs).id];
                      utils::appendf(&src, "-%zu(%%rbp)", var.offset);
                      break;
                    }
                    case 2: // PhysReg
                    {
                      ir::PhysReg phy = std::get<2>(binop.lhs);
                      const char* reg = phy.name.c_str();
                      utils::appendf(&src, "%%%s", reg);
                      break;
                    }
                  }
                  utils::appendf(&output, "  add%c    %s, %%%s\n", dst_suff, src.content, dst);
                  break;
                }
              }

              if (binop.lhs.index() == 1) {
                Variable var = local_vars[std::get<1>(binop.lhs).id];
                utils::appendf(&output, "  mov%c    -%zu(%%rbp), %%%s\n", dst_suff, var.offset, dst);

                utils::Str src = utils::init(4);
                switch (binop.rhs.index()) {
                  case 0: // Constant
                  {
                    ir::Constant constant = std::get<0>(binop.rhs);
                    if (constant.value.index() == 0) {
                      int64_t value = std::get<0>(constant.value);
                      utils::appendf(&src, "$%ld", value);
                    }
                    if (constant.value.index() == 1) {
                      double value = std::get<1>(constant.value);
                      utils::appendf(&src, "$%lf", value);
                    }
                    break;
                  }
                  case 1: // VirtReg
                  {
                    Variable var = local_vars[std::get<1>(binop.rhs).id];
                    utils::appendf(&src, "-%zu(%%rbp)", var.offset);
                    break;
                  }
                  default:
                    unreachable();
                }

                utils::appendf(&output, "  add%c    %s, %%%s\n", dst_suff, src.content, dst);
                break;
              }

              if (binop.rhs.index() == 1) {
                Variable var = local_vars[std::get<1>(binop.rhs).id];
                utils::appendf(&output, "  mov%c    -%zu(%%rbp), %%%s\n", dst_suff, var.offset, dst);

                utils::Str src = utils::init(4);
                switch (binop.lhs.index()) {
                  case 0: // Constant
                  {
                    ir::Constant constant = std::get<0>(binop.lhs);
                    if (constant.value.index() == 0) {
                      int64_t value = std::get<0>(constant.value);
                      utils::appendf(&src, "$%ld", value);
                    }
                    if (constant.value.index() == 1) {
                      double value = std::get<1>(constant.value);
                      utils::appendf(&src, "$%lf", value);
                    }
                    break;
                  }
                  case 1: // VirtReg
                  {
                    Variable var = local_vars[std::get<1>(binop.lhs).id];
                    utils::appendf(&src, "-%zu(%%rbp)", var.offset);
                    break;
                  }
                  default:
                    unreachable();
                }

                utils::appendf(&output, "  add%c    %s, %%%s\n", dst_suff, src.content, dst);
                break;
              }

              // the only remaining case is Constant with Constant
              // which is handled in ir generation
              unreachable();
            }
            case ir::BinOp::Op::Sub: // substraction
            {
              todo();
            }
            case ir::BinOp::Op::Mul: // multiplication
            {
              todo();
            }
            case ir::BinOp::Op::Div: // division
            {
              todo();
            }
          }

          break;
        }
        case 3: // UnOp
        {
          todo();
        }
      }
    }
    void Gen::generate_data() {
      if (const_fps.empty())
        return;

      utils::append(&output, "# data\n");

      // helper
      auto kind_to_string = [](Directive::Kind kind) {
        switch (kind) {
          case Directive::Kind::Float:
            return ".float";
          case Directive::Kind::Double:
            return ".double";
          case Directive::Kind::Asciz:
            return ".asciz";
          default:
            unreachable();
        }
      };

      for (auto element : const_fps) {
        DataLabel label = element.second;
        utils::appendf(&output, "%s:\n", label.name.c_str());

        for (auto dir : label.dirs) {
          switch (dir.data.index()) {
            case 0: // double
            {
              double value = std::get<0>(dir.data);
              utils::appendf(&output, "  %s  %lf\n", kind_to_string(dir.kind), value);
              break;
            }
            case 1: // std::string
            {
              std::string value = std::get<1>(dir.data);
              utils::appendf(&output, "  %s  %s\n", kind_to_string(dir.kind), value.c_str());
              break;
            }
          }
        }
      }
    }

    void Gen::generate_terminator(ir::Terminator& term, ir::Type& return_type) {
      switch (term.index()) {
        case 0: // Return
        {
          ir::Return& ret = std::get<0>(term);
          const char ret_suff = type_suffix(return_type);
          const char* ret_reg = type_default_register(return_type);

          switch (ret.value.index()) {
            case 0: // Constant
            {
              ir::Constant& constant = std::get<0>(ret.value);

              switch (constant.value.index()) {
                case 0: // int64_t
                {
                  int64_t value = std::get<0>(constant.value);

                  if (value == 0)
                    utils::appendf(&output, "  xor%c    %%%s, %%%s\n", ret_suff, ret_reg, ret_reg);
                  else
                    utils::appendf(&output, "  mov%c    $%lu, %%%s\n", ret_suff, value, ret_reg);

                  break;
                }
                case 1: // double
                {
                  double value = std::get<1>(constant.value);

                  if (value == 0)
                    utils::append(&output, "  pxor    %xmm0, %xmm0\n");

                  DataLabel label;
                  if (const_fps.find(value) == const_fps.end()) {
                    Directive::Kind kind;

                    if (return_type.size == 4)
                      kind = Directive::Kind::Float;
                    else if (return_type.size == 8)
                      kind = Directive::Kind::Double;
                    else
                      unreachable();

                    label.name = ".CFPS" + std::to_string(const_fps.size());
                    label.dirs.push_back(Directive{ .kind = kind, .data = value });
                    const_fps[value] = label;
                  }

                  // already exist
                  else
                    label = const_fps[value];

                  utils::appendf(&output, "  movs%c   %s(%%rip), %%%s\n", ret_suff, label.name.c_str(), ret_reg);
                }
              }

              break;
            }
            case 1: // Register
            {
              Variable value = local_vars[std::get<1>(ret.value).id];

              utils::appendf(&output, "  mov%c    -%zu(%%rbp), %%%s\n", ret_suff, value.offset, ret_reg);
              break;
            }
          }

          utils::appendf(&output, "  popq    %%rbp\n");
          utils::appendf(&output, "  ret\n");
        }
      }
    }
    void Gen::generate_default_terminator(ir::Type& type) {
      utils::appendf(&output, "  nop\n");

      if (type.kind == ir::Type::Kind::Float) {
        const char suff = (type.size == 4) ? 'd' : 'q';
        const char* reg = (type.size == 32) ? "eax" : "rax";

        utils::appendf(&output, "  mov%c    %%%s, %%xmm0\n", suff, reg);
      }

      utils::appendf(&output, "  popq    %%rbp\n");
      utils::appendf(&output, "  ret\n");
    }

    char Gen::type_suffix(ir::Type& type) {
      switch (type.kind) {
        case ir::Type::Kind::Float: // f32/f64
        {
          if (type.size == 4)
            return 's';
          else if (type.size == 8)
            return 'd';
          else
            unreachable();
        }
        default: // i/u(1, 16, 32, 64)
        {
          if (type.size == 1)
            return 'b';
          else if (type.size == 2)
            return 'w';
          else if (type.size == 4)
            return 'l';
          else if (type.size == 8)
            return 'q';
          else
            unreachable();
        }
      }
    }
    char Gen::integer_suffix(unsigned int size) {
      // clang-format off
      switch (size) {
         case 1: return 'b'; 
         case 2: return 'w';
         case 4: return 'l'; 
         case 8: return 'q';
         default:
           printf("Undefined integer size suffix for: %u\n", size);
           exit(1);
      }
      // clang-format on
    }
    char Gen::fp_suffix(unsigned int size) {
      // clang-format off
      switch (size) {
         case 4: return 's'; 
         case 8: return 'd';
         default:
           printf("Undefined fp size suffix for: %u\n", size);
           unreachable();
      }
      // clang-format on
    }

    char* Gen::type_default_register(ir::Type& type) {
      if (type.kind == ir::Type::Kind::Float)
        return (char*)"xmm0";

      // clang-format off
      switch (type.size) {
         case 1: return (char*)"al"; 
         case 2: return (char*)"ax";
         case 4: return (char*)"eax"; 
         case 8: return (char*)"rax";
         default: unreachable();
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
    char* Gen::subreg_name(const std::string& reg, size_t size) {
      // clang-format off
      if (reg == "rax") {
        switch (size) {
         case 1: return (char*)"al";
         case 2: return (char*)"ax";
         case 4: return (char*)"eax";
         case 8: return (char*)"rax";
        }
      } else if (reg == "rbx") {
        switch (size) {
         case 1: return (char*)"bl";
         case 2: return (char*)"bx";
         case 4: return (char*)"ebx";
         case 8: return (char*)"rbx";
        }
      } else if (reg == "rcx") {
        switch (size) {
         case 1: return (char*)"cl";
         case 2: return (char*)"cx";
         case 4: return (char*)"ecx";
         case 8: return (char*)"rcx";
        }
      } else if (reg == "rdx") {
        switch (size) {
         case 1: return (char*)"dl";
         case 2: return (char*)"dx";
         case 4: return (char*)"edx";
         case 8: return (char*)"rdx";
        }
      } else if (reg == "rsp") {
        switch (size) {
         case 1: return (char*)"spl";
         case 2: return (char*)"sp";
         case 4: return (char*)"esp";
         case 8: return (char*)"rsp";
        }
      } else if (reg == "rbp") {
        switch (size) {
         case 1: return (char*)"bpl";
         case 2: return (char*)"bp";
         case 4: return (char*)"ebp";
         case 8: return (char*)"rbp";
        }
      } else if (reg == "rsi") {
        switch (size) {
         case 1: return (char*)"sil";
         case 2: return (char*)"si";
         case 4: return (char*)"esi";
         case 8: return (char*)"rsi";
        }
      } else if (reg == "rdi") {
        switch (size) {
         case 1: return (char*)"dil";
         case 2: return (char*)"di";
         case 4: return (char*)"edi";
         case 8: return (char*)"rdi";
        }
      } else if (reg == "r8") {
        switch (size) {
         case 1: return (char*)"r8b";
         case 2: return (char*)"r8w";
         case 4: return (char*)"r8d";
         case 8: return (char*)"r8";
        }
      } else if (reg == "r9") {
        switch (size) {
         case 1: return (char*)"r9b";
         case 2: return (char*)"r9w";
         case 4: return (char*)"r9d";
         case 8: return (char*)"r9";
        }
      } else if (reg == "r10") {
        switch (size) {
         case 1: return (char*)"r10b";
         case 2: return (char*)"r10w";
         case 4: return (char*)"r10d";
         case 8: return (char*)"r10";
        }
      } else if (reg == "r11") {
        switch (size) {
         case 1: return (char*)"r11b";
         case 2: return (char*)"r11w";
         case 4: return (char*)"r11d";
         case 8: return (char*)"r11";
        }
      } else if (reg == "r12") {
        switch (size) {
         case 1: return (char*)"r12b";
         case 2: return (char*)"r12w";
         case 4: return (char*)"r12d";
         case 8: return (char*)"r12";
        }
      } else if (reg == "r13") {
        switch (size) {
         case 1: return (char*)"r13b";
         case 2: return (char*)"r13w";
         case 4: return (char*)"r13d";
         case 8: return (char*)"r13";
        }
      } else if (reg == "r14") {
        switch (size) {
         case 1: return (char*)"r14b";
         case 2: return (char*)"r14w";
         case 4: return (char*)"r14d";
         case 8: return (char*)"r14";
        }
      } else if (reg == "r15") {
        switch (size) {
         case 1: return (char*)"r15b";
         case 2: return (char*)"r15w";
         case 4: return (char*)"r15d";
         case 8: return (char*)"r15";
        }
      }

      unreachable();
      // clang-format on
    }
  } // namespace codegen
} // namespace phantom
