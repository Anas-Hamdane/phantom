#include <irgen/Gen.hpp>

namespace phantom {
  namespace irgen {
    vec::Vec<Instruction> Gen::gen() {
      vec::Vec<Instruction> out;

      for (Stmt& stmt : ast) {
      }

      return out;
    }

    Instruction Gen::generate_stmt(Stmt& stmt) {
      switch (stmt.kind) {
        case StmtKind::Invalid:
          printf("Invalid statements are not allowed in IR generation phase\n");
          exit(1);
        case StmtKind::FnDecl:
          return declare_function(stmt.data.fn_decl);
        default:
          printf("TODO\n");
          exit(1);
      }
    }

    Instruction Gen::declare_function(phantom::FnDecl fn) {
      std::vector<Register> params;
      for (size_t i = 0; i < fn.params_len; ++i) {
        VarDecl param = expr_area.get(fn.params[i]).data.var_decl;
        Type type = resolve_type();
        Register param{ .name = param.ide, .allocated = false, .type = Type::Char };
      }

      FnDecl decl{ .fn = {
                       .name = fn.name,
                       .params = fn.params,
                   } };
    }
  } // namespace irgen
} // namespace phantom
