// #include <irgen/Gen.hpp>
//
// namespace phantom {
//   namespace irgen {
//     std::vector<Instruction> Gen::gen() {
//       if (!output.empty())
//         output.clear();
//
//       for (StmtRef stmt_ref : ast) {
//         Stmt stmt = stmt_area.get(stmt_ref);
//
//         switch (stmt.kind) {
//           case StmtKind::Invalid:
//             printf("Invalid statements are not allowed in IR generation phase\n");
//             exit(1);
//           case StmtKind::FnDecl:
//             declare_function(stmt.data.fn_decl);
//         }
//       }
//
//       return output;
//     }
//
//     void Gen::declare_function(phantom::FnDecl fn) {
//       std::vector<Register> params;
//       for (size_t i = 0; i < fn.params_len; ++i) {
//         VarDecl param = expr_area.get(fn.params[i]).data.var_decl;
//         Type type = resolve_type();
//         Register param {.name = param.ide, .allocated = false, .type = Type::Char};
//       }
//
//       FnDecl decl {.fn = {.name = fn.name, .params = fn.params, }};
//     }
//   } // namespace irgen
// } // namespace phantom
