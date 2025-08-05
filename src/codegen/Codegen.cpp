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
        const char* reg = get_register_by_size(regs[i], param.type.size);

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
              utils::appendf(&dst, "%%%s", resolve_physical_register(phy));
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

              if (src.type.kind == ir::Type::Kind::Int && dst_type.kind == ir::Type::Kind::Int) {
                utils::append(&reg, type_default_register(dst_type));
                const char src_suff = integer_suffix(src_size);
                const char dst_suff = integer_suffix(dst_size);

                utils::Str mov = utils::init(10);
                if (src_size > dst_size)
                  utils::appendf(&mov, "movs%c%c", src_suff, dst_suff);
                else
                  utils::appendf(&mov, "mov%c", dst_suff);

                utils::appendf(&output, "  %-7s -%zu(%%rbp), %%%s\n",
                               mov.content, src.offset, reg.content);

                utils::dump(&mov);
              }

              else {
                utils::append(&reg, type_default_register(src.type));
                utils::appendf(&output, "  movs%c   -%zu(%%rbp), %%%s\n",
                               fp_suffix(src_size), src.offset, reg.content);
              }

              if (dst_type.kind == ir::Type::Kind::Float)
                utils::appendf(&output, "  movs%c   %%%s, %s\n",
                               fp_suffix(dst_size), reg.content, dst.content);
              else if (dst_type.kind == ir::Type::Kind::Int)
                utils::appendf(&output, "  mov%c    %%%s, %s\n",
                               integer_suffix(dst_size), reg.content, dst.content);

              utils::dump(&reg);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg src = std::get<2>(store.src);
              const char* reg = resolve_physical_register(src);

              // both are integers
              if (src.type.kind == ir::Type::Kind::Int && dst_type.kind == ir::Type::Kind::Int) {
                uint dst_size = dst_type.size;
                uint src_size = src.type.size;
                const char src_suff = integer_suffix(src_size);
                const char dst_suff = integer_suffix(dst_size);

                utils::Str mov = utils::init(10);
                if (src_size > dst_size)
                  utils::appendf(&mov, "movs%c%c", src_suff, dst_suff);
                else
                  utils::appendf(&mov, "mov%c", dst_suff);

                utils::appendf(&output, "  %-7s %%%s, %s\n", mov.content, reg, dst.content);
                utils::dump(&mov);
                break;
              }

              // both are floats
              utils::appendf(&output, "  movs%c   %%%s, %s\n", fp_suffix(src.type.size), reg, dst.content);
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
              ir::Type dst_type = binop.dst.type;

              const char* dst = resolve_physical_register(binop.dst);
              const char dst_suff = type_suffix(dst_type);

              const char* extra = (dst_type.kind == ir::Type::Kind::Float) ? "s" : "";

              // PhysReg
              if (binop.lhs.index() == 2) {
                ir::PhysReg lpr = std::get<2>(binop.lhs);
                const char* lr = resolve_physical_register(lpr);

                if (lr == dst) {
                  utils::Str src = utils::init(4);
                  switch (binop.rhs.index()) {
                    case 0: // Constant
                    {
                      ir::Constant constant = std::get<0>(binop.rhs);
                      if (constant.value.index() == 0) {
                        int64_t value = std::get<0>(constant.value);
                        utils::appendf(&src, "$%ld", value);
                      }

                      else if (constant.value.index() == 1) {
                        double value = std::get<1>(constant.value);
                        Directive::Kind kind;

                        if (dst_type.size == 4)
                          kind = Directive::Kind::Float;
                        else
                          kind = Directive::Kind::Double;

                        DataLabel label = constant_fp_label(value, kind);
                        utils::appendf(&src, "%s(%%rip)", label.name.c_str());
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
                      const char* reg = resolve_physical_register(phy);
                      utils::appendf(&src, "%%%s", reg);
                      break;
                    }
                  }

                  utils::appendf(&output, "  add%s%c    %s, %%%s\n", extra, dst_suff, src.content, dst);
                  utils::dump(&src);
                  break;
                }
              }
              if (binop.rhs.index() == 2) {
                ir::PhysReg rpr = std::get<2>(binop.rhs);
                const char* rr = resolve_physical_register(rpr);

                if (rr == dst) {
                  utils::Str src = utils::init(4);
                  switch (binop.lhs.index()) {
                    case 0: // Constant
                    {
                      ir::Constant constant = std::get<0>(binop.lhs);
                      if (constant.value.index() == 0) {
                        int64_t value = std::get<0>(constant.value);
                        utils::appendf(&src, "$%ld", value);
                      }

                      else if (constant.value.index() == 1) {
                        double value = std::get<1>(constant.value);
                        Directive::Kind kind;

                        if (dst_type.size == 4)
                          kind = Directive::Kind::Float;
                        else
                          kind = Directive::Kind::Double;

                        DataLabel label = constant_fp_label(value, kind);
                        utils::appendf(&src, "%s(%%rip)", label.name.c_str());
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
                      const char* reg = resolve_physical_register(phy);
                      utils::appendf(&src, "%%%s", reg);
                      break;
                    }
                  }

                  utils::appendf(&output, "  add%s%c    %s, %%%s\n", extra, dst_suff, src.content, dst);
                  utils::dump(&src);
                  break;
                }
              }

              // VirtReg
              if (binop.lhs.index() == 1) {
                Variable var = local_vars[std::get<1>(binop.lhs).id];
                utils::appendf(&output, "  mov%s%c    -%zu(%%rbp), %%%s\n", extra, dst_suff, var.offset, dst);

                utils::Str src = utils::init(4);
                switch (binop.rhs.index()) {
                  case 0: // Constant
                  {
                    ir::Constant constant = std::get<0>(binop.rhs);
                    if (constant.value.index() == 0) {
                      int64_t value = std::get<0>(constant.value);
                      utils::appendf(&src, "$%ld", value);
                    }

                    else if (constant.value.index() == 1) {
                      double value = std::get<1>(constant.value);
                      Directive::Kind kind;

                      if (dst_type.size == 4)
                        kind = Directive::Kind::Float;
                      else
                        kind = Directive::Kind::Double;

                      DataLabel label = constant_fp_label(value, kind);
                      utils::appendf(&src, "%s(%%rip)", label.name.c_str());
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

                utils::appendf(&output, "  add%s%c    %s, %%%s\n", extra, dst_suff, src.content, dst);
                utils::dump(&src);
                break;
              }
              if (binop.rhs.index() == 1) {
                Variable var = local_vars[std::get<1>(binop.rhs).id];
                utils::appendf(&output, "  mov%s%c    -%zu(%%rbp), %%%s\n", extra, dst_suff, var.offset, dst);

                utils::Str src = utils::init(4);
                switch (binop.lhs.index()) {
                  case 0: // Constant
                  {
                    ir::Constant constant = std::get<0>(binop.lhs);
                    if (constant.value.index() == 0) {
                      int64_t value = std::get<0>(constant.value);
                      utils::appendf(&src, "$%ld", value);
                    }

                    else if (constant.value.index() == 1) {
                      double value = std::get<1>(constant.value);
                      Directive::Kind kind;

                      if (dst_type.size == 4)
                        kind = Directive::Kind::Float;
                      else
                        kind = Directive::Kind::Double;

                      DataLabel label = constant_fp_label(value, kind);
                      utils::appendf(&src, "%s(%%rip)", label.name.c_str());
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

                utils::appendf(&output, "  add%s%c    %s, %%%s\n", extra, dst_suff, src.content, dst);
                utils::dump(&src);
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
        case 4: // Int2Float
        {
          ir::Int2Float cvt = std::get<4>(inst);
          const char* dst = resolve_physical_register(cvt.dst);
          switch (cvt.value.index()) {
            case 0: // Constant
            {
              ir::Constant constant = std::get<0>(cvt.value);
              if (constant.value.index() != 0) std::abort();

              int64_t value = std::get<0>(constant.value);
              if (value == 0) {
                utils::appendf(&output, "  pxor    %%%s, %%%s\n", dst, dst);
                break;
              }

              DataLabel label = constant_fp_label(value, Directive::Kind::Float);
              utils::appendf(&output, "  movss   %s(%%rip), %%%s\n", label.name.c_str(), dst);
              break;
            }
            case 1: // VirtReg
            {
              Variable var = local_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtsi2ss -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              utils::appendf(&output, "  cvtsi2ss %%%s, %%%s\n", resolve_physical_register(pr), dst);
              break;
            }
          }
          break;
        }
        case 5: // Int2Double
        {
          ir::Int2Double cvt = std::get<5>(inst);
          const char* dst = resolve_physical_register(cvt.dst);

          switch (cvt.value.index()) {
            case 0: // Constant
            {
              ir::Constant constant = std::get<0>(cvt.value);
              if (constant.value.index() != 0) std::abort();

              int64_t value = std::get<0>(constant.value);
              if (value == 0) {
                utils::appendf(&output, "  pxor    %%%s, %%%s\n", dst, dst);
                break;
              }

              DataLabel label = constant_fp_label(value, Directive::Kind::Double);
              utils::appendf(&output, "  movsd   %s(%%rip), %%%s\n", label.name.c_str(), dst);
              break;
            }
            case 1: // VirtReg
            {
              Variable var = local_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtsi2sd -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              utils::appendf(&output, "  cvtsi2sd %%%s, %%%s\n", resolve_physical_register(pr), dst);
              break;
            }
          }
          break;
        }
        case 6: // Float2Int
        {
          ir::Float2Int cvt = std::get<6>(inst);
          const char* dst = resolve_physical_register(cvt.dst);
          switch (cvt.value.index()) {
            case 0: // Constant
            {
              torevise();
              break;
            }
            case 1: // VirtReg
            {
              Variable var = local_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtss2si -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              utils::appendf(&output, "  cvtss2si %%%s, %%%s\n", resolve_physical_register(pr), dst);
              break;
            }
          }
          break;
        }
        case 7: // Float2Double
        {
          ir::Float2Double cvt = std::get<7>(inst);
          const char* dst = resolve_physical_register(cvt.dst);
          switch (cvt.value.index()) {
            case 0: // Constant
            {
              ir::Constant constant = std::get<0>(cvt.value);
              if (constant.value.index() != 1) std::abort();

              double value = std::get<1>(constant.value);
              if (value == 0) {
                utils::appendf(&output, "  pxor    %%%s, %%%s\n", dst, dst);
                break;
              }

              DataLabel label = constant_fp_label(value, Directive::Kind::Double);
              utils::appendf(&output, "  movsd   %s(%%rip), %%%s\n", label.name.c_str(), dst);
              break;
            }
            case 1: // VirtReg
            {
              Variable var = local_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtss2sd -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              utils::appendf(&output, "  cvtss2sd %%%s, %%%s\n", resolve_physical_register(pr), dst);
              break;
            }
          }
          break;
        }
        case 8: // Double2Int
        {
          ir::Double2Int cvt = std::get<8>(inst);
          const char* dst = resolve_physical_register(cvt.dst);
          switch (cvt.value.index()) {
            case 0: // Constant
            {
              torevise();
              break;
            }
            case 1: // VirtReg
            {
              Variable var = local_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtsd2si -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              utils::appendf(&output, "  cvtsd2si %%%s, %%%s\n",resolve_physical_register(pr), dst);
              break;
            }
          }
          break;
        }
        case 9: // Double2Float
        {
          ir::Double2Float cvt = std::get<9>(inst);
          const char* dst = resolve_physical_register(cvt.dst);
          switch (cvt.value.index()) {
            case 0: // Constant
            {
              torevise();
            }
            case 1: // VirtReg
            {
              Variable var = local_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtsd2ss -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              utils::appendf(&output, "  cvtsd2ss %%%s, %%%s\n", resolve_physical_register(pr), dst);
              break;
            }
          }
          break;
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

    DataLabel Gen::constant_fp_label(double value, Directive::Kind kind) {
      if (const_fps.find(value) != const_fps.end())
        return const_fps[value];

      DataLabel label;
      label.dirs.push_back(Directive{ .kind = kind, .data = value });
      label.name = ".CFPS" + std::to_string(const_fps.size());
      const_fps[value] = label;

      return label;
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
    char* Gen::resolve_physical_register(ir::PhysReg& pr) {
      // clang-format off
      switch (pr.reg) {
        case ir::PhysReg::Reg::I1: return get_register_by_size("rax", pr.type.size);
        case ir::PhysReg::Reg::I2: return get_register_by_size("rcx", pr.type.size);
        case ir::PhysReg::Reg::F1: return (char*) "xmm0";
        case ir::PhysReg::Reg::F2: return (char*) "xmm1";
      }
      // clang-format on
    }
  } // namespace codegen
} // namespace phantom
