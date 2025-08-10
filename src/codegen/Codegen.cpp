#include "codegen/Codegen.hpp"
#include "common.hpp"
#include <cassert>
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

        scope_vars[param.id] = Variable{ .type = param.type, .offset = offset };
      }

      for (size_t i = tmp; i < params_size; ++i) {
        ir::VirtReg param = fn.params.at(i);

        const char suff = integer_suffix(param.type.size);
        const char* reg = get_register_by_size("rax", param.type.size);

        utils::appendf(&output, "  mov%c    %zu(%%rbp), %%%s\n", suff, ((i - tmp) + 2) * 8, reg);
        utils::appendf(&output, "  mov%c    %%%s, -%zu(%%rbp)\n", suff, reg, offset + param.type.size);
        offset += param.type.size;

        scope_vars[param.id] = Variable{ .type = param.type, .offset = offset };
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
          scope_vars[alloca.reg.id] = Variable{ .type = alloca.type, .offset = offset };
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
                  return store_constant_in_memory(constant, memory);
                }
                case 1: // VirtReg
                {
                  ir::VirtReg value = std::get<1>(store.src);
                  return store_memory_in_memory(memory, value);
                }
                case 2: // PhysReg
                {
                  ir::PhysReg value = std::get<2>(store.src);
                  return store_register_in_memory(value, memory);
                }
                default:
                  unreachable();
              }
            }
            case 1: // PhysReg
            {
              ir::PhysReg dst = std::get<1>(store.dst);
              switch (store.src.index()) {
                case 0: // Constant
                {
                  ir::Constant constant = std::get<0>(store.src);
                  return store_constant_in_register(constant, dst);
                }
                case 1: // VirtReg
                {
                  ir::VirtReg memory = std::get<1>(store.src);
                  return store_memory_in_register(memory, dst);
                }
                case 2: // PhysReg
                {
                  ir::PhysReg src = std::get<2>(store.src);
                  return store_register_in_register(dst, src);
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
            // NOTE: Constant + Constant is handled in the IR generation
            case ir::BinOp::Op::Add: // addition
            {
              switch (binop.lhs.index()) {
                case 0: // Constant
                {
                  ir::Constant constant = std::get<0>(binop.lhs);

                  switch (binop.rhs.index()) {
                    case 1: // VirtReg
                    {
                      ir::VirtReg memory = std::get<1>(binop.rhs);
                      store_constant_in_register(constant, binop.dst);
                      return add_memory_to_register(memory, binop.dst);
                    }
                    case 2: // PhysReg
                    {
                      ir::PhysReg src = std::get<2>(binop.rhs);
                      add_constant_to_register(constant, src);

                      if (src.rid != binop.dst.rid || src.type.kind != binop.dst.type.kind)
                        store_register_in_register(src, binop.dst);

                      return;
                    }
                  }
                  unreachable();
                }
                case 1: // VirtReg
                {
                  ir::VirtReg memory = std::get<1>(binop.lhs);

                  switch (binop.rhs.index()) {
                    case 0: // Constant
                    {
                      ir::Constant constant = std::get<0>(binop.rhs);
                      store_memory_in_register(memory, binop.dst);
                      return add_constant_to_register(constant, binop.dst);
                    }
                    case 1: // VirtReg
                    {
                      ir::VirtReg src = std::get<1>(binop.rhs);
                      store_memory_in_register(memory, binop.dst);
                      return add_memory_to_register(src, binop.dst);
                    }
                    case 2: // PhysReg
                    {
                      ir::PhysReg src = std::get<2>(binop.rhs);
                      add_memory_to_register(memory, src);

                      if (src.rid != binop.dst.rid || src.type.kind != binop.dst.type.kind)
                        store_register_in_register(src, binop.dst);

                      return;
                    }
                  }
                  unreachable();
                }
                case 2: // PhysReg
                {
                  ir::PhysReg left = std::get<2>(binop.lhs);

                  switch (binop.rhs.index()) {
                    case 0: // Constant
                    {
                      ir::Constant constant = std::get<0>(binop.rhs);
                      add_constant_to_register(constant, left);

                      if (left.rid != binop.dst.rid || left.type.kind != binop.dst.type.kind)
                        store_register_in_register(left, binop.dst);

                      return;
                    }
                    case 1: // VirtReg
                    {
                      ir::VirtReg memory = std::get<1>(binop.rhs);
                      add_memory_to_register(memory, left);

                      if (left.rid != binop.dst.rid || left.type.kind != binop.dst.type.kind)
                        store_register_in_register(left, binop.dst);

                      return;
                    }
                    case 2: // PhysReg
                    {
                      ir::PhysReg right = std::get<2>(binop.rhs);
                      add_register_to_register(right, left);

                      if (left.rid != binop.dst.rid || left.type.kind != binop.dst.type.kind)
                        store_register_in_register(left, binop.dst);

                      return;
                    }
                  }
                  unreachable();
                }
              }
              unreachable();
            }
            case ir::BinOp::Op::Sub: // substraction
            {
              switch (binop.lhs.index()) {
                case 0: // Constant
                {
                  ir::Constant constant = std::get<0>(binop.lhs);

                  switch (binop.rhs.index()) {
                    case 1: // VirtReg
                    {
                      ir::VirtReg memory = std::get<1>(binop.rhs);
                      store_constant_in_register(constant, binop.dst);
                      return sub_memory_from_register(memory, binop.dst);
                    }
                    case 2: // PhysReg
                    {
                      ir::PhysReg right = std::get<2>(binop.rhs);

                      if (right.rid != binop.dst.rid || right.type.kind != binop.dst.type.kind) {
                        store_constant_in_register(constant, binop.dst);
                        return sub_register_from_register(right, binop.dst);
                      }

                      // else
                      ir::PhysReg ir = binop.dst;
                      ir.rid = TR_INDEX; // using the temporary register

                      store_constant_in_register(constant, ir);
                      sub_register_from_register(right, ir);

                      return store_register_in_register(ir, binop.dst);
                    }
                  }
                  unreachable();
                }
                case 1: // VirtReg
                {
                  ir::VirtReg memory = std::get<1>(binop.lhs);

                  switch (binop.rhs.index()) {
                    case 0: // Constant
                    {
                      ir::Constant constant = std::get<0>(binop.rhs);
                      store_memory_in_register(memory, binop.dst);
                      return sub_constant_from_register(constant, binop.dst);
                    }
                    case 1: // VirtReg
                    {
                      ir::VirtReg src = std::get<1>(binop.rhs);
                      store_memory_in_register(memory, binop.dst);
                      return sub_memory_from_register(src, binop.dst);
                    }
                    case 2: // PhysReg
                    {
                      ir::PhysReg right = std::get<2>(binop.rhs);

                      if (right.rid != binop.dst.rid || right.type.kind != binop.dst.type.kind) {
                        store_memory_in_register(memory, binop.dst);
                        return sub_register_from_register(right, binop.dst);
                      }

                      // else
                      ir::PhysReg ir = binop.dst;
                      ir.rid = TR_INDEX; // using the temporary register

                      store_memory_in_register(memory, ir);
                      sub_register_from_register(right, ir);

                      return store_register_in_register(ir, binop.dst);
                    }
                  }
                  unreachable();
                }
                case 2: // PhysReg
                {
                  ir::PhysReg left = std::get<2>(binop.lhs);

                  switch (binop.rhs.index()) {
                    case 0: // Constant
                    {
                      ir::Constant constant = std::get<0>(binop.rhs);

                      if (left.rid != binop.dst.rid || left.type.kind != binop.dst.type.kind)
                        store_register_in_register(left, binop.dst);

                      return sub_constant_from_register(constant, binop.dst);
                    }
                    case 1: // VirtReg
                    {
                      ir::VirtReg memory = std::get<1>(binop.rhs);

                      if (left.rid != binop.dst.rid || left.type.kind != binop.dst.type.kind)
                        store_register_in_register(left, binop.dst);

                      return sub_memory_from_register(memory, binop.dst);
                    }
                    case 2: // PhysReg
                    {
                      ir::PhysReg right = std::get<2>(binop.rhs);
                      sub_register_from_register(right, left);

                      if (left.rid != binop.dst.rid || left.type.kind != binop.dst.type.kind)
                        store_register_in_register(left, binop.dst);

                      return;
                    }
                  }
                  unreachable();
                }
              }
              unreachable();
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
          const char* dst = physical_register_name(cvt.dst);
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

              DataLabel label = constant_label((float)value, Directive::Kind::Float);
              utils::appendf(&output, "  movss   %s(%%rip), %%%s\n", label.name.c_str(), dst);
              break;
            }
            case 1: // VirtReg
            {
              Variable var = scope_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtsi2ss -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              const char* prn = physical_register_name(pr);

              if (pr.type.size < 4) {
                const char* ir = get_register_by_size(prn, 4);
                const char prs = type_suffix(pr.type);

                utils::appendf(&output, "  movs%cl  %%%s, %%%s\n", prs, prn, ir);
                prn = ir;
              }

              utils::appendf(&output, "  cvtsi2ss %%%s, %%%s\n", prn, dst);
              break;
            }
          }
          break;
        }
        case 5: // Int2Double
        {
          ir::Int2Double cvt = std::get<5>(inst);
          const char* dst = physical_register_name(cvt.dst);

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

              DataLabel label = constant_label((double)value, Directive::Kind::Double);
              utils::appendf(&output, "  movsd   %s(%%rip), %%%s\n", label.name.c_str(), dst);
              break;
            }
            case 1: // VirtReg
            {
              Variable var = scope_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtsi2sd -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              const char* prn = physical_register_name(pr);

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
          const char* dst = physical_register_name(cvt.dst);
          switch (cvt.value.index()) {
            case 0: // Constant
            {
              torevise();
              break;
            }
            case 1: // VirtReg
            {
              Variable var = scope_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtss2si -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              utils::appendf(&output, "  cvtss2si %%%s, %%%s\n", physical_register_name(pr), dst);
              break;
            }
          }
          break;
        }
        case 7: // Float2Double
        {
          ir::Float2Double cvt = std::get<7>(inst);
          const char* dst = physical_register_name(cvt.dst);
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

              DataLabel label = constant_label(value, Directive::Kind::Double);
              utils::appendf(&output, "  movsd   %s(%%rip), %%%s\n", label.name.c_str(), dst);
              break;
            }
            case 1: // VirtReg
            {
              Variable var = scope_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtss2sd -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              utils::appendf(&output, "  cvtss2sd %%%s, %%%s\n", physical_register_name(pr), dst);
              break;
            }
          }
          break;
        }
        case 8: // Double2Int
        {
          ir::Double2Int cvt = std::get<8>(inst);
          const char* dst = physical_register_name(cvt.dst);
          switch (cvt.value.index()) {
            case 0: // Constant
            {
              torevise();
              break;
            }
            case 1: // VirtReg
            {
              Variable var = scope_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtsd2si -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              utils::appendf(&output, "  cvtsd2si %%%s, %%%s\n", physical_register_name(pr), dst);
              break;
            }
          }
          break;
        }
        case 9: // Double2Float
        {
          ir::Double2Float cvt = std::get<9>(inst);
          const char* dst = physical_register_name(cvt.dst);
          switch (cvt.value.index()) {
            case 0: // Constant
            {
              torevise();
            }
            case 1: // VirtReg
            {
              Variable var = scope_vars[std::get<1>(cvt.value).id];
              utils::appendf(&output, "  cvtsd2ss -%zu(%%rbp), %%%s\n", var.offset, dst);
              break;
            }
            case 2: // PhysReg
            {
              ir::PhysReg pr = std::get<2>(cvt.value);
              utils::appendf(&output, "  cvtsd2ss %%%s, %%%s\n", physical_register_name(pr), dst);
              break;
            }
          }
          break;
        }
        case 10: // IntExtend
        {
          ir::IntExtend extend = std::get<10>(inst);
          const char* drn = physical_register_name(extend.dst);
          const char drs = type_suffix(extend.dst.type);

          switch (extend.value.index()) {
            case 1: // VirtReg
            {
              Variable variable = scope_vars[std::get<1>(extend.value).id];
              const char vs = type_suffix(variable.type);
              const size_t vo = variable.offset;

              utils::appendf(&output, "  movs%c%c  -%zu(%%rbp), %%%s\n", vs, drs, vo, drn);
              return;
            }
            case 2: // PhysReg
            {
              ir::PhysReg reg = std::get<2>(extend.value);
              const char* rn = physical_register_name(reg);
              const char rs = type_suffix(reg.type);

              utils::appendf(&output, "  movs%c%c  %%%s, %%%s\n", rs, drs, rn, drn);
              return;
            }
          }
          unreachable();
        }
      }
    }
    void Gen::generate_data() {
      // TODO: reformat
      utils::append(&output, "# data\n");

      for (auto element : floats_data) {
        DataLabel label = element.second;
        utils::appendf(&output, "%s:\n", label.name.c_str());

        for (auto dir : label.dirs) {
          float value = std::get<1>(dir.data);
          utils::appendf(&output, "  .float  %f\n", value);
        }
      }

      for (auto element : doubles_data) {
        DataLabel label = element.second;
        utils::appendf(&output, "%s:\n", label.name.c_str());

        for (auto dir : label.dirs) {
          double value = std::get<2>(dir.data);
          utils::appendf(&output, "  .double  %lf\n", value);
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

                  DataLabel label = constant_label(value, kind);
                  utils::appendf(&output, "  movs%c   %s(%%rip), %%%s\n", ret_suff, label.name.c_str(), ret_reg);
                }
              }

              break;
            }
            case 1: // Register
            {
              Variable value = scope_vars[std::get<1>(ret.value).id];
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
    DataLabel Gen::constant_label(std::variant<double, std::string> value, Directive::Kind kind) {
      switch (kind) {
        case Directive::Kind::Float: {
          assert(value.index() == 0);
          float fv = std::get<0>(value);

          if (floats_data.find(fv) != floats_data.end())
            return floats_data[fv];

          DataLabel label;
          label.dirs.push_back(Directive{ .data = fv, .kind = kind });
          label.name = ".CSTS" + std::to_string(constants_size++);
          floats_data[fv] = label;
          return label;
        }
        case Directive::Kind::Double: {
          assert(value.index() == 0);
          double dv = std::get<0>(value);

          if (doubles_data.find(dv) != doubles_data.end())
            return doubles_data[dv];

          DataLabel label;
          label.dirs.push_back(Directive{ .data = dv, .kind = kind });
          label.name = ".CSTS" + std::to_string(constants_size++);
          doubles_data[dv] = label;
          return label;
        }
        case Directive::Kind::Asciz: {
          todo();
        }
        case Directive::Kind::Long: {
          todo();
        }
      }
      unreachable();
    }

    void Gen::store_constant_in_memory(ir::Constant& constant, ir::VirtReg& memory) {
      Variable variable = scope_vars[memory.id];
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

      DataLabel label = constant_label(v, kind);

      utils::appendf(&output, "  movs%c   %s(%%rip), %%xmm0\n", ds, label.name.c_str());
      utils::appendf(&output, "  movs%c   %%xmm0, -%zu(%%rbp)\n", ds, vo);
    }
    void Gen::store_register_in_memory(ir::PhysReg& reg, ir::VirtReg& memory) {
      Variable variable = scope_vars[memory.id];
      const char* rn = physical_register_name(reg);
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
    void Gen::store_memory_in_memory(ir::VirtReg& src, ir::VirtReg& dst) {
      Variable variable = scope_vars[dst.id];
      Variable dst_var = scope_vars[src.id];

      // intermediate register
      const char* ir = get_register_by_size("rdx", dst_var.type.size);
      char* mov;

      if (is_float(dst_var.type) || is_float(variable.type))
        mov = generate_floating_point_move(dst_var.type);
      else
        mov = generate_integer_move(dst_var.type, dst_var.type);

      utils::appendf(&output, "  %-7s -%zu(%%rbp), %%%s\n", mov, dst_var.offset, ir);
      free(mov);

      if (is_float(dst_var.type) || is_float(variable.type))
        mov = generate_floating_point_move(variable.type);
      else
        mov = generate_integer_move(dst_var.type, variable.type);

      utils::appendf(&output, "  %-7s %%%s, -%zu(%%rbp)\n", mov, ir, variable.offset);
      free(mov);
    }
    void Gen::store_constant_in_register(ir::Constant& constant, ir::PhysReg& reg) {
      const char* name = physical_register_name(reg);
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

      DataLabel label = constant_label(v, kind);

      utils::appendf(&output, "  movs%c   %s(%%rip), %%%s\n", ds, label.name.c_str(), name);
    }
    void Gen::store_register_in_register(ir::PhysReg& src, ir::PhysReg& dst) {
      const char* rn = physical_register_name(dst);
      const char* vn = physical_register_name(src);

      if (strcmp(rn, vn) == 0)
        return;

      char* mov;
      if (is_float(src.type) || is_float(dst.type))
        mov = generate_floating_point_move(src.type);
      else
        mov = generate_integer_move(src.type, dst.type);

      utils::appendf(&output, "  %-7s %%%s, %%%s\n", mov, vn, rn);
      free(mov);
    }
    void Gen::store_memory_in_register(ir::VirtReg& memory, ir::PhysReg& reg) {
      Variable variable = scope_vars[memory.id];
      const size_t vo = variable.offset;
      const char* rn = physical_register_name(reg);

      char* mov;
      if (is_float(reg.type) || is_float(variable.type))
        mov = generate_floating_point_move(reg.type);
      else
        mov = generate_integer_move(variable.type, reg.type);

      utils::appendf(&output, "  %-7s -%zu(%%rbp), %%%s\n", mov, vo, rn);
      free(mov);
    }

    void Gen::add_constant_to_register(ir::Constant& constant, ir::PhysReg& reg) {
      const char* rn = physical_register_name(reg);
      const char rs = type_suffix(reg.type);

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

          DataLabel label = constant_label(v, kind);
          utils::appendf(&output, "  adds%c    %s(%%rip), %%%s\n", rs, label.name.c_str(), rn);
          return;
        }
      }
      unreachable();
    }
    void Gen::add_register_to_register(ir::PhysReg& src, ir::PhysReg& dst) {
      const char* dn = physical_register_name(dst);
      const char* vn = physical_register_name(src);

      const char ds = type_suffix(dst.type);
      const char* extra = is_float(dst.type) ? "s" : "";

      utils::appendf(&output, "  add%s%c    %%%s, %%%s\n", extra, ds, vn, dn);
    }
    void Gen::add_memory_to_register(ir::VirtReg& memory, ir::PhysReg& reg) {
      Variable variable = scope_vars[memory.id];
      const size_t vo = variable.offset;

      const char* dn = physical_register_name(reg);
      const char ds = type_suffix(reg.type);

      const char* extra = is_float(reg.type) ? "s" : "";
      utils::appendf(&output, "  add%s%c    -%zu(%%rbp), %%%s\n", extra, ds, vo, dn);
    }

    void Gen::sub_constant_from_register(ir::Constant& constant, ir::PhysReg& reg) {
      const char* rn = physical_register_name(reg);
      const char rs = type_suffix(reg.type);

      switch (constant.value.index()) {
        case 0: // int64_t
        {
          int64_t v = std::get<0>(constant.value);
          if (v == 0) return;
          utils::appendf(&output, "  sub%c    $%lu, %%%s\n", rs, v, rn);
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

          DataLabel label = constant_label(v, kind);
          utils::appendf(&output, "  subs%c   %s(%%rip), %%%s\n", rs, label.name.c_str(), rn);
          return;
        }
      }
      unreachable();
    }
    void Gen::sub_register_from_register(ir::PhysReg& src, ir::PhysReg& dst) {
      const char* dn = physical_register_name(dst);
      const char* vn = physical_register_name(src);

      const char ds = type_suffix(dst.type);
      const char* extra = is_float(dst.type) ? "s" : "";

      utils::appendf(&output, "  sub%s%c    %%%s, %%%s\n", extra, ds, vn, dn);
    }
    void Gen::sub_memory_from_register(ir::VirtReg& memory, ir::PhysReg& reg) {
      Variable variable = scope_vars[memory.id];
      const size_t vo = variable.offset;

      const char* dn = physical_register_name(reg);
      const char ds = type_suffix(reg.type);

      const char* extra = is_float(reg.type) ? "s" : "";
      utils::appendf(&output, "  sub%s%c    -%zu(%%rbp), %%%s\n", extra, ds, vo, dn);
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
    const char* Gen::physical_register_name(ir::PhysReg& pr) {
      if (is_float(pr.type))
        return float_registers[pr.rid];

      return get_register_by_size(integer_registers[pr.rid], pr.type.size);
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
