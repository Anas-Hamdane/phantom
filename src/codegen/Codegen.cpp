#include "codegen/Codegen.hpp"
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
        size_t size = (param.type.bitwidth == 0) ? 1 : (param.type.bitwidth / 8);

        const char suff = integer_suffix(size);
        const char* reg = subreg_name(regs[i], size);

        utils::appendf(&output, "  mov%c    %%%s, -%zu(%%rbp)\n", suff, reg, offset + size);
        offset += size;

        local_vars[param.id] = Variable{ .type = param.type, .offset = offset };
      }

      for (size_t i = tmp; i < params_size; ++i) {
        ir::VirtReg param = fn.params.at(i);
        size_t size = (param.type.bitwidth == 0) ? 1 : (param.type.bitwidth / 8);

        const char suff = integer_suffix(size);
        const char* reg = size_areg(size);

        utils::appendf(&output, "  mov%c    %zu(%%rbp), %%%s\n", suff, ((i - tmp) + 2) * 8, reg);
        utils::appendf(&output, "  mov%c    %%%s, -%zu(%%rbp)\n", suff, reg, offset + size);
        offset += size;

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
          offset += (alloca.type.bitwidth == 0) ? 1 : (alloca.type.bitwidth / 8);
          local_vars[alloca.reg.id] = Variable{ .type = alloca.type, .offset = offset };
          break;
        }
        case 1: // Store
        {
          ir::Store& store = std::get<1>(inst);
          utils::Str dst = utils::init(4);
          Type dst_type;
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
              utils::appendf(&dst, "%%%s", subreg_name((phy.name == 'A') ? "rax" : "rcx", type_size(dst_type)));
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

                    if (dst_type.bitwidth == 32)
                      kind = Directive::Kind::Float;
                    else if (dst_type.bitwidth == 64)
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
              uint dst_size = type_size(dst_type);
              uint src_size = type_size(src.type);
              utils::Str reg = utils::init(5);

              if (src.type.kind == Type::Kind::FP && dst_type.kind == Type::Kind::Int) {
                utils::append(&reg, type_default_register(dst_type));
                utils::appendf(&output, "  cvtts%c2si -%zu(%%rbp), %%eax\n", fp_suffix(src_size), src.offset);
              }

              else if (src.type.kind == Type::Kind::Int && dst_type.kind == Type::Kind::FP) {
                utils::append(&reg, type_default_register(dst_type));
                utils::appendf(&output, "  cvtsi2s%c -%zu(%%rbp), %%%s\n",
                               fp_suffix(dst_size), src.offset, reg.content);
              }

              else if (src.type.kind == Type::Kind::FP && dst_type.kind == Type::Kind::FP) {
                utils::append(&reg, type_default_register(src.type));

                if (src.type.bitwidth != dst_type.bitwidth) {
                  const char src_suff = fp_suffix(src_size);
                  const char dst_suff = fp_suffix(dst_size);
                  utils::appendf(&output, "  movs%c   -%zu(%%rbp), %%%s\n", src_suff, src.offset, reg.content);
                  utils::appendf(&output, "  cvts%c2s%c %%%s, %%%s\n", src_suff, dst_suff, reg.content, reg.content);
                }

                else {
                  utils::appendf(&output, "  movs%c   -%zu(%%rbp), %%%s\n", fp_suffix(src_size), src.offset, reg.content);
                }
              }

              else if (src.type.kind == Type::Kind::Int && dst_type.kind == Type::Kind::Int) {
                utils::append(&reg, type_default_register(dst_type));

                if (src.type.bitwidth > dst_type.bitwidth)
                  utils::appendf(&output, "  movs%c%c  -%zu(%%rbp), %%%s\n",
                                 integer_suffix(src_size), integer_suffix(dst_size), src.offset, reg.content);

                else
                  utils::appendf(&output, "  mov%c    -%zu(%%rbp), %%%s\n", integer_suffix(dst_size), src.offset, reg.content);
              }

              else {
                todo();
              }

              if (dst_type.kind == Type::Kind::FP)
                utils::appendf(&output, "  movs%c   %%%s, %s\n", fp_suffix(dst_size), reg.content, dst.content);
              else if (dst_type.kind == Type::Kind::Int)
                utils::appendf(&output, "  mov%c    %%%s, %s\n", integer_suffix(dst_size), reg.content, dst.content);

              utils::dump(&reg);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg src = std::get<2>(store.src);
              const char* reg = subreg_name((src.name == 'A') ? "rax" : "rcx", type_size(src.type));

              if (src.type.kind == Type::Kind::Int && dst_type.kind == Type::Kind::Int) {
                uint dst_size = type_size(dst_type);
                uint src_size = type_size(src.type);

                if (src.type.bitwidth > dst_type.bitwidth)
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
              uint dst_size = type_size(binop.dst.type);
              const char* dst = subreg_name((binop.dst.name == 'A') ? "rax" : "rcx", dst_size);
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
                      const char* reg = subreg_name((phy.name == 'A') ? "rax" : "rcx", type_size(phy.type));
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
                      const char* reg = subreg_name((phy.name == 'A') ? "rax" : "rcx", type_size(phy.type));
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

    void Gen::generate_terminator(ir::Terminator& term, Type& return_type) {
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

                    if (return_type.bitwidth == 32)
                      kind = Directive::Kind::Float;
                    else if (return_type.bitwidth == 64)
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
    void Gen::generate_default_terminator(Type& type) {
      utils::appendf(&output, "  nop\n");

      if (type.kind == Type::Kind::FP) {
        const char suff = (type.bitwidth == 32) ? 'd' : 'q';
        const char* reg = (type.bitwidth == 32) ? "eax" : "rax";

        utils::appendf(&output, "  mov%c    %%%s, %%xmm0\n", suff, reg);
      }

      utils::appendf(&output, "  popq    %%rbp\n");
      utils::appendf(&output, "  ret\n");
    }

    char Gen::type_suffix(Type& type) {
      switch (type.kind) {
        case Type::Kind::FP: // f32/f64
        {
          uint size = type.bitwidth / 8;

          if (size == 4)
            return 's';
          else if (size == 8)
            return 'd';
          else
            unreachable();
        }
        default: // i/u(1, 16, 32, 64)
        {
          uint size = (type.bitwidth == 1) ? 1 : (type.bitwidth / 8);

          if (size == 1)
            return 'b';
          else if (size == 2)
            return 'w';
          else if (size == 4)
            return 'l';
          else if (size == 8)
            return 'q';
          else
            unreachable();
        }
      }
    }
    uint Gen::type_size(Type& type) {
      return (type.bitwidth == 1) ? 1 : (type.bitwidth / 8);
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
           exit(1);
      }
      // clang-format on
    }

    char* Gen::type_default_register(Type& type) {
      uint size = type_size(type);

      if (type.kind == Type::Kind::FP)
        return (char*)"xmm0";

      // clang-format off
      switch (size) {
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
