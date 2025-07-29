#include <codegen/Codegen.hpp>
#include <cstring>

namespace phantom {
  namespace codegen {
    const char* Codegen::codegen() {
      str::append(&output, ".text\n");
      for (StmtRef& stmt_ref : this->ast) {
        Stmt stmt = stmt_area.get(stmt_ref);
        generate_stmt(stmt);
      }

      return output.content;
    }

    void Codegen::generate_stmt(Stmt& stmt) {
      switch (stmt.kind) {
        case StmtKind::Invalid:
          printf("You messed up!\n");
          exit(1);
        case StmtKind::FnDecl:
          declare_function(stmt.data.fn_decl);
          break;
        case StmtKind::FnDef:
          generate_function(stmt.data.fn_def);
          break;
        case StmtKind::Return:
          generate_return(stmt.data.ret);
          break;
        case StmtKind::Expmt:
          generate_expression(stmt.data.expmt);
          break;
        default:
          printf("Well, next time\n");
          exit(1);
      }
    }
    void Codegen::generate_function(FnDef& fn) {
      FnDecl dec = stmt_area.get(fn.declaration).data.fn_decl;
      declare_function(dec);

      current_function = &fn_table[dec.name];

      str::appendf(&output, ".globl %s\n", current_function->name);
      str::append(&output, ".p2align 4\n");
      str::appendf(&output, ".type %s, @function\n", current_function->name);
      str::appendf(&output, "%s:\n", current_function->name);

      str::append(&output, "    pushq   %rbp\n");
      str::append(&output, "    movq    %rsp, %rbp\n");

      // TODO: generate body
      for (size_t i = 0; i < fn.body_len; ++i) {
        Stmt stmt = stmt_area.get(fn.body[i]);
        generate_stmt(stmt);
      }

      if (!current_function->terminated) {
        str::append(&output, "    popq    %rbp\n");
        str::append(&output, "    ret\n");
      }

      current_function = nullptr;
    }
    void Codegen::generate_return(Return& ret) {
      // if (ret.expr == 0)
      // TODO: handle void return

      if (current_function == nullptr)
        printf("`return` statements are not allowed outside of functions MF\n");

      Expr value = expr_area.get(ret.expr);
      load_to_reg("rax", value);

      str::append(&output, "    popq    %rbp\n");
      str::append(&output, "    ret\n");

      if (current_function)
        current_function->terminated = true;
    }
    void Codegen::generate_expression(Expmt& expmt) {
      Expr expr = expr_area.get(expmt.expr);

      switch (expr.kind) {
        case ExprKind::Invalid:
          printf("You messed up!\n");
          exit(1);
        case ExprKind::VarDecl:
          generate_vardecl(expr.data.var_decl);
          break;
        case ExprKind::BinOp:
          generate_binop(expr.data.binop);
          break;
        default:
          printf("TODO\n");
      }
    }

    void Codegen::generate_vardecl(VarDecl& vardecl) {
      declare_variable(vardecl);

      if (vardecl.value != 0)
        asgn(vardecl.ide, vardecl.value);
    }
    void Codegen::generate_binop(BinOp& binop) {
      switch (binop.op) {
        case Token::Kind::Eq:
          return asgn(binop.left, binop.right);
        default:
          printf("TODO\n");
          exit(1);
      }
    }

    void Codegen::declare_function(FnDecl& decl) {
      Function fn;

      fn.name = decl.name;

      // Well, it's the result of mixing `Region based memory` with `Tagged unions`
      fn.type = (FnType)expr_area.get(decl.type).data.data_type.type;
      // TODO: `fn.args`

      fn_table[decl.name] = fn;
    }
    void Codegen::declare_variable(VarDecl& decl) {
      if (decl.ide == 0 || (decl.type == 0 && decl.value == 0)) {
        printf("You messed up on a variable declaration!\n");
        exit(1);
      }

      Expr ide = expr_area.get(decl.ide);
      if (ide.kind != ExprKind::Identifier) {
        printf("You messed up on a variable identifier\n");
        exit(1);
      }

      Variable var;
      var.name = ide.data.ide.name;

      if (decl.type != 0) {
        DataType data_type = expr_area.get(decl.type).data.data_type;

        if (data_type.type == Type::Void) {
          printf("You messed up! Variables can't have a `void` type");
          exit(1);
        }

        var.type = (VarType)data_type.type;
      }

      // TODO: handle the absence of `decl.type` and the presence of `decl.value`

      var.index = offset + (unsigned int)var.type;
      offset += var.index;
      vars_table[var.name] = var;
    }

    void Codegen::load_to_reg(const char* reg, Expr& expr) {
      switch (expr.kind) {
        case ExprKind::IntLit: {
          uint64_t value = expr.data.int_lit.value;
          str::appendf(&output, "    movq    $%lu, %%%s\n", value, reg);
          break;
        }
        case ExprKind::Identifier: {
          const char* name = expr.data.ide.name;
          check_identifier(name);

          Variable var = vars_table[name];
          str::Str inst = str::init("mov");

          unsigned int reg_size = regs_table.at(reg);
          unsigned int var_size = (unsigned int)var.type;

          // pretend we are dealing with signed values
          if (reg_size > var_size)
            str::appendf(&inst, "s%c%c", size_suffix(var_size), size_suffix(reg_size));
          else
            str::appendf(&inst, "%c", size_suffix(reg_size));

          str::appendf(&output, "    %-8s-%zu(%%rbp), %%%s\n", inst.content, var.index, reg);
          break;
        }
        default:
          printf("TODO: everything else\n");
      }
    }
    void Codegen::store_value(int64_t value, Expr& expr) {
      if (expr.kind != ExprKind::Identifier) {
        printf("Why are you trying to store on non-identifier things?\n");
        exit(1);
      }

      const char* name = expr.data.ide.name;
      check_identifier(name);

      Variable var = vars_table[name];
      const char suffix = size_suffix((unsigned int) var.type);
      str::appendf(&output, "    mov%c    $%ld, -%zu(%%rbp)\n", suffix, value, var.index);
    }

    void Codegen::add(ExprRef left_ref, ExprRef right_ref) {

      Expr left = expr_area.get(left_ref);
      Expr right = expr_area.get(right_ref);

      if (left.kind != ExprKind::Identifier && left.kind != ExprKind::IntLit) {
        printf("TODO: `add` operation for `left` side non-(Identifier, IntLit) expressions\n");
        exit(1);
      }

      if (right.kind != ExprKind::Identifier && right.kind != ExprKind::IntLit) {
        printf("TODO: `add` operation for `right` side non-(Identifier, IntLit) expressions\n");
        exit(1);
      }

      if (left.kind == ExprKind::IntLit && right.kind == ExprKind::IntLit) {
        uint64_t result = left.data.int_lit.value + right.data.int_lit.value; 
        // store_value(result, );
      }

    }
    // void Codegen::sub();
    // void Codegen::mul();
    // void Codegen::div();
    void Codegen::asgn(ExprRef left_ref, ExprRef right_ref) {
      Expr left = expr_area.get(left_ref);
      Expr right = expr_area.get(right_ref);

      if (left.kind != ExprKind::Identifier) {
        printf("TODO: `asgn` for non-identifier left sides\n");
        exit(1);
      }

      const char* name = left.data.ide.name;
      check_identifier(name);

      Variable left_var = vars_table[name];
      switch (right.kind) {
        case ExprKind::IntLit: {
          char var_suff = size_suffix((unsigned int) left_var.type);
          uint64_t value = right.data.int_lit.value;
          str::appendf(&output, "    mov%c    $%lu, -%zu(%%rbp)\n", var_suff, value, left_var.index);
          break;
        }
        case ExprKind::Identifier: {
          unsigned int size = (unsigned int)left_var.type;
          const char suffix = size_suffix(size);
          const char* reg = size_areg((unsigned int)left_var.type);

          load_to_reg(reg, right);
          str::appendf(&output, "    mov%c    %%%s, -%zu(%%rbp)\n", suffix, reg, left_var.index);
          break;
        }
        default:
          printf("TODO\n");
      }
    }
    // void Codegen::neg();

    char Codegen::size_suffix(unsigned int size) {
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
    char* Codegen::size_areg(unsigned int size) {
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

    void Codegen::check_identifier(const char* name) {
      if (vars_table.find(name) == vars_table.end()) {
        printf("Use of undeclared identifier: %s\n", name);
        exit(1);
      }
    }
  } // namespace codegen
} // namespace phantom
