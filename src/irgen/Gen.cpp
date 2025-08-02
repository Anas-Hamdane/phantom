#include "irgen/Gen.hpp"
#include <cassert>

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
        default: std::abort();
      }
      // clang-format on
    }
    void Gen::generate_return(std::unique_ptr<ast::Return>& ast_rt) {
      if (!current_function) {
        printf("You messed up!\n");
        exit(1);
      }

      if (current_function->terminated) {
        printf("blocks can't have more than one terminator\n");
        exit(1);
      }

      Return ret{
        .value = generate_expr(ast_rt->expr)
      };

      current_function->terminator = ret;
      current_function->terminated = true;
    }

    void Gen::define_function(std::unique_ptr<ast::FnDef>& ast_fn) {
      Function fn;
      fn.name = ast_fn->decl->name;
      fn.return_type = *(ast_fn->decl->type);
      fn.defined = true;

      auto old_scope_vars = scope_vars;
      nrid = 0;

      for (auto& param : ast_fn->decl->params) {
        if (scope_vars.find(param->name) != scope_vars.end()) {
          printf("Duplicated variable with the same name\n");
          exit(1);
        }

        Register reg = allocate_register(*param->type);
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
      fn.return_type = *(ast_decl->type);
      fn.defined = false;

      auto old_scope_vars = scope_vars;
      nrid = 0;

      for (auto& param : ast_decl->params) {
        if (scope_vars.find(param->name) != scope_vars.end()) {
          printf("Duplicated variable with the same name\n");
          exit(1);
        }

        Register reg = allocate_register(*param->type);
        fn.params.push_back(reg);

        scope_vars[param->name] = reg;
      }

      scope_vars = old_scope_vars;
      program.funcs.push_back(fn);
    }

    Value Gen::generate_expr(std::unique_ptr<ast::Expr>& expr) {
      switch (expr->index()) {
        case 0: // IntLit
        {
          std::unique_ptr<ast::IntLit>& lit = std::get<0>(*expr);

          Constant constant;
          constant.type.kind = Type::Kind::Int;

          if (((int)lit->value) >= INT_MIN_VAL && ((int)lit->value) <= INT_MAX_VAL)
            constant.type.bitwidth = 32;
          else if (((long long)lit->value) >= LONG_MIN_VAL && ((long long)lit->value) <= LONG_MAX_VAL)
            constant.type.bitwidth = 64;
          else {
            printf("Integer literal is too large to be represented in a data type\n");
            exit(1);
          }

          // uint64_t
          constant.value = lit->value;
          return constant;
        }
        case 1: // FloatLit
        {
          std::unique_ptr<ast::FloatLit>& lit = std::get<1>(*expr);

          Constant constant;
          constant.type.kind = Type::Kind::FP;
          if (lit->value >= FLOAT_MIN_VAL && lit->value <= FLOAT_MAX_VAL)
            constant.type.bitwidth = 32;
          else if (lit->value >= DOUBLE_MIN_VAL && lit->value <= DOUBLE_MAX_VAL)
            constant.type.bitwidth = 64;
          else {
            printf("Float literal is too large to be represented in a data type\n");
            exit(1);
          }

          // double
          constant.value = lit->value;
          return constant;
        }
        case 2: // StrLit
        {
          // TODO:
        }
        case 3: // ArrLit
        {
          // TODO:
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

          if (binop->op == Token::Kind::Eq) {
            assert(lhs.index() == 1 && "can't assign to a non-variable destination\n");

            create_store(std::get<1>(lhs), rhs);
            return rhs;
          }

          BinOp::Op op;
          // clang-format off
          switch (binop->op) {
            case Token::Kind::Plus:  op = BinOp::Op::Add; break;
            case Token::Kind::Minus: op = BinOp::Op::Sub; break;
            case Token::Kind::Mul:   op = BinOp::Op::Mul; break;
            case Token::Kind::Div:   op = BinOp::Op::Div; break;
            default:                std::abort();
          }
          // clang-format on

          Type lty = (lhs.index() == 0) ? std::get<0>(lhs).type : std::get<1>(lhs).type;
          Type rty = (rhs.index() == 0) ? std::get<0>(rhs).type : std::get<1>(rhs).type;

          Type type;
          type.bitwidth = (lty.bitwidth > rty.bitwidth) ? lty.bitwidth : rty.bitwidth;
          if (lty.kind == Type::Kind::FP || rty.kind == Type::Kind::FP)
            type.kind = Type::Kind::FP;
          else if (lty.kind == Type::Kind::Int || rty.kind == Type::Kind::Int)
            type.kind = Type::Kind::Int;
          else
            type.kind = Type::Kind::UnsInt;

          Register dst = allocate_register(type);
          current_function->body.push_back(BinOp{ .op = op, .lhs = lhs, .rhs = rhs, .dst = dst });
          return dst;
        }
        case 6: // UnOp
        {
          std::unique_ptr<ast::UnOp>& unop = std::get<6>(*expr);
          Value operand = generate_expr(unop->operand);

          UnOp::Op op;
          // clang-format off
          switch (unop->op) {
            case Token::Kind::Minus: op = UnOp::Op::Neg; break;
            case Token::Kind::Not:   op = UnOp::Op::Not; break;
            default:                 std::abort();
          }
          // clang-format on

          Type type = (operand.index() == 0) ? std::get<0>(operand).type : std::get<1>(operand).type;
          Register dst = allocate_register(type);

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
            initialized = true;
            type = (value.index() == 0) ? std::get<0>(value).type : std::get<1>(value).type;
          }

          // override even if there's an initialized
          // Priority goes to the specified type
          if (decl->type)
            type = *decl->type;

          Register reg = allocate_register(type);
          scope_vars[decl->name] = reg;

          Alloca alloca{ .type = type, .reg = reg };
          current_function->body.push_back(alloca);

          if (initialized) {
            Store store{ .src = value, .dst = reg };
            current_function->body.push_back(store);
            return value;
          }

          return {};
        }
        case 8: // FnCall
        {
          // TODO:
        }
      }

      std::abort();
    }
    void Gen::create_store(Register dst, Value src) {
      Store store;
      store.dst = dst;
      store.src = src;

      current_function->body.push_back(store);
    }
    Register Gen::allocate_register(Type type) {
      Register reg{
        .id = nrid++,
        .type = type
      };

      return reg;
    }
  } // namespace ir
} // namespace phantom
