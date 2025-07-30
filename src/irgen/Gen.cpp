#include <irgen/Gen.hpp>

namespace phantom {
  namespace ir {
    Program Gen::gen() {
      program.funcs.init();
      program.globals.init();

      for (Stmt& stmt : ast) {
        generate_stmt(stmt);
      }

      return program;
    }

    void Gen::generate_stmt(Stmt& stmt) {
      // clang-format off
      switch (stmt.kind) {
        case StmtKind::Invalid: perror("Invalid statements are not allowed in IR generation phase\n"); exit(1);
        case StmtKind::FnDecl: declare_function(stmt.data.fn_decl); break;
        case StmtKind::FnDef: generate_function(stmt.data.fn_def); break;
        case StmtKind::Return: generate_return(stmt.data.ret); break;
        case StmtKind::Expmt: generate_expr(*stmt.data.expmt.expr); break;
        default: printf("TODO\n"); exit(1);
      }
      // clang-format on
    }
    void Gen::generate_function(phantom::FnDef& ast_fn) {
      Function fn;
      fn.name = ast_fn.declaration->name;
      fn.return_type = ast_fn.declaration->type->type;
      fn.blocks.init();
      fn.params.init();
      fn.defined = true;
      fn.externed = false;

      auto old_vars = local_vars;
      nrid = 0;
      nbid = 0;

      for (auto& param : ast_fn.declaration->params) {
        if (local_vars.find(param.ide->name) != local_vars.end()) {
          printf("Duplicated variable with the same name\n");
          exit(1);
        }

        Register reg = allocate(param.type->type);
        fn.params.push(reg);
        local_vars[param.ide->name] = reg;
      }

      // entry block
      BasicBlock entry;
      entry.id = nbid;
      entry.insts.init();
      fn.blocks.push(entry);

      current_function = &fn;
      current_block = &fn.blocks.at(nbid);
      nbid++;

      for (Stmt& stmt : ast_fn.body) {
        generate_stmt(stmt);
      }

      local_vars = old_vars;
      program.funcs.push(fn);
    }
    void Gen::generate_return(phantom::Return& rt) {
      if (!current_function || !current_block) {
        printf("You messed up!\n");
        exit(1);
      }

      if (current_block->terminated) {
        printf("blocks can't have more than one terminator\n");
        exit(1);
      }

      Return ret{
        .value = generate_expr(*rt.expr)
      };

      current_block->terminator = {
        .kind = TermKind::Return,
        .data = { .ret = ret }
      };
      current_block->terminated = true;
    }
    void Gen::declare_function(phantom::FnDecl& ast_fn) {
      Function fn;
      fn.name = ast_fn.name;
      fn.return_type = ast_fn.type->type;
      fn.defined = false;
      fn.externed = false;

      program.funcs.push(fn);
    }

    Value Gen::generate_expr(Expr& expr) {
      //clang-format off
      switch (expr.kind) {
        case ExprKind::IntLit: {
          uint64_t value = expr.data.int_lit.value;
          Type type = proper_int_type(value);

          Constant con{
            .type = type,
            .value = { .int_val = value }
          };

          return Value{
            .kind = ValueKind::Constant,
            .value = { .con = con }
          };
        }
        case ExprKind::FloatLit: {
          double value = expr.data.float_lit.value;
          Type type = proper_float_type(value);

          Constant con{
            .type = type,
            .value = { .float_val = value }
          };

          return Value{
            .kind = ValueKind::Constant,
            .value = { .con = con }
          };
        }
        case ExprKind::Identifier: {
          Identifier ide = expr.data.ide;
          if (local_vars.find(ide.name) == local_vars.end()) {
            printf("Use of undeclared Identifier: %s\n", ide.name);
            exit(1);
          }

          Register src = local_vars[ide.name];
          Register dst = allocate(src.type);

          Load load{ .src = src, .dst = dst };
          current_block->insts.push({ .kind = InstrKind::Load, .inst = { .load = load } });

          return Value{ .kind = ValueKind::Register, .value = { .reg = dst } };
        }
        case ExprKind::BinOp: {
          phantom::BinOp ast_binop = expr.data.binop;
          Value lhs = generate_expr(*ast_binop.lhs);
          Value rhs = generate_expr(*ast_binop.rhs);
          BinOp::Op op = binop_op(ast_binop.op);

          Type type = binop_type(lhs, rhs);
          Register dst = allocate(type);

          BinOp binop{ .op = op, .lhs = lhs, .rhs = rhs, .dst = dst };
          current_block->insts.push({ .kind = InstrKind::BinOp, .inst = { .binop = binop } });

          return Value{ .kind = ValueKind::Register, .value = { .reg = dst } };
        }
        case ExprKind::UnOp: {
          phantom::UnOp ast_unop = expr.data.unop;
          Value operand = generate_expr(*ast_unop.operand);
          UnOp::Op op = unop_op(ast_unop.op);

          Type type = value_type(operand);
          Register dst = allocate(type);

          UnOp unop{ .op = op, .operand = operand, .dst = dst };
          current_block->insts.push({ .kind = InstrKind::UnOp, .inst = { .unop = unop } });

          return Value{ .kind = ValueKind::Register, .value = { .reg = dst } };
        }
        case ExprKind::VarDecl: {
          phantom::VarDecl ast_vardecl = expr.data.var_decl;
          const char* name = ast_vardecl.ide->name;

          if (local_vars.find(name) != local_vars.end()) {
            printf("Redefinition of variable: %s\n", name);
            exit(1);
          }
          if (!ast_vardecl.type && !ast_vardecl.value) {
            printf("Error: could not retrieve variable type: %s\n", name);
            exit(1);
          }

          if (ast_vardecl.value) {
            Value v = generate_expr(*ast_vardecl.value);
            Type type;

            if (ast_vardecl.type)
              type = ast_vardecl.type->type;
            else
              type = value_type(v);

            Register reg = allocate(type);
            Alloca alloca{ .type = type, .reg = reg };
            current_block->insts.push({ .kind = InstrKind::Alloca, .inst = { .alloca = alloca } });

            Store store{ .src = v, .dst = reg };
            current_block->insts.push({ .kind = InstrKind::Store, .inst = { .store = store } });

            return v;
          }

          Type type = ast_vardecl.type->type;
          Register reg = allocate(type);

          Alloca alloca{ .type = type, .reg = reg };
          current_block->insts.push({ .kind = InstrKind::Alloca, .inst = { .alloca = alloca } });

          return Value{
            .kind = ValueKind::Constant, .value = { .con = { .type = Type::Int, .value = { .int_val = 0 } } }
          };
        }
        default:
          printf("todo\n");
          exit(1);
      }
      // clang-format on
    }

    Register Gen::allocate(Type type) {
      Register reg{
        .id = nrid++,
        .type = type
      };

      return reg;
    }
    Type Gen::proper_int_type(uint64_t value) {
      if (value >= INT_MIN_VAL && value <= INT_MAX_VAL)
        return Type::Int;

      return Type::Long;
    }
    Type Gen::proper_float_type(double value) {
      if (value >= FLOAT_MIN_VAL && value <= FLOAT_MAX_VAL)
        return Type::Float;

      return Type::Double;
    }
    Type Gen::value_type(Value& v) {
      return (v.kind == ValueKind::Constant) ? v.value.con.type : v.value.reg.type;
    }
    Type Gen::binop_type(Value& lhs, Value& rhs) {
      Type lty = value_type(lhs);
      Type rty = value_type(rhs);

      uint lsz = (uint)lty;
      uint rsz = (uint)rty;

      return (lsz > rsz) ? lty : rty;
    }
    BinOp::Op Gen::binop_op(Token::Kind op) {
      // clang-format off
      switch (op) {
        case Token::Kind::Plus:  return BinOp::Op::Add;
        case Token::Kind::Minus: return BinOp::Op::Sub;
        case Token::Kind::Mul:   return BinOp::Op::Mul;
        case Token::Kind::Div:   return BinOp::Op::Div;
        default: printf("todo\n"); exit(1);
      }
      // clang-format on
    }
    UnOp::Op Gen::unop_op(Token::Kind op) {
      // clang-format off
      switch (op) {
        case Token::Kind::Minus:  return UnOp::Op::Neg;
        case Token::Kind::Not:    return UnOp::Op::Not;
        default: printf("todo\n"); exit(1);
      }
      // clang-format on
    }
  } // namespace ir
} // namespace phantom
