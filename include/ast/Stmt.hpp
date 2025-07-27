#pragma once

#include <common.hpp>
#include "Expr.hpp"

namespace phantom {
  enum class StmtKind {
    Invalid,
    Return,
    Expmt,
    FnDecl,
    FnDef,
  };

  struct Return {
    ExprRef expr;
  };
  struct Expmt {
    ExprRef expr;
  };
  struct FnDecl {
    const char* name;
    ExprRef type;
    ExprRef* params;
    size_t params_len;
  };
  struct FnDef {
    StmtRef declaration;
    StmtRef* body;
    size_t body_len;
  };

  struct Stmt {
    StmtKind kind;

    union {
      Invalid invalid;
      Return ret;
      Expmt expmt;
      FnDecl fn_decl;
      FnDef fn_def;
    } data;
  };

} // namespace phantom
