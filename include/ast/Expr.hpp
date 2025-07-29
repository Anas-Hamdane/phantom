#pragma once

#include <Token.hpp>
#include <common.hpp>
#include <utils/vec.hpp>

namespace phantom {
  struct Expr;
  enum class ExprKind : uint {
    Invalid = 0,
    DataType,
    BoolLit,
    CharLit,
    IntLit,
    FloatLit,
    StrLit,
    ArrLit,
    Identifier,
    BinOp,
    UnOp,
    VarDecl,
    Param,
    FnCall
  };

  struct Invalid {};
  struct DataType {
    Type type;
    Expr* length;
  };
  struct BoolLit {
    bool value;
  };
  struct CharLit {
    char value;
  };
  struct IntLit {
    const char* form;
    uint64_t value;
  };
  struct FloatLit {
    const char* form;
    long double value;
  };
  struct StrLit {
    const char* value;
  };
  struct ArrLit {
    vec::Vec<Expr> elements;
  };
  struct Identifier {
    const char* name;
  };
  struct BinOp {
    Expr* left;
    Token::Kind op;
    Expr* right;
  };
  struct UnOp {
    Expr* expr;
    Token::Kind op;
  };
  struct VarDecl {
    Expr* ide;
    Expr* value;
    Expr* type;
  };
  struct Param {
    Expr* ide;
    Expr* type;
  };
  struct FnCall {
    const char* name;
    vec::Vec<Expr> args;
  };

  struct Expr {
    ExprKind kind;

    union {
      Invalid invalid;
      DataType data_type;
      BoolLit bool_lit;
      CharLit char_lit;
      IntLit int_lit;
      FloatLit float_lit;
      StrLit str_lit;
      ArrLit arr_lit;
      Identifier ide;
      BinOp binop;
      UnOp unop;
      VarDecl var_decl;
      Param param;
      FnCall fn_call;
    } data;
  };

} // namespace phantom
