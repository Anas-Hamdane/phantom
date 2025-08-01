#pragma once

#include "Expr.hpp"
#include <common.hpp>

namespace phantom {
  namespace ast {
    struct Return;
    struct Expmt;
    struct FnDecl;
    struct FnDef;

    using Stmt = std::variant<std::unique_ptr<Return>, std::unique_ptr<Expmt>,
                              std::unique_ptr<FnDecl>, std::unique_ptr<FnDef>>;

    struct Return {
      std::unique_ptr<Expr> expr;
    };
    struct Expmt {
      std::unique_ptr<Expr> expr;
    };
    struct FnDecl {
      std::string name;
      std::unique_ptr<Type> type;
      std::vector<std::unique_ptr<VarDecl>> params;
    };
    struct FnDef {
      std::unique_ptr<FnDecl> decl;
      std::vector<std::unique_ptr<Stmt>> body;
    };
  } // namespace ast
} // namespace phantom
