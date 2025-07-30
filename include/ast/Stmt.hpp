#pragma once

#include "Expr.hpp"
#include <common.hpp>

namespace phantom {
  struct Stmt;
  enum class StmtKind {
    Invalid = 0,
    Return,
    Expmt,
    FnDecl,
    FnDef,
  };

  struct Return {
    Expr* expr;
  };
  struct Expmt {
    Expr* expr;
  };
  struct FnDecl {
    const char* name;
    DataType* type;
    utils::Vec<Param> params;
  };
  struct FnDef {
    FnDecl* declaration;
    utils::Vec<Stmt> body;
  };

  struct Stmt {
    StmtKind kind;

    union {
      Return ret;
      Expmt expmt;
      FnDecl fn_decl;
      FnDef fn_def;
    } data;
  };
} // namespace phantom
