#include <codegen/Codegen.hpp>

namespace phantom {
  namespace codegen {
    void Codegen::codegen() {
      for (StmtRef& stmt_ref : this->ast) {
        Stmt stmt = stmt_area.get(stmt_ref);    

        switch (stmt.kind) {
          case StmtKind::Invalid:
            exit(1);
          case StmtKind::FnDef:
            generate_function(stmt);
        }
      }
    }

    void Codegen::generate_function(Stmt& stmt) {
      FnDef fn = stmt.data.fn_def;
      FnDecl dec = stmt_area.get(fn.declaration).data.fn_decl;

      // TODO: complete from HEREEEEEEEEEEEEEEEEEEEEEEEEEEE
      str::appendf(&output, ".global %s\n", dec.name);
    }
  }
}
