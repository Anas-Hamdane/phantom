#pragma once

#include "Expr.hpp"
#include <common.hpp>

namespace phantom {
  enum class StmtKind {
    Invalid = 0,
    Return,
    Expmt,
    FnDecl,
    FnDef,
  };

  struct Return;
  struct Expmt;
  struct FnDecl;
  struct FnDef;

  struct Stmt {
    StmtKind kind;

    union {
      struct Return {
        std::unique_ptr<Expr> expr;
      };
      struct Expmt {
        std::unique_ptr<Expr> expr;
      };
      struct FnDecl {
        const char* name;
        std::unique_ptr<Expr> type;
        std::vector<std::unique_ptr<Expr>> params;
      };
      struct FnDef {
        std::unique_ptr<Stmt> declaration;
        std::vector<std::vector<Stmt>> body;
        size_t body_len;
      };
    } data;
  };
} // namespace phantom
