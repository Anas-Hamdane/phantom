#include "irgen/Gen.hpp"
#include <cassert>
#include <cstring>

namespace phantom {
  namespace ir {
    Program Gen::gen() {
      // the only supported arch for now
      Target target{
        .arch = "x86_64",
        .kernel = "linux"
      };

      program.target = target;

      for (auto& stmt : ast) {
        generate_stmt(stmt);
      }

      return program;
    }

    void Gen::generate_stmt(std::unique_ptr<ast::Stmt>& stmt) {
      // clang-format off
      switch (stmt->index()) {
        case 0: generate_return(std::get<0>(*stmt));     break; // Return
        case 1: generate_expr(std::get<1>(*stmt)->expr); break; // Expmt
        case 2: declare_function(std::get<2>(*stmt));    break; // FnDecl
        case 3: define_function(std::get<3>(*stmt));     break; // FnDef
        default: unreachable();
      }
      // clang-format on
    }
    void Gen::generate_return(std::unique_ptr<ast::Return>& ast_rt) {
      if (!current_function) {
        printf("You messed up!\n");
        exit(1);
      }

      if (current_function->terminated) {
        printf("functions can't have more than one return\n");
        exit(1);
      }

      if (current_function->return_type.is_void && ast_rt->expr != nullptr) {
        printf("function does not return something has a return value\n");
        exit(1);
      }

      Return ret;
      if (ast_rt) {
        ret.value = generate_expr(ast_rt->expr);

        // check return type
        Type type = extract_value_type(ret.value);
        if (type.kind != current_function->return_type.kind) {
          printf("incorrect return type for function: %s\n", current_function->name.c_str());
          exit(1);
        }
      }

      current_function->terminator = ret;
      current_function->terminated = true;
    }
    Value Gen::generate_expr(std::unique_ptr<ast::Expr>& expr) {
      switch (expr->index()) {
        case 0: // IntLit
        {
          std::unique_ptr<ast::IntLit>& lit = std::get<0>(*expr);

          Constant constant;
          constant.type.kind = Type::Kind::Int;

          int64_t value = (int64_t)lit->value;
          if (value >= INT_MIN_VAL && value <= INT_MAX_VAL)
            constant.type.size = 4;
          else if (value >= LONG_MIN_VAL && value <= LONG_MAX_VAL)
            constant.type.size = 8;
          else {
            // TODO: better errors
            printf("Integer literal is too large to be represented in a data type\n");
            exit(1);
          }

          // int64_t
          constant.value = (int64_t)lit->value;
          return constant;
        }
        case 1: // FloatLit
        {
          std::unique_ptr<ast::FloatLit>& lit = std::get<1>(*expr);

          Constant constant;
          constant.type.kind = Type::Kind::Float;

          if (lit->value >= FLOAT_MIN_VAL && lit->value <= FLOAT_MAX_VAL)
            constant.type.size = 4;
          else if (lit->value >= DOUBLE_MIN_VAL && lit->value <= DOUBLE_MAX_VAL)
            constant.type.size = 8;
          else {
            // TODO: better errors
            printf("Float literal is too large to be represented in a data type\n");
            exit(1);
          }

          // double
          constant.value = lit->value;
          return constant;
        }
        case 2: // StrLit
        {
          todo();
        }
        case 3: // ArrLit
        {
          todo();
        }
        case 4: // Identifier
        {
          std::unique_ptr<ast::Identifier>& ide = std::get<4>(*expr);
          if (scope_vars.find(ide->name) == scope_vars.end()) {
            printf("Use of undeclared Identifier: %s\n", ide->name.c_str());
            exit(1);
          }

          return scope_vars[ide->name];
        }
        case 5: // BinOp
        {
          std::unique_ptr<ast::BinOp>& binop = std::get<5>(*expr);
          Value lhs = generate_expr(binop->lhs);
          Value rhs = generate_expr(binop->rhs);

          // Handle assignment as a special case
          if (binop->op == Token::Kind::Eq) {
            assert(lhs.index() == 1 && "can't assign to a non-variable destination");
            generate_assignment(std::get<1>(lhs), rhs);
            return rhs;
          }

          // basic constant-folding
          if (lhs.index() == 0 && rhs.index() == 0) {
            Constant lv = std::get<0>(lhs);
            Constant rv = std::get<0>(rhs);

            Constant result;
            result.type.size = std::max(lv.type.size, rv.type.size);

            if (lv.type.kind == Type::Kind::Float || rv.type.kind == Type::Kind::Float) {
              double lvalue = extract_double_constant(lv.value);
              double rvalue = extract_double_constant(rv.value);

              result.type.kind = Type::Kind::Float;
              result.value = calculate_double_constant(binop->op, lvalue, rvalue);
            } else {
              int64_t lvalue = extract_integer_constant(lv.value);
              int64_t rvalue = extract_integer_constant(rv.value);

              result.type.kind = Type::Kind::Int;
              result.value = calculate_integer_constant(binop->op, lvalue, rvalue);
            }

            return result;
          }

          // clang-format off
          BinOp::Op op;
          switch (binop->op) {
            case Token::Kind::Plus:  op = BinOp::Op::Add; break;
            case Token::Kind::Minus: op = BinOp::Op::Sub; break;
            case Token::Kind::Mul:   op = BinOp::Op::Mul; break;
            case Token::Kind::Div:   op = BinOp::Op::Div; break;
            default:                 unreachable();
          }
          // clang-format on

          Type lty = extract_value_type(lhs);
          Type rty = extract_value_type(rhs);

          Type type;
          type.size = lty.size > rty.size ? lty.size : rty.size;

          if (lty.kind == Type::Kind::Float || rty.kind == Type::Kind::Float)
            type.kind = Type::Kind::Float;
          else
            type.kind = Type::Kind::Int;

          cast_if_needed(lhs, lty, type);
          cast_if_needed(rhs, rty, type);

          // Binary operations store the result in physical register treated temporaries
          PhysReg dst;

          // if any side is itself a physical register (temporary), use it to store
          // the result value of the operation
          if (lhs.index() == 2)
            dst = std::get<2>(lhs);
          else if (rhs.index() == 2)
            dst = std::get<2>(rhs);

          // if not allocate a new one
          else
            dst = allocate_physical_register(type);

          // override the type to avoid using the `lhs` or `rhs` types
          dst.type = type;

          current_function->body.push_back(BinOp{
              .op = op,
              .lhs = lhs,
              .rhs = rhs,
              .dst = dst,
          });

          return dst;
        }
        case 6: // UnOp
        {
          // TODO: check this
          std::unique_ptr<ast::UnOp>& unop = std::get<6>(*expr);
          Value operand = generate_expr(unop->operand);

          UnOp::Op op;
          // clang-format off
          switch (unop->op) {
            case Token::Kind::Minus: op = UnOp::Op::Neg; break;
            case Token::Kind::Not:   op = UnOp::Op::Not; break;
            default:                 unreachable();
          }
          // clang-format on

          Type type = (operand.index() == 0) ? std::get<0>(operand).type : std::get<1>(operand).type;
          PhysReg dst = allocate_physical_register(type);

          current_function->body.push_back(UnOp{ .op = op, .operand = operand, .dst = dst });
          return dst;
        }
        case 7: // VarDecl
        {
          std::unique_ptr<ast::VarDecl>& decl = std::get<7>(*expr);

          if (scope_vars.find(decl->name) != scope_vars.end()) {
            printf("Redefinition of variable: %s\n", decl->name.c_str());
            exit(1);
          }

          Type type;
          Value value = {};
          bool initialized = false;

          if (decl->init) {
            value = generate_expr(decl->init);
            type = extract_value_type(value);
            initialized = true;
          }

          // override even if there's an initialized
          // Priority goes to the specified type
          if (decl->type)
            type = resolve_type(*decl->type);

          VirtReg reg = allocate_vritual_register(type);
          scope_vars[decl->name] = reg;

          Alloca alloca{ .type = type, .reg = reg };
          current_function->body.push_back(alloca);

          if (initialized) {
            generate_assignment(reg, value);
            return value;
          }

          return {};
        }
        case 8: // FnCall
        {
          todo();
        }
      }

      unreachable();
    }

    void Gen::define_function(std::unique_ptr<ast::FnDef>& ast_fn) {
      Function fn;
      fn.name = ast_fn->decl->name;

      if (ast_fn->decl->type)
        fn.return_type = resolve_type(*ast_fn->decl->type);
      else
        fn.return_type.is_void = true;

      fn.defined = true;

      auto old_scope_vars = scope_vars;
      nrid = 0;

      for (auto& param : ast_fn->decl->params) {
        if (scope_vars.find(param->name) != scope_vars.end()) {
          printf("Duplicated variable with the same name\n");
          exit(1);
        }

        assert(param->type != nullptr);
        Type type = resolve_type(*param->type);
        VirtReg reg = allocate_vritual_register(type);
        fn.params.push_back(reg);

        scope_vars[param->name] = reg;
      }

      current_function = &fn;
      for (auto& stmt : ast_fn->body) {
        generate_stmt(stmt);
      }

      scope_vars = old_scope_vars;
      program.funcs.push_back(fn);
    }
    void Gen::declare_function(std::unique_ptr<ast::FnDecl>& ast_decl) {
      Function fn;
      fn.name = ast_decl->name;

      if (ast_decl->type)
        fn.return_type = resolve_type(*ast_decl->type);
      else
        fn.return_type.is_void = true;

      fn.defined = false;

      auto old_scope_vars = scope_vars;
      nrid = 0;

      for (auto& param : ast_decl->params) {
        if (scope_vars.find(param->name) != scope_vars.end()) {
          printf("Duplicated variable with the same name\n");
          exit(1);
        }

        assert(param->type != nullptr);
        Type type = resolve_type(*param->type);
        VirtReg reg = allocate_vritual_register(type);
        fn.params.push_back(reg);

        scope_vars[param->name] = reg;
      }

      scope_vars = old_scope_vars;
      program.funcs.push_back(fn);
    }

    void Gen::generate_assignment(VirtReg& dst, Value& src) {
      Type dtype = dst.type;
      Type stype = extract_value_type(src);

      bool cast_needed = need_cast(dtype, stype);

      if (!cast_needed)
        return generate_store(dst, src);

      PhysReg reg = allocate_physical_register(dtype);
      generate_cast(src, reg, stype, dtype);
      src = reg;

      generate_store(dst, src);
    }
    void Gen::generate_store(std::variant<VirtReg, PhysReg> dst, Value src) {
      Store store{ .src = src, .dst = dst };
      current_function->body.push_back(store);
    }
    void Gen::generate_cast(Value& src, PhysReg dst, Type& stype, Type& dtype) {
      Instruction cast;

      if (stype.kind == Type::Kind::Int && dtype.kind == Type::Kind::Float) {
        if (dtype.size == 4)
          cast = Int2Float{ .value = src, .dst = dst };
        else if (dtype.size == 8)
          cast = Int2Double{ .value = src, .dst = dst };
      }

      else if (stype.kind == Type::Kind::Float && dtype.kind == Type::Kind::Float) {
        if (stype.size == 4 && dtype.size == 8)
          cast = Float2Double{ .value = src, .dst = dst };
        else if (stype.size == 8 && dtype.size == 4)
          cast = Double2Float{ .value = src, .dst = dst };
      }

      else { // Float -> Int

        // WARNING: make sure here the register is 32-bit (4-bytes)
        dst.name = get_register_by_size(dst.name.c_str(), 4);
        dst.type.size = 4;

        if (stype.size == 4)
          cast = Float2Int{ .value = src, .dst = dst };
        else if (stype.size == 8)
          cast = Double2Int{ .value = src, .dst = dst };
      }

      current_function->body.push_back(cast);
    }

    VirtReg Gen::allocate_vritual_register(Type& type) {
      VirtReg reg{
        .id = nrid++,
        .type = type
      };

      return reg;
    }
    PhysReg Gen::allocate_physical_register(Type& type) {
      std::string name;

      if (type.kind == Type::Kind::Int) {
        name = subreg_name((lubpr == 'C') ? "rax" : "rcx", type.size);
        lubpr = (lubpr == 'C') ? 'A' : 'C';
      } else {
        name = (lubfppr == '1') ? "xmm0" : "xmm1";
        lubfppr = (lubfppr == '1') ? '0' : '1';
      }

      PhysReg reg{
        .name = name,
        .type = type
      };

      return reg;
    }

    double Gen::extract_double_constant(std::variant<int64_t, double>& v) {
      if (v.index() == 0)
        return (double)std::get<0>(v);
      else
        return std::get<1>(v);
    }
    int64_t Gen::extract_integer_constant(std::variant<int64_t, double>& v) {
      if (v.index() == 0)
        return std::get<0>(v);
      else
        return (int64_t)std::get<1>(v);
    }
    double Gen::calculate_double_constant(Token::Kind op, double lv, double rv) {
      switch (op) {
        case Token::Kind::Plus:
          return lv + rv;
        case Token::Kind::Minus:
          return lv - rv;
        case Token::Kind::Mul:
          return lv * rv;
        case Token::Kind::Div:
          return lv / rv;
        default:
          unreachable();
      }
    }
    int64_t Gen::calculate_integer_constant(Token::Kind op, int64_t lv, int64_t rv) {
      // clang-format off
      switch (op) {
        case Token::Kind::Plus:  return lv + rv;
        case Token::Kind::Minus: return lv - rv;
        case Token::Kind::Mul:   return lv * rv;
        case Token::Kind::Div:   return lv / rv;
        default:                 unreachable();
      }
      // clang-format on
    }

    void Gen::cast_if_needed(Value& v, Type& vtype, Type& target) {
      if (!need_cast(vtype, target))
        return;

      if (v.index() == 2)
        return generate_cast(v, std::get<2>(v), vtype, target);

      PhysReg reg = allocate_physical_register(vtype);
      generate_cast(v, reg, vtype, target);
      v = reg;
    }
    bool Gen::need_cast(Type& dtype, Type& stype) {
      if (dtype.kind == stype.kind && dtype.size == stype.size)
        return false;

      // integers don't need cast
      if (dtype.kind == Type::Kind::Int && stype.kind == Type::Kind::Int)
        return false;

      return true;
    }
    Type Gen::extract_value_type(Value& value) {
      switch (value.index()) {
        case 0: // constant
          return std::get<0>(value).type;
        case 1: // VirtReg
          return std::get<1>(value).type;
        case 2: // PhysReg
          return std::get<2>(value).type;
        default:
          unreachable();
      }
    }
    Type Gen::resolve_type(phantom::Type& type) {
      if (type.kind == phantom::Type::Kind::FP) {
        uint size = type.bitwidth / 8;
        assert(size == 4 || size == 8 && "Floating points bitwidth must be either 4 or 8");
        return Type{ .kind = Type::Kind::Float, .size = size, .is_void = false };
      }

      else if (type.kind == phantom::Type::Kind::Int) {
        uint size = (type.bitwidth == 1) ? 1 : (type.bitwidth / 8);
        assert(size == 1 || size == 2 || size == 4 || size == 8 && "Integers bitwidth must be either 4 or 8");
        return Type{ .kind = Type::Kind::Int, .size = size, .is_void = false };
      }

      unreachable();
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
  } // namespace ir
} // namespace phantom
