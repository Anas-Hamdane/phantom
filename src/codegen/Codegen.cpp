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
        const char* reg = get_register_by_size("rax", param.type.size);

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

          switch (store.dst.index()) {
            case 0: // VirtReg
            {
              ir::VirtReg memory = std::get<0>(store.dst);
              switch (store.src.index()) {
                case 0: // Constant
                {
                  ir::Constant constant = std::get<0>(store.src);
                  return store_constant_in_memory(memory, constant);
                }
                case 1: // VirtReg
                {
                  ir::VirtReg value = std::get<1>(store.src);
                  return store_memory_in_memory(memory, value);
                }
                case 2: // PhysReg
                {
                  ir::PhysReg value = std::get<2>(store.src);
                  return store_register_in_memory(memory, value);
                }
                default:
                  unreachable();
              }
            }
            case 1: // PhysReg
            {
              ir::PhysReg phy = std::get<1>(store.dst);
              switch (store.src.index()) {
                case 0: // Constant
                {
                  ir::Constant constant = std::get<0>(store.src);
                  return store_constant_in_register(phy, constant);
                }
                case 1: // VirtReg
                {
                  ir::VirtReg value = std::get<1>(store.src);
                  return store_memory_in_register(phy, value);
                }
                case 2: // PhysReg
                {
                  ir::PhysReg value = std::get<2>(store.src);
                  return store_register_in_register(phy, value);
                }
                default:
                  unreachable();
              }
            }
            default:
              unreachable();
          }

          break;
        }
        case 2: // BinOp
        {
          ir::BinOp binop = std::get<2>(inst);

          switch (binop.op) {
            case ir::BinOp::Op::Add: // addition
            {
              switch (binop.lhs.index()) {
                case 0: // Constant
                {
                  ir::Constant constant = std::get<0>(binop.lhs);
                  switch (binop.rhs.index()) {
                    case 1: // VirtReg
                    {
                      ir::VirtReg virt = std::get<1>(binop.rhs);
                      return add_constant_to_memory(constant, virt, binop.dst);
                    }
                    case 2: // PhysReg
                    {
                      ir::PhysReg phy = std::get<2>(binop.rhs);
                      add_constant_to_register(constant, phy);

                      if (phy.reg != binop.dst.reg)
                        store_register_in_register(binop.dst, phy);

                      return;
                    }
                    default:
                      unreachable();
                  }
                }
                case 1: // VirtReg
                {
                  ir::VirtReg virt = std::get<1>(binop.lhs);
                  switch (binop.rhs.index()) {
                    case 0: // Constant
                    {
                      ir::Constant constant = std::get<0>(binop.rhs);
                      return add_constant_to_memory(constant, virt, binop.dst);
                    }
                    case 1: // VirtReg
                    {
                      ir::VirtReg virt2 = std::get<1>(binop.rhs);
                      return add_memory_to_memory(virt, virt2, binop.dst);
                    }
                    case 2: // PhysReg
                    {
                      ir::PhysReg phy = std::get<2>(binop.rhs);
                      add_memory_to_register(virt, phy);

                      if (phy.reg != binop.dst.reg)
                        store_register_in_register(binop.dst, phy);

                      return;
                    }
                    default:
                      unreachable();
                  }
                }
                case 2: // PhysReg
                {
                  ir::PhysReg phy = std::get<2>(binop.lhs);
                  switch (binop.rhs.index()) {
                    case 0: // Constant
                    {
                      ir::Constant constant = std::get<0>(binop.rhs);
                      add_constant_to_register(constant, phy);
                      break;
                    }
                    case 1: // VirtReg
                    {
                      ir::VirtReg virt = std::get<1>(binop.rhs);
                      add_memory_to_register(virt, phy);
                      break;
                    }
                    case 2: // PhysReg
                    {
                      ir::PhysReg p = std::get<2>(binop.rhs);
                      add_register_to_register(phy, p);
                      break;
                    }
                    default:
                      unreachable();
                  }

                  if (phy.reg != binop.dst.reg)
                    store_register_in_register(binop.dst, phy);

                  return;
                }
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
              const char* prn = resolve_physical_register(pr);

              if (pr.type.size < 4) {
                const char* ir = get_register_by_size(prn, 4);
                const char prs = type_suffix(pr.type);

                utils::appendf(&output, "  movs%cl  %%%s, %%%s\n", prs, prn, ir);
                prn = ir;
              }

              utils::appendf(&output, "  cvtsi2sd %%%s, %%%s\n", prn, dst);
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
              utils::appendf(&output, "  cvtsd2si %%%s, %%%s\n", resolve_physical_register(pr), dst);
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

                  if (value == 0) {
                    utils::append(&output, "  pxor    %xmm0, %xmm0\n");
                    break;
                  }

                  Directive::Kind kind;

                  if (return_type.size == 4)
                    kind = Directive::Kind::Float;
                  else
                    kind = Directive::Kind::Double;

                  DataLabel label = constant_fp_label(value, kind);
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
              // TODO: PhysReg?
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
        const char* reg = (type.size == 4) ? "eax" : "rax";

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

    void Gen::store_constant_in_memory(ir::VirtReg& memory, ir::Constant& constant) {
      Variable variable = local_vars[memory.id];
      const char ds = type_suffix(variable.type);
      const size_t vo = variable.offset;

      if (constant.value.index() == 0) {
        int64_t v = std::get<0>(constant.value);
        utils::appendf(&output, "  mov%c    $%ld, -%zu(%%rbp)\n", ds, v, vo);
        return;
      }

      // else 'double' case
      double v = std::get<1>(constant.value);

      if (v == 0) {
        utils::append(&output, "  pxor %xmm3, %xmm3\n");
        utils::appendf(&output, "  movs%c    %%xmm3, -%zu(%%rbp)\n", ds, vo);
        return;
      }

      Directive::Kind kind;
      // clang-format off
      if (constant.type.size == 4) kind = Directive::Kind::Float;
      else kind = Directive::Kind::Double;
      // clang-format on

      DataLabel label = constant_fp_label(v, kind);

      utils::appendf(&output, "  movs%c   %s(%%rip), %%xmm0\n", ds, label.name.c_str());
      utils::appendf(&output, "  movs%c   %%xmm0, %zu(%%rbp)\n", ds, vo);
    }
    void Gen::store_register_in_memory(ir::VirtReg& memory, ir::PhysReg& reg) {
      Variable variable = local_vars[memory.id];
      const char* rn = resolve_physical_register(reg);
      const size_t vo = variable.offset;

      char* mov;
      if (is_float(variable.type) || is_float(reg.type))
        mov = generate_floating_point_move(variable.type);
      else
        mov = generate_integer_move(reg.type, variable.type);

      if (variable.type.size <= reg.type.size) {
        utils::appendf(&output, "  %-7s %%%s, -%zu(%%rbp)\n", mov, rn, vo);
        free(mov);
        return;
      }

      // (destination > src) => extension needed => you can't store the result
      // directly into a variable.

      // the mov instruction has the correct conversion but in x86-64
      // if the mov convert (movslq) the destination should be a register
      // therefor we will have to use an 'intermediate register'.

      // intermediate register
      const char* ir = get_register_by_size("rdx", variable.type.size);
      utils::appendf(&output, "  %-7s %%%s, %%%s\n", mov, rn, ir);
      utils::appendf(&output, "  mov%c    %%%s, -%zu(%%rbp)\n", type_suffix(variable.type), ir, vo);
      free(mov);
    }
    void Gen::store_memory_in_memory(ir::VirtReg& memory, ir::VirtReg& value) {
      Variable variable = local_vars[memory.id];
      Variable dst = local_vars[value.id];

      // intermediate register
      const char* ir = get_register_by_size("rdx", dst.type.size);
      char* mov;

      if (is_float(dst.type) || is_float(variable.type))
        mov = generate_floating_point_move(dst.type);
      else
        mov = generate_integer_move(dst.type, dst.type);

      utils::appendf(&output, "  %-7s -%zu(%%rbp), %%%s\n", mov, dst.offset, ir);
      free(mov);

      if (is_float(dst.type) || is_float(variable.type))
        mov = generate_floating_point_move(variable.type);
      else
        mov = generate_integer_move(dst.type, variable.type);

      utils::appendf(&output, "  %-7s %%%s, -%zu(%%rbp)\n", mov, ir, variable.offset);
      free(mov);
    }
    void Gen::store_constant_in_register(ir::PhysReg& reg, ir::Constant& constant) {
      const char* name = resolve_physical_register(reg);
      const char ds = type_suffix(reg.type);

      if (constant.value.index() == 0) {
        int64_t v = std::get<0>(constant.value);
        utils::appendf(&output, "  mov%c    $%ld, %%%s\n", ds, v, name);
        return;
      }

      // else 'double' case
      double v = std::get<1>(constant.value);

      if (v == 0) {
        utils::appendf(&output, "  pxor %%%s, %%%s\n", name, name);
        return;
      }

      Directive::Kind kind;
      // clang-format off
      if (constant.type.size == 4) kind = Directive::Kind::Float;
      else kind = Directive::Kind::Double;
      // clang-format on

      DataLabel label = constant_fp_label(v, kind);

      utils::appendf(&output, "  movs%c   %s(%%rip), %%%s\n", ds, label.name.c_str(), name);
    }
    void Gen::store_register_in_register(ir::PhysReg& reg, ir::PhysReg& value) {
      const char* rn = resolve_physical_register(reg);
      const char* vn = resolve_physical_register(value);

      char* mov;
      if (is_float(value.type) || is_float(reg.type))
        mov = generate_floating_point_move(value.type);
      else
        mov = generate_integer_move(value.type, reg.type);

      utils::appendf(&output, "  %-7s %%%s, %%%s\n", mov, vn, rn);
      free(mov);
    }
    void Gen::store_memory_in_register(ir::PhysReg& reg, ir::VirtReg& memory) {
      Variable variable = local_vars[memory.id];
      const size_t vo = variable.offset;
      const char* rn = resolve_physical_register(reg);

      char* mov;
      if (is_float(reg.type) || is_float(variable.type))
        mov = generate_floating_point_move(reg.type);
      else
        mov = generate_integer_move(variable.type, reg.type);

      utils::appendf(&output, "  %-7s -%zu(%%rbp), %%%s\n", mov, vo, rn);
      free(mov);
    }

    void Gen::add_constant_to_register(ir::Constant& constant, ir::PhysReg& phy) {
      const char* rn = resolve_physical_register(phy);
      const char rs = type_suffix(phy.type);

      switch (constant.value.index()) {
        case 0: // int64_t
        {
          int64_t v = std::get<0>(constant.value);
          if (v == 0) return;
          utils::appendf(&output, "  add%c    $%lu, %%%s\n", rs, v, rn);
          return;
        }
        case 1: // double
        {
          double v = std::get<1>(constant.value);
          if (v == 0) return;

          Directive::Kind kind;
          // clang-format off
          if (constant.type.size == 4) kind = Directive::Kind::Float;
          else kind = Directive::Kind::Double;
          // clang-format on

          DataLabel label = constant_fp_label(v, kind);
          utils::appendf(&output, "  adds%c   %s(%%rip), %%%s\n", rs, label.name.c_str(), rn);
          return;
        }
      }
      unreachable();
    }
    void Gen::add_register_to_register(ir::PhysReg& value, ir::PhysReg& dst) {
      const char* dn = resolve_physical_register(dst);
      const char* vn = resolve_physical_register(value);

      const char ds = type_suffix(dst.type);
      const char* extra = is_float(dst.type) ? "s" : "";

      if (dst.type.size > value.type.size) {
        const char* ir = get_register_by_size(vn, dst.type.size);
        const char vs = type_suffix(value.type);

        utils::appendf(&output, "  movs%c%c %%%s, %%%s\n", vs, ds, vn, ir);
        vn = ir;
      }

      utils::appendf(&output, "  add%s%c   %%%s, %%%s\n", extra, ds, vn, dn);
    }
    void Gen::add_memory_to_register(ir::VirtReg& memory, ir::PhysReg& phy) {
      Variable variable = local_vars[memory.id];
      const size_t vo = variable.offset;

      const char* dn = resolve_physical_register(phy);
      const char ds = type_suffix(phy.type);

      const char* extra = is_float(phy.type) ? "s" : "";
      utils::appendf(&output, "  add%s%c   -%zu(%%rbp), %%%s\n", extra, ds, vo, dn);
    }
    void Gen::add_constant_to_memory(ir::Constant& constant, ir::VirtReg& memory, ir::PhysReg& dst) {
      store_memory_in_register(dst, memory);
      const char* dn = resolve_physical_register(dst);
      const char ds = type_suffix(dst.type);

      switch (constant.value.index()) {
        case 0: // int64_t
        {
          int64_t v = std::get<0>(constant.value);
          if (v == 0) return;
          utils::appendf(&output, "  add%c    $%lu, %%%s\n", ds, v, dn);
          return;
        }
        case 1: // double
        {
          double v = std::get<1>(constant.value);
          if (v == 0) return;

          Directive::Kind kind;
          // clang-format off
          if (constant.type.size == 4) kind = Directive::Kind::Float;
          else kind = Directive::Kind::Double;
          // clang-format on

          DataLabel label = constant_fp_label(v, kind);
          utils::appendf(&output, "  adds%c   %s(%%rip), %%%s\n", ds, label.name.c_str(), dn);
          return;
        }
      }

      unreachable();
    }
    void Gen::add_memory_to_memory(ir::VirtReg& value, ir::VirtReg& memory, ir::PhysReg& dst) {
      store_memory_in_register(dst, memory);

      Variable variable = local_vars[value.id];
      const size_t vo = variable.offset;

      const char* dn = resolve_physical_register(dst);
      const char ds = type_suffix(dst.type);

      const char* extra = is_float(dst.type) ? "s" : "";
      utils::appendf(&output, "  add%s%c   -%zu(%%rbp), %%%s\n", extra, ds, vo, dn);
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
    char Gen::floating_point_suffix(unsigned int size) {
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

    char* Gen::generate_integer_move(ir::Type& src, ir::Type& dst) {
      utils::Str mov = utils::init("mov");
      const char ds = integer_suffix(dst.size);
      const char ss = integer_suffix(src.size);

      if (dst.size <= src.size)
        utils::appendf(&mov, "%c", ds);

      else
        utils::appendf(&mov, "s%c%c", ss, ds);

      return mov.content;
    }
    char* Gen::generate_floating_point_move(ir::Type& type) {
      utils::Str mov = utils::init("movs");
      const char suff = floating_point_suffix(type.size);

      utils::appendf(&mov, "%c", suff);
      return mov.content;
    }

    bool Gen::is_integer(ir::Type& type) {
      return (type.kind == ir::Type::Kind::Int);
    }
    bool Gen::is_float(ir::Type& type) {
      return (type.kind == ir::Type::Kind::Float);
    }
  } // namespace codegen
} // namespace phantom
