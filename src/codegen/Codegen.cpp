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
        default:
          printf("TODO: everything else\n");
      }
    }
    // void Codegen::mov_to(const char* dst, Expr& expr) {
    //   switch (expr.kind) {
    //     case ExprKind::IntLit: {
    //       uint64_t value = expr.data.int_lit.value;
    //       str::appendf(&output, "    movq    $%lu, %s\n", value, dst);
    //       break;
    //     }
    //     case ExprKind::Identifier: {
    //       const char* name = expr.data.ide.name;
    //       if (vars_table.find(name) == vars_table.end()) {
    //         printf("Use of undeclared identifier: %s\n", name);
    //         exit(1);
    //       }
    //
    //       Variable ide = vars_table[name];
    //
    //     }
    //     default:
    //       printf("TODO: everything else\n");
    //   }
    // }

    // void Codegen::add();
    // void Codegen::sub();
    // void Codegen::mul();
    // void Codegen::div();
    void Codegen::asgn(ExprRef left_ref, ExprRef right_ref) {
      Expr left = expr_area.get(left_ref);
      Expr right = expr_area.get(right_ref);

      if (left.kind != ExprKind::Identifier) {
        printf("TODO\n");
        exit(1);
      }

      std::string name = left.data.ide.name;
      if (vars_table.find(name) == vars_table.end()) {
        printf("Use of undeclared identifier: %s\n", name.c_str());
        exit(1);
      }

      Variable left_var = vars_table[name];
      switch (right.kind) {
        case ExprKind::IntLit: {
          char var_suff = resolve_suffix(left_var.type);
          uint64_t value = right.data.int_lit.value;
          str::appendf(&output, "    mov%c    $%lu, -%zu(%%rbp)\n", var_suff, value, left_var.index);
          break;
        }
        case ExprKind::Identifier: {
          const char* ide_name = right.data.ide.name;
          if (vars_table.find(ide_name) == vars_table.end()) {
            printf("Use of undeclared identifier: %s\n", ide_name);
            exit(1);
          }
          Variable right_var = vars_table[ide_name];

          char left_suff = resolve_suffix(left_var.type);
          char* left_reg = resolve_reg(left_var.type);
          str::Str mov = str::init(9);
          str::Str reg = str::init(5);

          if (left_var.type > right_var.type) {
            str::appendf(&mov, "movs%c%c", resolve_suffix(right_var.type), left_suff);
            str::appendf(&reg, "%%%s", left_reg);
          } else if (right_var.type > left_var.type) {
            str::appendf(&mov, "mov%c", resolve_suffix(right_var.type));
            str::appendf(&reg, "%%%s", resolve_reg(right_var.type));
          } else {
            str::appendf(&mov, "mov%c", left_suff);
            str::appendf(&reg, "%%%s", left_reg);
          }

          str::appendf(&output, "    %-8s-%zu(%%rbp), %s\n", mov.content, right_var.index, reg.content);
          str::appendf(&output, "    mov%c    %%%s, -%zu(%%rbp)\n", left_suff, left_reg, left_var.index);
          break;
        }
        default:
          printf("TODO\n");
      }
    }
    // void Codegen::neg();

    char Codegen::resolve_suffix(VarType type) {
      switch (type) {
        case VarType::Bool:
          // case VarType::Char:
          return 'b';
        case VarType::Short:
          return 'w';
        case VarType::Int:
          return 'l';
        case VarType::Long:
          return 'q';
      }
    }
    char* Codegen::resolve_reg(VarType type) {
      switch (type) {
        case VarType::Bool:
          // case VarType::Char:
          return (char*)"al";
        case VarType::Short:
          return (char*)"ax";
        case VarType::Int:
          return (char*)"eax";
        case VarType::Long:
          return (char*)"rax";
      }
    }
  } // namespace codegen
} // namespace phantom
